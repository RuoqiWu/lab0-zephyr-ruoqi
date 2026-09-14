#include "bme280_raw.h"
#include <errno.h>

static uint16_t le16(const uint8_t *b)
{
 return (uint16_t)b[0] | ((uint16_t)b[1] << 8);
}

static int wait_clear(struct bme_raw *s, uint8_t mask)
{
 for (int i = 0; i < 50; ++i) {
  uint8_t status;
  int ret = s->bus.read(s->bus.ctx, 0xf3, &status, 1);
  if (ret) { return ret; }
  if (!(status & mask)) { return 0; }
  s->bus.sleep_ms(2);
 }
 return -ETIMEDOUT;
}

int bme_raw_init(struct bme_raw *s, struct bme_bus bus)
{
 if (!s || !bus.read || !bus.write || !bus.sleep_ms) { return -EINVAL; }
 *s = (struct bme_raw){.bus = bus};
 uint8_t id, cal[6];
 int ret = bus.read(bus.ctx, 0xd0, &id, 1);
 if (ret) { return ret; }
 if (id != 0x60) { return -ENODEV; }
 ret = bus.write(bus.ctx, 0xe0, 0xb6);
 if (ret) { return ret; }
 bus.sleep_ms(3);
 ret = wait_clear(s, 1); /* Wait for NVM calibration copy. */
 if (ret) { return ret; }
 ret = bus.read(bus.ctx, 0x88, cal, sizeof(cal));
 if (ret) { return ret; }
 s->t1 = le16(cal);
 s->t2 = (int16_t)le16(cal + 2);
 s->t3 = (int16_t)le16(cal + 4);
 if (s->t1 == 0 || s->t1 == UINT16_MAX || s->t2 == 0) { return -EINVAL; }
 ret = bus.write(bus.ctx, 0xf2, 0); /* Humidity skipped. */
 if (ret) { return ret; }
 s->ready = true;
 return 0;
}

int bme_raw_temperature(struct bme_raw *s, int32_t *centi_c)
{
 if (!s || !centi_c || !s->ready) { return -EINVAL; }
 /* Temperature x1, pressure skipped, forced measurement. */
 int ret = s->bus.write(s->bus.ctx, 0xf4, 0x21);
 if (ret) { return ret; }
 s->bus.sleep_ms(10); /* Exceeds the temperature-only x1 conversion time. */
 ret = wait_clear(s, 8);
 if (ret) { return ret; }
 uint8_t b[3];
 ret = s->bus.read(s->bus.ctx, 0xfa, b, sizeof(b));
 if (ret) { return ret; }
 int32_t adc = ((uint32_t)b[0] << 12) | ((uint32_t)b[1] << 4) | (b[2] >> 4);
 if (adc == 0x80000) { return -ENODATA; }
 /* Bosch integer compensation, with widened intermediates. Result: 0.01 C. */
 int64_t v1 = ((((int64_t)adc >> 3) - (int64_t)s->t1 * 2) * s->t2) >> 11;
 int64_t d = ((int64_t)adc >> 4) - s->t1;
 int64_t v2 = (((d * d) >> 12) * s->t3) >> 14;
 int64_t t = ((v1 + v2) * 5 + 128) >> 8;
 if (t < -4000 || t > 8500) { return -ERANGE; }
 *centi_c = (int32_t)t;
 return 0;
}
