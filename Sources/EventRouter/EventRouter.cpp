#include "EventRouter.h"

using namespace j2;

void EventRouter::publish(const std::string& name, const boost::any value) {
    (*signal_for(name))(value);
}

Subscription<> EventRouter::subscribe(const std::string& name,
                                boost::function1<void, boost::any> callback) {
    return Subscription<>(*this,
                          name,
                          signal_for(name)->connect(callback));
}

SignalPtr EventRouter::signal_for(const std::string& name) {
    Subscriptions::iterator it = _subscriptions.find(name);
    if (it != _subscriptions.end()) return it->second;
    _subscriptions.insert(std::make_pair(name, SignalPtr(new Signal())));
    return _subscriptions.find(name)->second;
}

