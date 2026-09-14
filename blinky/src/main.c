/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#ifdef CONFIG_SUM_PRINT
#include "sum_printk.h"
#elif defined(CONFIG_SUM_LOG)
#include "sum_log.h"
#endif

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   2000
#define BUTTON_POLL_TIME_MS 20

/* The devicetree node identifier for the "led0" alias. */
//#define LED0_NODE DT_ALIAS(led0)
//#define BUTTON_NODE DT_ALIAS(sw0)
#define LED5180_NODE DT_ALIAS(led5180)
#define BUTTON5180_NODE DT_ALIAS(button5180)

/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
*/
//static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED5180_NODE, gpios);

static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON5180_NODE, gpios);

int main(void)
{
	int ret;
	int button_state;
	int sum_result;
	bool led_state = true;
	bool last_button_state = false;

	/* Light LED2 before sleeping or printing to diagnose startup. */
	if (!gpio_is_ready_dt(&led)) {
		printf("LED device is not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printf("Failed to configure LED\n");
		return 0;
	}

	/* Wait five seconds for the serial monitor to reconnect. */
	k_msleep(5000);

	/* Part 6: run the selected sum implementation once. */
#ifdef CONFIG_SUM_PRINT
	sum_result = sum_printk(3, 5);
#elif defined(CONFIG_SUM_LOG)
	sum_result = sum_log(3, 5);
#endif
	(void)sum_result;

	/* Part 5: button setup. */
	if (!gpio_is_ready_dt(&button)) {
		printf("Button device is not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		printf("Failed to configure button\n");
		return 0;
	}

	while (1) {
		button_state = gpio_pin_get_dt(&button);

		if (button_state < 0) {
			printf("Failed to read button\n");
			return 0;
		}

		if (button_state && !last_button_state) {
			ret = gpio_pin_toggle_dt(&led);

			if (ret < 0) {
				printf("Failed to toggle LED\n");
				return 0;
			}

			led_state = !led_state;
			printf("LED state: %s\n",
			       led_state ? "ON" : "OFF");
		}

		last_button_state = button_state;
		k_msleep(BUTTON_POLL_TIME_MS);
	}

	return 0;
}
