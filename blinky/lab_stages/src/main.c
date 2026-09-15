#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/sys/printk.h>

#if defined(CONFIG_LAB_BUTTON_DEMO)
#define DEMO_LED_NODE DT_ALIAS(led5180)
static const struct gpio_dt_spec button =
	GPIO_DT_SPEC_GET(DT_ALIAS(button5180), gpios);
#else
/* Part 2 keeps the original Blinky LED; Part 5 switches to LED2. */
#define DEMO_LED_NODE DT_ALIAS(led0)
#endif

static const struct gpio_dt_spec led =
	GPIO_DT_SPEC_GET(DEMO_LED_NODE, gpios);

int main(void)
{
	if (!gpio_is_ready_dt(&led)) {
		printk("LED GPIO is not ready\n");
		return 0;
	}
	int ret = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		printk("LED configuration failed: %d\n", ret);
		return 0;
	}
	bool on = false;

#if defined(CONFIG_LAB_BUTTON_DEMO)
	if (!gpio_is_ready_dt(&button)) {
		printk("Button GPIO is not ready\n");
		return 0;
	}
	ret = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (ret < 0) {
		printk("Button configuration failed: %d\n", ret);
		return 0;
	}
	bool was_pressed = false;
	while (1) {
		int pressed = gpio_pin_get_dt(&button);
		if (pressed < 0) {
			printk("Button read failed: %d\n", pressed);
			return 0;
		}
		if (pressed && !was_pressed) {
			ret = gpio_pin_toggle_dt(&led);
			if (ret < 0) {
				printk("LED toggle failed: %d\n", ret);
				return 0;
			}
			on = !on;
			printk("Part 5 LED2: %s\n", on ? "ON" : "OFF");
		}
		was_pressed = pressed;
		k_msleep(20);
	}
#else
	while (1) {
		ret = gpio_pin_toggle_dt(&led);
		if (ret < 0) {
			printk("LED toggle failed: %d\n", ret);
			return 0;
		}
		on = !on;
		printk("Blinky LED: %s; toggle interval: %d ms\n",
		       on ? "ON" : "OFF", CONFIG_LAB_BLINK_INTERVAL_MS);
		k_msleep(CONFIG_LAB_BLINK_INTERVAL_MS);
	}
#endif
}
