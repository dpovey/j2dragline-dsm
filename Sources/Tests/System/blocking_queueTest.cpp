#include <gtest/gtest.h>
#include "blocking_queue.h"

using namespace std;
using namespace j2;

TEST(blocking_queue, can_enq_and_deq_without_blocking) {
    j2::blocking_queue<int> queue;
    queue.enq(99);
    EXPECT_EQ(1, (queue.size()));
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(99, queue.deq());
    EXPECT_EQ(0, queue.size());
    EXPECT_TRUE(queue.empty());
}
