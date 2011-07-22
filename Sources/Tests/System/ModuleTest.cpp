#include <gtest/gtest.h>
#include <Module.h>

using namespace j2;
using namespace std;

TEST(Module, can_create_and_receive_messages) {
    class TestModule : public Module {
    public:
        TestModule(EventRouter* router) :
            Module(router) {
            bind_value("string", &stringValue);
        }
        std::string stringValue;
    };

    EventRouter* router = new EventRouter();
    TestModule test(router);
    router->publish("string", std::string("Hello World"));
    EXPECT_EQ("Hello World", test.stringValue);
}
