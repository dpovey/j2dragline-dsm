#ifndef _BLOCKING_QUEUE
#define _BLOCKING_QUEUE

#include <queue>
#include <tr1/memory>
#include <boost/thread.hpp>

namespace j2 {

template<typename _Tp, class _Sequence = std::queue<_Tp> >
class blocking_queue {
public:
    typedef typename _Sequence::value_type                value_type;
    typedef typename _Sequence::reference                 reference;
    typedef typename _Sequence::const_reference           const_reference;
    typedef typename _Sequence::size_type                 size_type;
    typedef          _Sequence                            container_type;

    typedef boost::shared_mutex Mutex;
    typedef boost::shared_lock<Mutex> ReadLock;
    typedef boost::unique_lock<Mutex> WriteLock;
    
    blocking_queue() { }

    explicit
    blocking_queue(const _Sequence& sequence) : _queue(sequence) { }

    // TODO: Support timeout
    value_type deq() {
        WriteLock lock(_mutex);
        while(_queue.empty()) { _not_empty.wait(lock); }
        const_reference result = _queue.front();
        _queue.pop();
        return result;
    }

    void push(const value_type& item) {
        enq(item);
    }

    void enq(const value_type& item) {
        WriteLock lock(_mutex);
        _queue.push(item);
        _not_empty.notify_one();
    }

    size_type size() const {
        ReadLock lock(_mutex);
        return _queue.size();
    }

    bool empty() const {
        ReadLock lock(_mutex);
        return _queue.empty();
    }

protected:
    _Sequence _queue;
    mutable Mutex _mutex;
    boost::condition_variable_any _not_empty;
};

} // namespace j2

#endif // _BLOCKING_QUEUE
