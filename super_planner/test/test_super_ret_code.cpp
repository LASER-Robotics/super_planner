#include <gtest/gtest.h>
#include <super_core/super_ret_code.hpp>

using namespace super_planner;

TEST(SuperReturnCode, PreservesKnownDescriptions) {
    EXPECT_EQ(SUPER_RET_CODE_STR(SUPER_SUCCESS_WITH_BACKUP),
              "Success, with backup trajectory also success");
    EXPECT_EQ(SUPER_RET_CODE_STR(SUPER_SUCCESS_NO_BACKUP),
              "Success, without need of backup");
    EXPECT_EQ(SUPER_RET_CODE_STR(SUPER_SUCCESS), "Success");
    EXPECT_EQ(SUPER_RET_CODE_STR(SUPPER_UNDEFINED), "Undefined");
    EXPECT_EQ(SUPER_RET_CODE_STR(SUPER_NO_ODOM),
              "No odom, return at the start of the replan");
    EXPECT_EQ(SUPER_RET_CODE_STR(SUPER_NO_START_POINT),
              "Cannot find a start point in the local map");
}

TEST(SuperReturnCode, HandlesUnknownValues) {
    EXPECT_EQ(SUPER_RET_CODE_STR(99), "Undefined");
    EXPECT_EQ(SUPER_RET_CODE_STR(-99), "Undefined");
}
