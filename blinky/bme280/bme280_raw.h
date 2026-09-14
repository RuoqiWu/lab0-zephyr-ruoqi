#ifndef LAB_BME280_RAW_H
#define LAB_BME280_RAW_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
struct bme_bus {
 void *ctx;
 int (*read)(void *, uint8_t, uint8_t *, size_t);
 int (*write)(void *, uint8_t, uint8_t);
 void (*sleep_ms)(unsigned int);
};
struct bme_raw {
 struct bme_bus bus;
 uint16_t t1;
 int16_t t2, t3;
 bool ready;
};
int bme_raw_init(struct bme_raw *s, struct bme_bus bus);
int bme_raw_temperature(struct bme_raw *s, int32_t *centi_c);
#endif
