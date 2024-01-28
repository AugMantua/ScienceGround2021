use std::sync::Arc;

use mqttRouter::MqttRouter;
use regex::Regex;
use rumqttc::{AsyncClient, Event, MqttOptions, Packet, QoS};

pub struct MqttHandler {
    router: MqttRouter,
    client: AsyncClient,
    eventloop: rumqttc::EventLoop,
}

pub struct HandlerOptions {
    pub verbose: bool,
    pub max_num_messages: Option<usize>,
}

impl MqttHandler {
    pub fn new(client_id: &str, host: &str, port: u16) -> Self {
        let router = MqttRouter::new();
        let mqttoptions = MqttOptions::new(client_id, host, port);
        let (client, eventloop) = AsyncClient::new(mqttoptions, 10);

        MqttHandler {
            router,
            client,
            eventloop,
        }
    }

    pub async fn subscribe(&self, topic: &str) {
        let general_topic = format!("{}/#", topic);
        self.client
            .subscribe(general_topic, QoS::AtLeastOnce)
            .await
            .unwrap();
    }

    pub async fn handle_events(&mut self, options: HandlerOptions) {
        let mut num_messages = 0;
        loop {
            match self.eventloop.poll().await {
                Ok(event) => match event {
                    Event::Incoming(Packet::Publish(p)) => {
                        if options.verbose {
                            println!("Received message on topic {}", p.topic);
                        }
                        self.router.route_message(&p.topic, &p.payload);
                        if let Some(max_num_messages) = options.max_num_messages {
                            num_messages += 1;
                            if num_messages >= max_num_messages {
                                break;
                            }
                        }
                    }
                    _ => {}
                },
                Err(e) => {
                    eprintln!("Error polling event loop: {}", e);
                    break;
                }
            }
        }
    }

    pub async fn close(&mut self) {
        self.client.disconnect().await;
    }

    pub fn add_handler(&mut self, patter: Regex, handler: Arc<dyn Fn(&str, &[u8]) + Send + Sync>) {
        self.router.add_handler(patter, handler);
    }
}
