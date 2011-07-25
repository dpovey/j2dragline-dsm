#include <functional>
#include <boost/any.hpp>
#include <boost/bind.hpp>
#include <gtest/gtest.h>
#include <Module.h>

using namespace j2;
using namespace std;

class TestModule : public Module {
public:
    void setString(const std::string& value) {
        puts("Called set string value");
        stringValue = value;
    }
    
    TestModule(EventRouter* router) :
        Module(router),
        stringValue("") {
        bind_value("string", &stringValue);
        bind_fn<std::string>("set_string",
                boost::bind(&TestModule::setString, this, _1));
    }
    
    std::string stringValue;
};


TEST(Module, can_create_and_receive_messages) {

    EventRouter* router = new EventRouter();
    TestModule test(router);
    EXPECT_EQ("", test.stringValue);
    router->publish("string", std::string("Hello World"));
    EXPECT_EQ("", test.stringValue);
    test.process_all();
    EXPECT_EQ("Hello World", test.stringValue);
    test.stringValue = "";
    router->publish("set_string", std::string("Hello Again World"));
    EXPECT_EQ("", test.stringValue);
    test.process_all();
    EXPECT_EQ("Hello Again World", test.stringValue);
}
