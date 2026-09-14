#include <zephyr/kernel.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/sys/printk.h>
#include "bme280_raw.h"

#define BME_NODE DT_NODELABEL(bme280_lab)
BUILD_ASSERT(DT_NODE_HAS_STATUS(BME_NODE, okay), "BME280 node must be enabled");
BUILD_ASSERT(DT_REG_ADDR(BME_NODE) == 0x77, "Expected BME280 address 0x77");
static const struct i2c_dt_spec sensor = I2C_DT_SPEC_GET(BME_NODE);
static int read_reg(void *ctx, uint8_t reg, uint8_t *data, size_t len)
{
 return i2c_burst_read_dt(ctx, reg, data, len);
}
static int write_reg(void *ctx, uint8_t reg, uint8_t value)
{
 return i2c_reg_write_byte_dt(ctx, reg, value);
}
static void delay(unsigned int ms) { k_msleep(ms); }
static void sensor_thread(void *a, void *b, void *c)
{
 ARG_UNUSED(a); ARG_UNUSED(b); ARG_UNUSED(c);
 struct bme_raw s = {0};
 struct bme_bus bus = {.ctx = (void *)&sensor, .read = read_reg,
                      .write = write_reg, .sleep_ms = delay};
 k_msleep(5500);
 printk("BME280: I2C1 SDA=P1.15 SCL=P1.14 address=0x77\n");
 while (1) {
  if (!i2c_is_ready_dt(&sensor)) {
   printk("BME280: I2C controller not ready\n");
   k_msleep(5000); continue;
  }
  if (!s.ready) {
   int ret = bme_raw_init(&s, bus);
   if (ret) {
    uint8_t id = 0;
    int alt = i2c_burst_read(sensor.bus, 0x76, 0xd0, &id, 1);
    printk("BME280: init failed (%d); check supply, GND, SDA/SCL\n", ret);
    if (!alt && id == 0x60) {
     printk("BME280: found at 0x76; change devicetree address to match module\n");
    }
    k_msleep(5000); continue;
   }
   printk("BME280: chip ID=0x60, calibration T1=%u T2=%d T3=%d\n",
          s.t1, s.t2, s.t3);
  }
  int32_t t;
  int ret = bme_raw_temperature(&s, &t);
  if (ret) {
   printk("BME280: temperature read failed (%d)\n", ret);
   s.ready = false;
  } else {
   int32_t magnitude = t < 0 ? -t : t;
   printk("Temperature: %s%d.%02d C\n", t < 0 ? "-" : "",
          magnitude / 100, magnitude % 100);
  }
  k_msleep(2000);
 }
}
K_THREAD_DEFINE(bme_thread, 1536, sensor_thread, NULL, NULL, NULL, 7, 0, 0);
