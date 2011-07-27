#ifndef _TIMESTAMP_H
#define _TIMESTAMP_H

#include <cassert>
#include <boost/operators.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/chrono.hpp>

namespace j2 {

    class Timestampable : public boost::operators< Timestampable > {
        typedef boost::chrono::high_resolution_clock Clock;
        typedef Clock::time_point Timestamp;
    public:
        Timestampable() :
            _timestamp(Clock::now()) { }
        Timestampable (Timestamp timestamp) : 
            _timestamp(timestamp) { }        
        Timestamp timestamp() const { return _timestamp; }

        bool operator==(const Timestampable& other) const {
            return timestamp() == other.timestamp();
        }

        bool operator<(const Timestampable& other) const {
            return timestamp() < other.timestamp();
        }

    protected:
        Timestamp _timestamp;
    };

    template <typename T>
    class Timestamped : public Timestampable, 
                        public boost::operators< Timestamped<T> > {
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

    /**
     * Represents a synchronized set of timestamped values 
     */
    template <int TOLERANCE = 0,
              class DURATION = boost::chrono::milliseconds,
              typename T = j2::Timestampable>
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
        bool synchronized(boost::chrono::duration<Rep, Period> tolerance) const {
            return (max().timestamp() - min().timestamp()) <= tolerance;
        }

        bool synchronized() {
            return synchronized(DURATION(TOLERANCE));
        }

    private:
        void sort() const { 
            std::sort(_timestamps.begin(), _timestamps.end(), 
                      *boost::lambda::_1 < *boost::lambda::_2);
        }
        mutable std::vector<T*> _timestamps;
    };
    

} // namespace j2

#endif // _TIMESTAMP_H
