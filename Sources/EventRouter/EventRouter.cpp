#include "EventRouter.h"

using namespace j2;

EventRouter* EventRouter::default_instance = 0;

EventRouter* EventRouter::instance() { 
    if (EventRouter::default_instance) return EventRouter::default_instance;
    EventRouter::default_instance = new EventRouter(); 
    return EventRouter::default_instance;
}

void EventRouter::publish(const std::string& name, const boost::any value) {
    (*signal_for(name))(name, value);
}

Subscription<> EventRouter::subscribe(const std::string& name,
                                      boost::function2<void, const std::string, boost::any> callback) {
    return Subscription<>(*this,
                          name,
                          signal_for(name)->connect(callback));
}

SignalPtr EventRouter::signal_for(const std::string& name) {
    std::map<std::string, SignalPtr>::iterator it = _subscriptions.find(name);
    if (it != _subscriptions.end()) return it->second;
    _subscriptions.insert(std::make_pair(name, SignalPtr(new Signal())));
    return _subscriptions.find(name)->second;
}

