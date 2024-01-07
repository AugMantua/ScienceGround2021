#[cfg(test)]
mod test_integration {
    use std::sync::{Arc, Mutex};

    use mqttRouter::{MqttRouter, HandlerFunction};
    use regex::Regex;
    use spy::{Spy, spy};
    use pretty_assertions::assert_eq;


    #[test]
    fn test_integration() {
        let mut router = MqttRouter::new();
        let (spy_fn, spy) = spy!();
        // mutex spy_fn so we can send it to the handler
        let mutex = Arc::new(Mutex::new(spy_fn));
        let handler: HandlerFunction = Arc::new(move |topic, payload| {
            assert_eq!(topic, "test");
            assert_eq!(payload, b"payload");
            mutex.lock().unwrap()();
        });
        router.add_handler(Regex::new("test").unwrap(), handler);
        router.route_message("test", b"payload");
        let mut snaphost = spy.snapshot();
        assert_eq!(snaphost.num_of_calls(), 1);
        router.route_message("test", b"payload");
        snaphost = spy.snapshot();
        assert_eq!(snaphost.num_of_calls(), 1);
        router.route_message("notExistingTopic", b"aRandomPayload");
        snaphost = spy.snapshot();
        assert_eq!(snaphost.num_of_calls(), 0);
    }
}
