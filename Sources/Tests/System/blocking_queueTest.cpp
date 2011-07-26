#include <tr1/memory>
#include <gtest/gtest.h>
#include "blocking_queue.h"

using namespace std;
using namespace j2;

TEST(blocking_queue, can_enq_and_deq_without_blocking) {
    blocking_queue<int> queue;
    queue.enq(99);
    EXPECT_EQ(1, (queue.size()));
    EXPECT_FALSE(queue.empty());
    EXPECT_EQ(99, queue.deq());
    EXPECT_EQ(0, queue.size());
    EXPECT_TRUE(queue.empty());
}

class producer {
public:
    producer(blocking_queue<int> &queue, int value) : queue(queue), value(value) { }
    void operator()() {
        queue.enq(value);
    }
    blocking_queue<int>& queue;
    int value;
};

class consumer {
public:
    consumer(blocking_queue<int> &queue, int *result) : queue(queue), result(result) { }

    void operator()() {
        *result = queue.deq();
    }
    blocking_queue<int>& queue;
    int *result;
};


TEST(blocking_queue, can_enq_in_one_thread_deq_in_other) {
    blocking_queue<int> queue;
    int result;

    producer p(queue, 99);
    consumer c(queue, &result);
    boost::thread c_thread(c);
    boost::thread p_thread(p);
    p_thread.join();
    c_thread.join();
    EXPECT_EQ(99, result);
}

static void join_and_free(boost::thread* t) {
    t->join();
    delete t;
}

TEST(blocking_queue, can_have_multiple_readers_and_writers) {
    blocking_queue<int> queue;
    int result;

    vector< boost::thread* > threads;
    vector< int* > results;
    for (int i=0; i<32; i++) {
        producer p(queue, i);
        threads.push_back(new boost::thread(p));
    }
    for (int i=0; i<32; i++) {
        int* i = new int;
        results.push_back(i);
        consumer c(queue, i);
        threads.push_back(new boost::thread(c));
    }

    // Wait for threads to complete
    for_each(threads.begin(), threads.end(), &join_and_free);

    // Check results
    uint32_t got_result;
    for (vector< int* >::iterator it = results.begin();
         it != results.end();
         ++it) {
        got_result |= (1 << *(*it));
        delete *it;
    }

    EXPECT_EQ(0xffffffff, got_result);
}

