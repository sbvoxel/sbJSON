#include "common.h"
#include "unity.h"

union Caster {
    double d;
    uint64_t u;
};

static bool bitcompare_double(double a, double b) {
    union Caster ca = (union Caster) {.d = a};
    union Caster cb = (union Caster) {.d = b};
    return ca.u == cb.u;
}

static bool test_double(char const* str, double expected) {
    sbJSON *json = sbj_parse(str);

    if (!json) {
        return false;
    }

    if (!sbj_is_array(json)) {
        return false;
    }

    if (!sbj_is_number(json->child)) {
        return false;
    }

    sbJSON* item = json->child;

    if (!item->is_number_double) {
        return false;
    }

    return bitcompare_double(item->u.valuedouble, expected);
}

// https://raw.githubusercontent.com/miloyip/nativejson-benchmark/master/src/main.cpp
static void parse_double() {
    TEST_ASSERT_TRUE(test_double("[0.0]", 0.0));
    TEST_ASSERT_TRUE(test_double("[-0.0]", -0.0));
    TEST_ASSERT_TRUE(test_double("[1.0]", 1.0));
    TEST_ASSERT_TRUE(test_double("[-1.0]", -1.0));
    TEST_ASSERT_TRUE(test_double("[1.5]", 1.5));
    TEST_ASSERT_TRUE(test_double("[-1.5]", -1.5));
    TEST_ASSERT_TRUE(test_double("[3.1416]", 3.1416));
    TEST_ASSERT_TRUE(test_double("[1E10]", 1E10));
    TEST_ASSERT_TRUE(test_double("[1e10]", 1e10));
    TEST_ASSERT_TRUE(test_double("[1E+10]", 1E+10));
    TEST_ASSERT_TRUE(test_double("[1E-10]", 1E-10));
    TEST_ASSERT_TRUE(test_double("[-1E10]", -1E10));
    TEST_ASSERT_TRUE(test_double("[-1e10]", -1e10));
    TEST_ASSERT_TRUE(test_double("[-1E+10]", -1E+10));
    TEST_ASSERT_TRUE(test_double("[-1E-10]", -1E-10));
    TEST_ASSERT_TRUE(test_double("[1.234E+10]", 1.234E+10));
    TEST_ASSERT_TRUE(test_double("[1.234E-10]", 1.234E-10));
    TEST_ASSERT_TRUE(test_double("[1.79769e+308]", 1.79769e+308));
    TEST_ASSERT_TRUE(test_double("[2.22507e-308]", 2.22507e-308));
    TEST_ASSERT_TRUE(test_double("[-1.79769e+308]", -1.79769e+308));
    TEST_ASSERT_TRUE(test_double("[-2.22507e-308]", -2.22507e-308));
    TEST_ASSERT_TRUE(test_double("[4.9406564584124654e-324]", 4.9406564584124654e-324)); // minimum denormal
    TEST_ASSERT_TRUE(test_double("[2.2250738585072009e-308]", 2.2250738585072009e-308)); // Max subnormal double
    TEST_ASSERT_TRUE(test_double("[2.2250738585072014e-308]", 2.2250738585072014e-308)); // Min normal positive double
    TEST_ASSERT_TRUE(test_double("[1.7976931348623157e+308]", 1.7976931348623157e+308)); // Max double
    TEST_ASSERT_TRUE(test_double("[1e-10000]", 0.0));                                   // must underflow
    TEST_ASSERT_TRUE(test_double("[18446744073709551616]", 18446744073709551616.0));    // 2^64 (max of uint64_t + 1, force to use double)
    TEST_ASSERT_TRUE(test_double("[-9223372036854775809]", -9223372036854775809.0));    // -2^63 - 1(min of int64_t + 1, force to use double)
    TEST_ASSERT_TRUE(test_double("[0.9868011474609375]", 0.9868011474609375));          // https://github.com/miloyip/rapidjson/issues/120
    TEST_ASSERT_TRUE(test_double("[123e34]", 123e34));                                  // Fast Path Cases In Disguise
    TEST_ASSERT_TRUE(test_double("[45913141877270640000.0]", 45913141877270640000.0));
    TEST_ASSERT_TRUE(test_double("[2.2250738585072011e-308]", 2.2250738585072011e-308)); // http://www.exploringbinary.com/php-hangs-on-numeric-value-2-2250738585072011e-308/
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(parse_double);
    return UNITY_END();
}
