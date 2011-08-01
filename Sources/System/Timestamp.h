#ifndef _TIMESTAMP_H
#define _TIMESTAMP_H

#include <cassert>
#include <boost/operators.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/chrono.hpp>

namespace j2 {

    class Timestampable {
    public:
        typedef boost::chrono::high_resolution_clock Clock;
        typedef Clock::time_point Timestamp;

        Timestamp virtual timestamp() const = 0;

        Timestamp virtual min() const = 0;

        Timestamp virtual max() const = 0;       

        bool operator==(const Timestampable& other) {
            return timestamp() == other.timestamp();
        }

        bool operator<(const Timestampable& other) const {
            return timestamp() < other.timestamp();
        }
    };

    /**
     * Represents a synchronized set of timestamped values 
     */
    template <int TOLERANCE = 0,
              class DURATION = boost::chrono::milliseconds,
              typename T = j2::Timestampable>
    class TimestampedSet : public Timestampable {
    public:
        TimestampedSet& add(T& timestamp) {
            _timestamps.push_back(&timestamp);
            return *this;
        }
        
        struct smallest {
            bool operator()(const T* a, const T* b) const {
                return a->min() < b->min();
            }
        };

        struct largest {
            bool operator()(const T* a, const T* b) const {
                return a->max() < b->max();
            }
        };


        Timestampable::Timestamp min() const { 
            if (_timestamps.empty()) return Timestampable::Timestamp::min();
            return (*std::min_element(_timestamps.begin(), _timestamps.end(), 
                                      typename TimestampedSet::smallest()))->timestamp();
        }

        Timestampable::Timestamp max() const { 
            if (_timestamps.empty()) return Timestampable::Timestamp::min();
            return (*std::max_element(_timestamps.begin(), _timestamps.end(), 
                                      typename TimestampedSet::largest()))->timestamp();
        }

        TimestampedSet& operator+=(T& timestamped) {
            add(timestamped);
            return *this;
        }

        TimestampedSet& operator,(T& timestamped) {
            add(timestamped);
            return *this;
        }

        template <class Rep, class Period>
        boost::chrono::duration<Rep, Period> jitter() const {
            return boost::chrono::duration<Rep,Period>(max() - min());
        }

        template <class Rep, class Period>
        bool is_synchronized(boost::chrono::duration<Rep, Period> tolerance) const {
            Timestampable::Timestamp min_timestamp = min();
            Timestampable::Timestamp max_timestamp = max();
            return min_timestamp > Timestampable::Timestamp::min() &&
                (max_timestamp - min_timestamp) <= tolerance;
        }

        bool is_synchronized() const {
            return is_synchronized(DURATION(TOLERANCE));
        }

        // Returns the (most recent) timestamp for this set
        virtual typename Timestampable::Timestamp timestamp() const {
            return max();
        }

    private:
        std::vector<T*> _timestamps;
    };

    template <typename T>
    class Timestamped : public Timestampable {
    public:
        typedef T value_type;
        typedef T& reference;
        typedef const T& const_reference;

        Timestamped() : _timestamp(Timestampable::Timestamp::min()) { }

        Timestamped(const T& value) : _value(value), _timestamp(Timestampable::Clock::now()) { }

        Timestamped(const T& value, Timestamp timestamp) : 
            _timestamp(timestamp),
            _value(value) { }

        Timestamped(const T& value, const Timestampable& timestampable) : 
            _timestamp(timestampable.timestamp()),
            _value(value) { }

        operator value_type() { return _value; }

        const T* operator->() const { return &_value; } 

        T* operator->() { return &_value; } 

        const T& operator*() const { return _value; } 

        T& operator*() { return _value; }

        const_reference value() const { return _value; }

        reference value() { return _value; }

        bool operator==(const Timestamped<T>& other) const {
            return value() == other.value() && timestamp() == other.timestamp();
        }

        Timestampable::Timestamp virtual timestamp() const { return _timestamp; }

        Timestamp virtual min() const { return timestamp(); }

        Timestamp virtual max() const { return timestamp(); }

    protected:
        Timestampable::Timestamp _timestamp;
        value_type _value;
    };

} // namespace j2

#endif // _TIMESTAMP_H
