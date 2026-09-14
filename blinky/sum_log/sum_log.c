#include <zephyr/logging/log.h>
#include "sum_log.h"

/* Show all four severities for this lab; other modules default to INFO. */
LOG_MODULE_REGISTER(sum_log, LOG_LEVEL_DBG);

int sum_log(int a, int b)
{
	int result = a + b;
	int inputs[2] = {a, b};

	LOG_ERR("Example error-level message");
	LOG_WRN("Example warning-level message");
	LOG_INF("SUM_LOG: %d + %d = %d", a, b, result);
	LOG_DBG("Example debug-level message");

	LOG_HEXDUMP_INF(inputs, sizeof(inputs), "Input values");

	return result;
}
