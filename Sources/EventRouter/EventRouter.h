#include <string>
#include <memory>
#include <tr1/unordered_map>
#include <vector>
#include <tr1/memory>
#include <tr1/functional>
#include <boost/any.hpp>
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/bind/protect.hpp>

#ifndef _EVENT_ROUTER_H
#define _EVENT_ROUTER_H

namespace j2 {

    // Template to adapt a callback function to a specific type to a
    // callback that can take boost::any and cast it.  This is used
    // by Subscription::deliver_with
    template <typename T> struct ANY_FUNC_ADAPTOR {
        static void ADAPT(const std::string& name,
                          const boost::any value, 
                          boost::function2<void, const std::string&, T> func) {
            func(name, boost::any_cast<T>(value));
        }
    };

    // Template to adapt a callback function that takes a ptr 
    // (actually any class that supports unary *) to a 
    // callback that can take boost::any and cast it.  This is used
    // by Subscription::assign_to
    template <typename T, typename P> struct ANY_PTR_ADAPTOR {
        static void ADAPT(const std::string& name, const boost::any item, P ptr) {
            *ptr = boost::any_cast<T>(item);
        }
    };

    /** @brief Signal used by @c EventRouter */
    typedef boost::signal<void (const std::string&, const boost::any)> Signal;

    /** @brief @c shared_ptr for Signal used by @c EventRouter */
    typedef std::tr1::shared_ptr<Signal> SignalPtr;


    // Forward declaration of EventRouter for Subscription
    class EventRouter;

    /**
     * @brief Subscription to events published by an EventRouter.
     * 
     * Subscription objects are obtained by calls to @c EventRouter::subscribe
     * and can be used to add targets for events; unsubscribe to events, or
     * temporarily block and unblock events from being received.
     *
     * When an event is triggered the value can either be assigned to one or more
     * pointer values (using @c Subscription::assign_to) and/or delivered using a
     * one or more callback functions (@c Subscription::deliver_with).  In either
     * case the type of the value used must match the type used to publish the
     * event or you will receive a runtime exception.
     *
     * Note: that holding a reference to a @Subscription is currently the only way
     * to unsubscribe from events.
     *
     * @tparam T type of event expected.
     *
     * @see EventRouter
     */
    // Note: EVENT_ROUTER template parameter is used here is used because of circular
    // dependency between EventRouter and Subscription
    template <typename T=boost::any, class EVENT_ROUTER = EventRouter>
    class Subscription {
    private:
        typedef std::vector<boost::signals::connection> connection_list;
    public:
        /** @brief Construct a @c Subscription from an existing connection. */
        Subscription(EVENT_ROUTER& parent,
                     const std::string& name,
                     boost::signals::connection connection) :
            _event_router(parent),
            _name(name) { 
            _connections.push_back(connection);
        }


        /** @brief Construct an unbound @c Subscription. */
        Subscription(EVENT_ROUTER& parent,
                     const std::string& name) :
            _event_router(parent),
            _name(name) { }

        
        /** @brief The name of the event we are subscribing to. */
        const std::string& name() const { return _name; }
        

        /** 
         * @brief When events are received, assign the result to the given pointer.
         * @param ptr pointer to assign result to
         * @return a reference to the @c Subscription to allow chaining
         **/
        Subscription& assign_to(T* ptr) {
            return adapt<T*>(ANY_PTR_ADAPTOR< T, T* >::ADAPT, ptr);
        }

        /** 
         * @brief When events are received, assign the result to the given pointer.
         * @param ptr @c shared_ptr to assign result to
         * @return a reference to the @c Subscription to allow chaining
         **/
        Subscription& assign_to(std::tr1::shared_ptr<T> ptr) {
            return adapt< std::tr1::shared_ptr<T> >
                (ANY_PTR_ADAPTOR< T, std::tr1::shared_ptr<T> >::ADAPT, ptr);
        }

        /** 
         * @brief When events are received call the given callback function.
         * @param func callback function to invoke
         * @return a reference to the @c Subscription to allow chaining
         **/
        Subscription& deliver_with(boost::function2<void, const std::string&, T> func) {
            return adapt< boost::function2<void, const std::string&, T> >(ANY_FUNC_ADAPTOR<T>::ADAPT, func);
        }

        /**
         * @brief temporarily block delivery of events for this @c Subscription.
         * @see unblock
         **/
        void block() {
            std::for_each(_connections.begin(), _connections.end(),
                          std::mem_fun(&boost::signals::connection::block));
        }

        /**
         * @brief remove a temporary block on delivery of events for this @c Subscription.
         * @see block
         **/
        void unblock() { 
            std::for_each(_connections.begin(), _connections.end(),
                          std::mem_fun(&boost::signals::connection::unblock));
        }

        /**
         * @brief unsubscribe from events.
         **/
        void unsubscribe() { 
            std::for_each(_connections.begin(), _connections.end(),
                          std::mem_fun(&boost::signals::connection::disconnect));
        }

    private:
        // Template used in adapting callback/ptr assignment
        template <typename DEST>
        Subscription& adapt(boost::function3<void, const std::string&, const boost::any, DEST> func,
                            DEST dest) {
            SignalPtr signal = _event_router.signal_for(_name);
            boost::signals::connection connection = 
                signal->connect(boost::bind(func, _1, _2, dest));
            _connections.push_back(connection);
            return *this;
        }

    private:
        EVENT_ROUTER& _event_router; // Reference to parent EventRouter
        const std::string _name;
        std::vector<boost::signals::connection> _connections;
    };

    template <class EVENT_ROUTER>
    class ImmediateDeliveryPolicy {
    public:
        void operator()(EVENT_ROUTER& router,
                        const std::string& name,
                        const boost::any value) {
            router.deliver(name, value);
        }
    };

    typedef ImmediateDeliveryPolicy<EventRouter> DefaultDeliveryPolicy;


    /**
     * @brief Publish/Subscribe mechanism for sending/receiving events.
     *
     **/
    class EventRouter {
    protected:
    public:
        /**
         * @brief Get the default instance of the EventRouter.
         *
         * Note: While @c EventRouter supports the Singleton pattern in this sense,
         * it is not enforced, so that a specific EventRouter instance may be used
         * for example in unit tests.
         */
        static EventRouter* instance();
    public:
        typedef std::tr1::function<void (EventRouter&, const std::string&, const boost::any)> DeliveryPolicy;

    public:
        EventRouter(DeliveryPolicy policy=DefaultDeliveryPolicy()) : 
            _deliver(policy) { }

        /**
         * @brief Publish an event of the given name with a value according to the
         * configured publication policy.
         *
         * @param name name of the event
         * @param value value (must match type used in subscription).
         */
        void publish(const std::string& name, const boost::any value) {
            _deliver(*this, name, value);
        }

        /**
         * @brief Deliver an event of the given name with a value.
         *
         * 
         * @param name name of the event
         * @param value value (must match type used in subscription).
         */
        void deliver(const std::string& name, const boost::any value);


        /**
         * @brief Create a subcription for a given event.
         * @param name name of event to subscribe to
         * @return a @c Subscription object that can be bound to callbacks or
         *  pointer values
         **/
        template <typename T>
        Subscription<T> subscribe(const std::string& name) {
            return Subscription<T, EventRouter>(*this, name);
        }

        /**
         * @brief Route messages for the given name to the destination event router.
         *
         * Provides a special subscription that can be used to route events between
         * two @c EventRouters.  Events published on one @c EventRouter are automatically
         * published on the other.
         *
         * @param name name of event to route
         * @param dest destimation event router
         * @return a @c Subscription object that can be used to unsubscribe from routed
         * events.
         */
        Subscription<> route(const std::string& name, EventRouter& dest) {
            return subscribe(name, 
                             boost::bind<void>(&EventRouter::publish,
                                               boost::ref(dest), _1, _2));
        }

        /**
         * @brief Create a subcription for a given event with a generic callback
         * @param name of event to subscribe to
         * @return a @c Subscription object bound to the generic callback
         **/
        Subscription<> subscribe(const std::string& name,
                                 boost::function2<void, const std::string, boost::any> callback);

        /**
         * @brief Return the boost signal used for event delivery for the corresponding name.
         * @param name of event to subscribe to
         * @return a @c boost::signal
         **/
        SignalPtr signal_for(const std::string& name);

    private:
        static EventRouter* default_instance; 

    private:
        std::tr1::unordered_map<std::string, SignalPtr> _subscriptions;
        DeliveryPolicy _deliver;
    };

    

} // namespace j2

#endif // _EVENT_ROUTER_H
