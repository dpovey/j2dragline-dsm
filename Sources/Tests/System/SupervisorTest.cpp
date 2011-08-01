#include <gtest/gtest.h>
#include <Supervisor.h>

using namespace j2;
using namespace std;
using namespace std::tr1;

class TestModule : public Module {
public:
    void setString(const std::string& value) {
        stringValue = value;
    }
    
    TestModule(std::tr1::shared_ptr<EventRouter> router) :
        Module(router),
        stringValue("") {
        bind_value("string", &stringValue);
        bind_fn<std::string>("set_string",
                boost::bind(&TestModule::setString, this, _1));
    }
    
    std::string stringValue;
};


TEST(Supervisor, can_instantiate_module) {
    EventRouter *router = new EventRouter;
    Supervisor supervisor(BasicScheduler::instance(), router);
    boost::shared_ptr<TestModule> module = supervisor.load<TestModule>();   
    EXPECT_FALSE(supervisor.schedule());
    router->publish("string", std::string("Hello World"));
    EXPECT_EQ(module.get(), supervisor.schedule().get());
    EXPECT_EQ("Hello World", module->stringValue);    
}
