#ifndef _SUPERVISOR_H
#define _SUPERVISOR_H

#include <vector>
#include <tr1/memory>
#include <boost/mem_fn.hpp>
#include <Module.h>
#include <EventRouter.h>


namespace j2 {

    typedef boost::shared_ptr<Module> SharedModule;

    typedef std::vector<SharedModule> ModuleList;

    /**
     * @class Strategy interface for scheduling modules.  The Scheduler must implement
     * the @c schedule method, to execute one or more actions from a ready module.
     */
    class Scheduler {
    public:
        /**
         * @brief Schedule a module to run.
         * Selects a ready module (if available) and schedules one or more actions from
         * its queue.
         * @param modules List of modules to schedule
         * @return Module that was scheduled or empty if none was ready
         */
        virtual boost::optional<Module*> schedule(ModuleList modules) = 0;
    };

    
    class BasicScheduler : public Scheduler {
    public:
        static Scheduler* instance();
    public:
        virtual boost::optional<Module*> schedule(ModuleList modules) {
            ModuleList::iterator it = std::find_if(modules.begin(), modules.end(),
                                                   boost::mem_fn(&j2::Module::is_ready));
            if (it != modules.end()) {
                (*it)->process_one();
                return boost::optional<Module*>(it->get());
            }        
            return boost::optional<Module*>();
        }
    private:
        BasicScheduler() { }
        static BasicScheduler* instance_;
    };

    inline Scheduler* BasicScheduler::instance() { 
        if (!instance_) instance_ = new BasicScheduler();
        return instance_;
    }

    class Supervisor {
    public:
        Supervisor(Scheduler *scheduler = BasicScheduler::instance(),
                   EventRouter* router = EventRouter::instance()) :
            scheduler(scheduler),
            router(router) { }

        template <class T>
        boost::shared_ptr<T> load() {
            SharedModule module(new T(router));
            modules.push_back(module);
            return boost::dynamic_pointer_cast<T>(module);
        }

        boost::optional<Module*> schedule() {
            return scheduler->schedule(modules);
        }

    private:    
        ModuleList modules;
        std::tr1::shared_ptr<EventRouter> router;
        Scheduler* scheduler;
    };

} // namespace j2

#endif // _SUPERVISOR_H
