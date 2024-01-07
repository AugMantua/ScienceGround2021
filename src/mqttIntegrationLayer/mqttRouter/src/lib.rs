use std::collections::HashMap;
use std::sync::{Arc, RwLock};
use regex::Regex;

pub type HandlerFunction = Arc<dyn Fn(&str, &[u8]) + Send + Sync>;

pub struct MqttRouter {
    handlers: Vec<(Regex, HandlerFunction)>,
    cache: RwLock<HashMap<String, HandlerFunction>>,
}

impl MqttRouter {
    pub fn new() -> MqttRouter {
        MqttRouter {
            handlers: Vec::new(),
            cache: RwLock::new(HashMap::new()),
        }
    }

    pub fn add_handler(&mut self, pattern: Regex, handler: HandlerFunction) {
        self.handlers.push((pattern, handler));
    }

    pub fn route_message(&self, topic: &str, payload: &[u8]) {
        if let Some(handler) = self.get_handler_for_topic(topic) {
            handler(topic, payload);
        } else {
            // Handle case where no handler is found, e.g., log an error
        }
    }

    fn get_handler_for_topic(&self, topic: &str) -> Option<HandlerFunction> {
        {
            let cache_read = self.cache.read().unwrap();
            if let Some(handler) = cache_read.get(topic) {
                return Some(handler.clone());
            }
        }

        let matched_handler = self.handlers.iter()
            .find(|(pattern, _)| pattern.is_match(topic))
            .map(|(_, handler)| handler.clone());

        if let Some(handler) = matched_handler {
            let mut cache_write = self.cache.write().unwrap();
            cache_write.insert(topic.to_string(), handler.clone());
            Some(handler)
        } else {
            None
        }
    }
}

#[test]
fn test_new() {
    let router = MqttRouter::new();
    assert!(router.handlers.is_empty());
    assert!(router.cache.read().unwrap().is_empty());
}

#[test]
fn test_add_handler() {
    let mut router = MqttRouter::new();
    let handler: HandlerFunction = Arc::new(|_, _| {});
    let pattern = Regex::new("test").unwrap();
    router.add_handler(pattern.clone(), handler.clone());
    assert_eq!(router.handlers.len(), 1);
    assert_eq!(router.handlers[0].0.as_str(), pattern.as_str());
    assert!(Arc::ptr_eq(&router.handlers[0].1, &handler));
}

#[test]
fn test_route_message() {
    let mut router = MqttRouter::new();
    let handler: HandlerFunction = Arc::new(|topic, payload| {
        assert_eq!(topic, "test");
        assert_eq!(payload, b"payload");
    });
    router.add_handler(Regex::new("test").unwrap(), handler);
    router.route_message("test", b"payload");
}

#[test]
fn test_get_handler_for_topic() {
    let mut router = MqttRouter::new();
    let handler: HandlerFunction = Arc::new(|_, _| {});
    router.add_handler(Regex::new("test").unwrap(), handler.clone());
    assert!(router.get_handler_for_topic("test").is_some());
    assert!(Arc::ptr_eq(
        &router.get_handler_for_topic("test").unwrap(),
        &handler
    ));
    assert!(router.get_handler_for_topic("nonexistent").is_none());
}