#include <iostream>
#include <gtest/gtest.h>
#include <EventRouter.h>

using namespace j2;
using namespace std;

static std::string received_string;
static int received_int;

void receive_string(boost::any value) {
    received_string = boost::any_cast<std::string>(value);
}

void receive_int(boost::any value) {
   received_int = boost::any_cast<int>(value);
}

TEST(EventRouter, can_pub_and_sub) {
    EventRouter router;
    std::string EXPECTED_STRING = "Hello World";
    int EXPECTED_INT = 99;
    int byref;
    std::tr1::shared_ptr<int> shared_ptr_int(new int());
    std::string byref_string;
    router.subscribe<std::string>("string").deliverResultWith(receive_string);
    router.subscribe<std::string>("string").assignResultTo(&byref_string);
    router.publish("string", EXPECTED_STRING);
    router.subscribe<int>("int")
        .deliverResultWith(receive_int)
        .assignResultTo(&byref)
        .assignResultTo(shared_ptr_int);
    router.publish("int", EXPECTED_INT); 
    EXPECT_EQ(EXPECTED_STRING, received_string);
    EXPECT_EQ(EXPECTED_STRING, byref_string);
    EXPECT_EQ(EXPECTED_INT, received_int);
    EXPECT_EQ(EXPECTED_INT, byref);    
}
