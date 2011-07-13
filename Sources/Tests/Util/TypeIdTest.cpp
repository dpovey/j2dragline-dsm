#include <gtest/gtest.h>

template <typename T>
class My {
public:
    template <typename T2>
    bool hasElement() { return typeid(T2) == typeid(T); }
    T data;
};

TEST(typeid, some_stuff) {
    My<int> m;
    m.data = 1;
    printf("%s:%d\n", typeid(m).name(), m.hasElement<int>());
}
