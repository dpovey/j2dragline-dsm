#ifndef _TIMESTAMP_H
#define _TIMESTAMP_H

#include <boost/lambda/lambda.hpp>
#include <boost/chrono.hpp>

namespace j2 {

    class Timestampable {
        typedef boost::chrono::high_resolution_clock Clock;
        typedef Clock::time_point Timestamp;
    public:
        Timestampable() :
            _timestamp(Clock::now()) { }
        Timestampable (Timestamp timestamp) : 
            _timestamp(timestamp) { }        
        Timestamp timestamp() const { return _timestamp; }
    protected:
        Timestamp _timestamp;
    };

    template <typename T>
    class Timestamped : public Timestampable {
    public:
        typedef T value_type;
        typedef T& reference;
        typedef const T& const_reference;

        Timestamped(const T& value) : _value(value) { }

        Timestamped(const T& value, Timestamp timestamp) : 
            Timestampable(timestamp),
            _value(value) { }

        Timestamped(const T& value, const Timestampable& timestampable) : 
            Timestampable(timestampable.timestamp()),
            _value(value) { }

        operator value_type() { return _value; }

        const_reference value() const { return _value; }

        reference value() { return _value; }

        bool operator==(const Timestamped<T>& other) const {
            return value() == other.value() && timestamp() == other.timestamp();
        }

        bool operator==(const T& other) const {
            return value() == other;
        }


    private:
        value_type _value;
    };


    template <typename T>
    class TimestampedSet {
    public:
        void add(T& timestamp) {
            _timestamps.push_back(&timestamp);
        }

        T min() const { 
            sort();
            return *_timestamps.front();
        }

        T max() const { 
            sort();
            return *_timestamps.back();
        }

        template <class Rep, class Period>
        boost::chrono::duration<Rep, Period> difference() const {
            return boost::chrono::duration<Rep,Period>(max().timestamp() - min().timestamp());
        }

        template <class Rep, class Period>
        bool all_same(boost::chrono::duration<Rep, Period> tolerance) const {
            return (max().timestamp() - min().timestamp()) <= tolerance;
        }

        bool all_same() {
            return all_same(boost::chrono::milliseconds(0));
        }

    private:

        struct ptr_to_timestamp_less_than
        {
            const bool operator()(const T *a, const T * b) const {
                // check for 0
                if (a == 0) {
                    return b != 0; // if b is also 0, then they are equal, hence a is not < than b
                } else if (b == 0) {
                    return false;
                } else {
                    return a->timestamp() < b->timestamp();
                }
            }
        };

        void sort() const { 
            std::sort(_timestamps.begin(), _timestamps.end(), 
                      typename TimestampedSet::ptr_to_timestamp_less_than()); }
        mutable std::vector<T*> _timestamps;
    };
    

} // namespace j2

#endif // _TIMESTAMP_H
