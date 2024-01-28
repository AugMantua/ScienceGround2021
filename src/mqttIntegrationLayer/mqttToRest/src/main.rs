use mqttToRest::MqttHandler;
use regex::Regex;
use std::sync::Arc;
use tokio;

#[tokio::main]
async fn main() {
    let mut handler = MqttHandler::new("mqttToRest", "localhost", 1883);
    handler.add_handler(
        Regex::new(r"terraiums/.+/temperature").unwrap(),
        Arc::new(handle_temperature),
    );
    handler.add_handler(
        Regex::new(r"terraiums/.+/humidity").unwrap(),
        Arc::new(handle_humidity),
    );
    handler.add_handler(
        Regex::new(r"terraiums/.+/co2").unwrap(),
        Arc::new(handle_co2),
    );
    handler.subscribe("terrariums").await;

    handler
        .handle_events(mqttToRest::HandlerOptions {
            verbose: true,
            max_num_messages: None,
        })
        .await;
}

fn handle_temperature(topic: &str, payload: &[u8]) {
    println!("Temperature Data: {} - {:?}", topic, payload);
}

fn handle_humidity(topic: &str, payload: &[u8]) {
    println!("Humidity Data: {} - {:?}", topic, payload);
}

fn handle_co2(topic: &str, payload: &[u8]) {
    println!("CO2 Data: {} - {:?}", topic, payload);
}
