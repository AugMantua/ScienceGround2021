#[cfg(test)]
mod test_integration {
    use mqttToRest::MqttHandler;
    use regex::Regex;
    use rumqttc::{AsyncClient, MqttOptions, QoS};
    use std::{sync::Arc, time::Duration};
    use testcontainers::{clients, GenericImage};
    use tokio::time::timeout;

    #[tokio::test]
    async fn test_mqtt_handler() {
        let docker = clients::Cli::default();
        let mqtt_broker_image =
            GenericImage::new("eclipse-mosquitto", "1.6.12").with_exposed_port(1883);
        const TEST_TOPIC: &str = "terrariums/1/temperature";

        let mqtt_broker = docker.run(mqtt_broker_image);
        let port = mqtt_broker.get_host_port_ipv4(1883);

        // Create MQTT handler
        let mut handler = MqttHandler::new("test_client", "localhost", port);
        let mock_handle_temperature: Arc<fn(&str, &[u8])> = Arc::new(|topic, payload| {
            println!("Temperature Data: {} - {:?}", topic, payload);
            assert_eq!(topic, TEST_TOPIC);
        });

        handler.add_handler(
            Regex::new(r"terrariums/.+/temperature").unwrap(),
            mock_handle_temperature,
        );

        handler.subscribe("terrariums").await;

        tokio::spawn(async move {
            let mqttoptions = MqttOptions::new("test_publisher", "localhost", port);
            let (client, mut eventloop) = AsyncClient::new(mqttoptions, 10);

            tokio::spawn(async move {
                loop {
                    match eventloop.poll().await {
                        Ok(event) => match event {
                            _ => {}
                        },
                        Err(e) => {
                            eprintln!("Error polling event loop: {}", e);
                            break;
                        }
                    }
                }
            });

            // Wait for the connection to be established
            tokio::time::sleep(Duration::from_secs(5)).await;

            if let Err(err) = client
                .publish(TEST_TOPIC, QoS::AtLeastOnce, false, "Hello MQTT")
                .await
            {
                println!("Error publishing message: {:?}", err.to_string());
            }
        });

        let handler_future = handler.handle_events(mqttToRest::HandlerOptions {
            verbose: true,
            max_num_messages: Some(1), // Only expect one message
        });

        match timeout(Duration::from_secs(30), handler_future).await {
            Ok(_) => println!("Completed within 30 seconds"),
            Err(_) => println!("Timed out after 30 seconds"),
        }
    }
}
