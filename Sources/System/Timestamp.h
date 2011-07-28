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

        T& min() const { 
            sort();
            return *_timestamps.front();
        }

        T& max() const { 
            sort();
            return *_timestamps.back();
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
            return boost::chrono::duration<Rep,Period>(max().timestamp() - min().timestamp());
        }

        template <class Rep, class Period>
        bool is_synchronized(boost::chrono::duration<Rep, Period> tolerance) const {
            sort();
            Timestampable::Timestamp min = _timestamps.front()->timestamp();
            Timestampable::Timestamp max = _timestamps.back()->timestamp();
            return min > Timestampable::Timestamp::min() &&
                (max - min) <= tolerance;
        }

        bool is_synchronized() const {
            return is_synchronized(DURATION(TOLERANCE));
        }

        // Returns the (most recent) timestamp for this set
        virtual typename Timestampable::Timestamp timestamp() const {
            return max().timestamp();
        }

    private:
        void sort() const { 
            std::sort(_timestamps.begin(), _timestamps.end(),
                      *boost::lambda::_1 < *boost::lambda::_2);
        }
        mutable std::vector<T*> _timestamps;
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

    protected:
        Timestampable::Timestamp _timestamp;
        value_type _value;
    };

} // namespace j2

#endif // _TIMESTAMP_H
