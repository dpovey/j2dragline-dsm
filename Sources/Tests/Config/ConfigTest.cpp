#include "config.h"
#include "gtest/gtest.h"

TEST(Config, has_version) {
  EXPECT_GE(4, J2DRAGLINE_VERSION_MAJOR);
}
