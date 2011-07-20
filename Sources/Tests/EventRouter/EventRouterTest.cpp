#include <iostream>
#include <gtest/gtest.h>
#include <EventRouter.h>

using namespace j2;
using namespace std;

void receive_string(const std::string name, const std::string& value) {
    EXPECT_EQ("string", name);
    EXPECT_EQ("Hello World", value);
}

void receive_int(const std::string name, int value) {
    EXPECT_EQ("int", name);
    EXPECT_EQ(99, value);
}

TEST(EventRouter, can_pub_and_sub) {
    EventRouter router;
    std::string EXPECTED_STRING = "Hello World";
    int EXPECTED_INT = 99;
    int byref;
    std::tr1::shared_ptr<int> shared_ptr_int(new int());
    std::string byref_string;
    router.subscribe<std::string>("string")
        .deliver_with(receive_string)
        .assign_to(&byref_string);
    
    router.publish("string", EXPECTED_STRING);
    router.subscribe<int>("int")
        .deliver_with(receive_int)
        .assign_to(&byref)
        .assign_to(shared_ptr_int);
    router.publish("int", EXPECTED_INT); 
    EXPECT_EQ(EXPECTED_STRING, byref_string);
    EXPECT_EQ(EXPECTED_INT, byref);    
}

TEST(EventRouter, can_route_between_routers) {
    EventRouter receiver_router;
    EventRouter publisher_router;
    std::string EXPECTED_STRING = "Hello World";
    int EXPECTED_INT = 99;
    int byref;
    std::tr1::shared_ptr<int> shared_ptr_int(new int());
    std::string byref_string;

    receiver_router.subscribe<std::string>("string")
        .deliver_with(receive_string)
        .assign_to(&byref_string);

    receiver_router.subscribe<int>("int")
        .deliver_with(receive_int)
        .assign_to(&byref)
        .assign_to(shared_ptr_int);

    publisher_router.route("string", receiver_router);
    publisher_router.route("int", receiver_router);
    publisher_router.publish("string", EXPECTED_STRING);
    publisher_router.publish("int", EXPECTED_INT); 
    EXPECT_EQ(EXPECTED_STRING, byref_string);
    EXPECT_EQ(EXPECTED_INT, byref);    
}
