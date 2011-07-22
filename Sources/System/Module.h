#include <tr1/memory>

#ifndef _MODULE_H
#define _MODULE_H

#include <EventRouter.h>

namespace j2 {

    class Module {        
    public:
        Module(EventRouter* central=EventRouter::instance()) :
            _central(central), _local() {
        }

        template <typename T>
        void bind_value(const std::string& name, T* value) {
            _local.subscribe<T>(name).assign_to(value);
            _central->route(name, _local);
        }
    private:
        std::tr1::shared_ptr<EventRouter> _central;
        EventRouter _local;
        EventQueue _queue;
    };

} // namespace j2

#endif // _MODULE_H

