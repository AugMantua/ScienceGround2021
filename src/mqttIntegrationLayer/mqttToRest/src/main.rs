use std::sync::Arc;
use regex::Regex;
use rumqttc::{MqttOptions, AsyncClient, QoS, EventLoop, Packet, Event};
use tokio;
use mqttRouter::{MqttRouter, HandlerFunction};

#[tokio::main]
async fn main() {
    let mut router = MqttRouter::new();
    router.add_handler(Regex::new(r"terraiums/.+/temperature").unwrap(), Arc::new(handle_temperature));
    router.add_handler(Regex::new(r"terraiums/.+/humidity").unwrap(), Arc::new(handle_humidity));
    router.add_handler(Regex::new(r"terraiums/.+/co2").unwrap(), Arc::new(handle_co2));

    let mqttoptions = MqttOptions::new("client_id", "broker_address", 1883);
    let (client, mut eventloop) = AsyncClient::new(mqttoptions, 10);

    // Subscribe to topics
    client.subscribe("terraiums/#", QoS::AtLeastOnce).await.unwrap();

    loop {
        match eventloop.poll().await.unwrap() {
            Event::Incoming(Packet::Publish(p)) => {
                router.route_message(&p.topic, &p.payload);
            }
            _ => {}
        }
    }
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
