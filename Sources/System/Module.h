#include <tr1/memory>
#include <boost/function.hpp>

#ifndef _MODULE_H
#define _MODULE_H

#include <EventRouter.h>

namespace j2 {

    class Module {        
    public:
        Module(EventRouter* central=EventRouter::instance()) :
            _central(central), 
            _queue(new EventQueue), 
            _local(QueueingDeliveryPolicy(_queue)) {
        }

        template <typename T>
        void bind_value(const std::string& name, T* value) {
            _local.subscribe<T>(name).assign_to(value);
            _central->route(name, _local);
        }

        template <typename T>
        void bind_fn(const std::string& name,
                     std::tr1::function<void(T)> fn) {
            _local.subscribe<T>(name).deliver_with2(fn);
            _central->route(name, _local);
        }

        int process(int n=1) {
            int nr_to_process = std::min(n, _queue->size());
            for (int i=0; i < nr_to_process; i++) {
                _queue->deliver();
            }
            return nr_to_process;
        }

        void process_all() {
            process(_queue->size());
        }

    protected:
        std::tr1::shared_ptr<EventRouter> _central;
        std::tr1::shared_ptr<EventQueue> _queue;
        EventRouter _local;
    };

} // namespace j2

#endif // _MODULE_H

