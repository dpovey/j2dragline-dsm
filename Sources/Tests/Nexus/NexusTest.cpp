#include <iostream>
#include <gtest/gtest.h>
#include <Nexus.h>

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

TEST(Nexus, can_pub_and_sub) {
    Nexus nexus;
    std::string EXPECTED_STRING = "Hello World";
    int EXPECTED_INT = 99;
    int byref;
    std::tr1::shared_ptr<int> shared_ptr_int(new int());
    std::string byref_string;
    nexus.subscribe<std::string>("string").deliverResultWith(receive_string);
    nexus.subscribe<std::string>("string").assignResultTo(&byref_string);
    nexus.publish("string", EXPECTED_STRING);
    nexus.subscribe<int>("int")
        .deliverResultWith(receive_int)
        .assignResultTo(&byref)
        .assignResultTo(shared_ptr_int);
    nexus.publish("int", EXPECTED_INT); 
    EXPECT_EQ(EXPECTED_STRING, received_string);
    EXPECT_EQ(EXPECTED_STRING, byref_string);
    EXPECT_EQ(EXPECTED_INT, received_int);
    EXPECT_EQ(EXPECTED_INT, byref);    
}
