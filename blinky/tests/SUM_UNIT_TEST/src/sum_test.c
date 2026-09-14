#include <zephyr/ztest.h>
#include "sum_log.h"

ZTEST(sum_log_test_suite, test_sum_log_basic)
{
	zassert_equal(sum_log(3, 5), 8, "3 + 5 should equal 8");
	zassert_equal(sum_log(5, 3), 8, "5 + 3 should equal 8");
}

ZTEST(sum_log_test_suite, test_sum_log_negative)
{
	zassert_equal(sum_log(-3, -5), -8, "Two negative inputs");
	zassert_equal(sum_log(-3, 5), 2, "Mixed signs with positive result");
	zassert_equal(sum_log(3, -5), -2, "Mixed signs with negative result");
}

ZTEST(sum_log_test_suite, test_sum_log_zero)
{
	zassert_equal(sum_log(0, 0), 0, "Both inputs zero");
	zassert_equal(sum_log(0, 5), 5, "Left input zero");
	zassert_equal(sum_log(-5, 0), -5, "Right input zero");
	zassert_equal(sum_log(5, -5), 0, "Opposite inputs cancel");
}

ZTEST_SUITE(sum_log_test_suite, NULL, NULL, NULL, NULL, NULL);
