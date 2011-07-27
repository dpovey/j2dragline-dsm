#include <gtest/gtest.h>
#include "Timestamp.h"

using namespace j2;

TEST(Timestamp, can_create_timestamp) {
    Timestamped<int> x(1);
    Timestamped<int> y(1, x);
    EXPECT_TRUE(x == y);
    EXPECT_TRUE(x == 1);
    EXPECT_TRUE(y == 1);

}

TEST(Timestamp, can_create_timestamped_set) {
    TimestampedSet<Timestampable> set;
    Timestamped<int> x(1);
    Timestamped<float> y(1.0);
    Timestamped<double> z(1.0);
    set.add(x);
    set.add(y);
    set.add(z);
    EXPECT_TRUE(set.all_same(boost::chrono::milliseconds(10)));
    sleep(1);
    z = Timestamped<double>(1.0);
    EXPECT_FALSE(set.all_same(boost::chrono::milliseconds(10)));
    
}

