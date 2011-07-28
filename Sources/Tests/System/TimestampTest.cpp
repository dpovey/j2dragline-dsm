#include <gtest/gtest.h>
#include "Timestamp.h"

using namespace j2;
using boost::chrono::milliseconds;
using boost::chrono::seconds;

TEST(Timestamp, can_create_timestamp) {
    Timestamped<int> x(1);
    Timestamped<int> y(1, x);
    EXPECT_TRUE(x == y);
    EXPECT_TRUE(*x == 1);
    EXPECT_TRUE(*y == 1);

}

TEST(Timestamp, can_create_timestamped_set) {
    TimestampedSet<10, milliseconds> accurate_set;
    TimestampedSet<10, seconds> loose_set;
    Timestamped<int> x(1);
    Timestamped<float> y(1.0);
    Timestamped<double> z(1.0);
    accurate_set.add(x);
    accurate_set.add(y);
    accurate_set.add(z);
    loose_set.add(x);
    loose_set.add(y);
    loose_set.add(z);
    EXPECT_TRUE(accurate_set.is_synchronized());
    EXPECT_TRUE(accurate_set.is_synchronized());

    // Sleep for a second then update timestamp for y
    sleep(1);
    y = Timestamped<float>(1.0);

    EXPECT_FALSE(accurate_set.is_synchronized());
    EXPECT_TRUE(loose_set.is_synchronized());
    
}

