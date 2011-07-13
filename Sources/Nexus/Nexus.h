#include <string>
#include <memory>
#include <map>
#include <vector>
#include <tr1/memory>
#include <boost/any.hpp>
#include <boost/signal.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>

#ifndef _NEXUS_H
#define _NEXUS_H

namespace j2 {

    template <typename T> struct ANY_FUNC_ADAPTOR {
        static void ADAPT(const boost::any value, boost::function1<void, T> func) {
            func(boost::any_cast<T>(value));
        }
    };

    template <typename T, typename P> struct ANY_PTR_ADAPTOR {
        static void ADAPT(const boost::any item, P ptr) {
            *ptr = boost::any_cast<T>(item);
        }
    };

    typedef boost::function1<void, const boost::any> OnMessage;
    typedef boost::signal<void (const boost::any)> Signal;
    typedef std::tr1::shared_ptr<Signal> SignalPtr;
    typedef std::map<std::string, SignalPtr> Subscriptions;

    class Nexus;

    template <typename T=boost::any, typename NEXUS = Nexus>
    class Subscription {
    private:
        typedef std::vector<boost::signals::connection> connection_list;
    public:

        Subscription(NEXUS& parent,
                     const std::string& name,
                     boost::signals::connection connection) :
            _nexus(parent),
            _name(name) { 
            _connections.push_back(connection);
        }

        Subscription(NEXUS& parent,
                     const std::string& name) :
            _nexus(parent),
            _name(name) { }
        
        const std::string& name() const { return _name; }
        
        Subscription& assignResultTo(T* ptr) {
            return adapt<T*>(ANY_PTR_ADAPTOR< T, T* >::ADAPT, ptr);
        }

        Subscription& assignResultTo(std::tr1::shared_ptr<T> ptr) {
            return adapt< std::tr1::shared_ptr<T> >
                (ANY_PTR_ADAPTOR< T, std::tr1::shared_ptr<T> >::ADAPT, ptr);
        }

        Subscription& deliverResultWith(boost::function1<void, T> func) {
            return adapt< boost::function1<void, T> >(ANY_FUNC_ADAPTOR<T>::ADAPT, func);
        }        

        void block() { 
            //boost::for_each(_connections.begin(), _connections.end(), _1->block());
            for (connection_list::iterator it = _connections.begin();
                 it != _connections.end();
                 it++) {
                it->block();
            }
        }

        void unblock() { 
            for (connection_list::iterator it = _connections.begin();
                 it != _connections.end();
                 it++) {
                it->unblock();
            }
        }

        void unsubscribe() { 
            for (connection_list::iterator it = _connections.begin();
                 it != _connections.end();
                 it++) {
                it->disconnect();
            }
        }

    private:
        template <typename DEST>
        Subscription& adapt(boost::function2<void, const boost::any, DEST> func,
                            DEST dest) {
            SignalPtr signal = _nexus.signal_for(_name);
            boost::signals::connection connection = 
                signal->connect(boost::bind(func, _1, dest));
            _connections.push_back(connection);
            return *this;
        }

    private:
        NEXUS& _nexus;
        const std::string _name;
        std::vector<boost::signals::connection> _connections;
    };

    class Nexus {
    public:

        //static Nexus default();
    public:
        void publish(const std::string& name, const boost::any value);

        template <typename T>
        Subscription<T> subscribe(const std::string& name) {
            return Subscription<T, Nexus>(*this, name);
        }

        Subscription<> subscribe(const std::string& name, 
                                  boost::function1<void, boost::any> callback);

        SignalPtr signal_for(const std::string& name);

    private:
        Subscriptions _subscriptions;
    };

    

} // namespace j2

#endif // _NEXUS_H
