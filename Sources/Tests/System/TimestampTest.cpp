#include <gtest/gtest.h>
#include <boost/thread.hpp>
#include <boost/random.hpp>
#include "Timestamp.h"

using namespace j2;
using boost::chrono::milliseconds;
using boost::chrono::seconds;

static boost::mt19937 gen;

TEST(Timestamp, can_create_timestamp) {
    Timestamped<int> x(1);
    Timestamped<int> y(1, x);
    EXPECT_TRUE(x == y);
    EXPECT_TRUE(*x == 1);
    EXPECT_TRUE(*y == 1);

}

static void sleep_ms(int ms) {
    boost::this_thread::sleep(boost::posix_time::milliseconds(ms)); 
}

static void random_jitter(int max_jitter_ms) {
    boost::random::uniform_int_distribution<int> dist(0, max_jitter_ms);
    sleep_ms(dist(gen));
}

TEST(Timestamp, can_create_timestamped_set) {
    TimestampedSet<50, milliseconds> accurate_set;
    TimestampedSet<10, seconds> loose_set;
    Timestamped<int> x(1);
    random_jitter(1);
    Timestamped<float> y(1.0);
    random_jitter(1);
    Timestamped<double> z(1.0);
    accurate_set += x, y, z;
    loose_set += x, y, z;
    EXPECT_TRUE(accurate_set.is_synchronized());
    EXPECT_TRUE(loose_set.is_synchronized());

    sleep_ms(100);
    y = Timestamped<float>(1.0);
    EXPECT_FALSE(accurate_set.is_synchronized());
    EXPECT_TRUE(loose_set.is_synchronized());
}

TEST(Timestamp, can_nest_timestamped_set) {
    TimestampedSet<50, milliseconds> _10_hz;
    Timestamped<int> a(1);
    Timestamped<float> b(1.0);
    Timestamped<double> c(1.0);
    _10_hz += a, b, c;

    TimestampedSet<25, milliseconds> _20_hz;
    Timestamped<int> x(1);
    Timestamped<float> y(1.0);
    Timestamped<double> z(1.0);
    _20_hz += x, y, z;

    TimestampedSet<50, milliseconds> all;
    all += _10_hz, _20_hz;

    for (int i=0; i < 20; i++) {
        random_jitter(50);
        x = Timestamped<int>(i);
        random_jitter(5);
        y = Timestamped<float>(i);
        random_jitter(5);
        z = Timestamped<double>(i);
        random_jitter(5);
        EXPECT_TRUE(_20_hz.is_synchronized());
        if (i % 2) {
            random_jitter(5);                    
            a = Timestamped<int>(i);
            random_jitter(5);                    
            b = Timestamped<float>(i);
            random_jitter(5);
            c = Timestamped<double>(i);
            EXPECT_TRUE(_10_hz.is_synchronized());
            EXPECT_TRUE(all.is_synchronized());
        }
    }
}



