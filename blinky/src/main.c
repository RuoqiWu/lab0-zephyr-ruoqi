/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   2000
#define BUTTON_POLL_TIME_MS 20

/* The devicetree node identifier for the "led0" alias. */
//#define LED0_NODE DT_ALIAS(led0)
#define LED5180_NODE DT_ALIAS(led5180)
#define BUTTON_NODE DT_ALIAS(sw0)


/*
 * A build error on this line means your board is unsupported.
 * See the sample documentation for information on how to fix this.
 */
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED5180_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);

int main(void)
{
	int ret;
	int button_state;
	bool led_state = true;
	bool last_button_state = false;

	if (!gpio_is_ready_dt(&led)) {
		printf("LED device not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (ret < 0) {
		printf("Failed to configure LED pin\n");
		return 0;
	}

	if (!gpio_is_ready_dt(&button)) {
		printf("Button device is not ready\n");
		return 0;
	}

	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		printf("Failed to configure button pin\n");
		return 0;
	}

while (1) {
		/* Poll the current button state */
		button_state = gpio_pin_get_dt(&button);

		if (button_state < 0) {
			printf("Failed to read button\n");
			return 0;
		}

		/* Toggle only when the button changes from released to pressed */
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

		/* Short delay for polling and basic button debouncing */
		k_msleep(BUTTON_POLL_TIME_MS);
	}

	return 0;
}