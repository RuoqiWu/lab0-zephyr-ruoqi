#include <zephyr/ztest.h>
#include <zephyr/devicetree.h>
#include <errno.h>
#include <string.h>
#include "bme280_raw.h"
static uint8_t regs[256];
static int fail_reg;
static bool busy;
static int mock_read(void *ctx, uint8_t reg, uint8_t *b, size_t len)
{
 ARG_UNUSED(ctx);
 if (reg == fail_reg) { return -EIO; }
 if (reg == 0xf3 && busy) { *b = 9; return 0; }
 memcpy(b, regs + reg, len); return 0;
}
static int mock_write(void *ctx, uint8_t reg, uint8_t value)
{
 ARG_UNUSED(ctx);
 if (reg == fail_reg) { return -EIO; }
 regs[reg] = value; return 0;
}
static void mock_sleep(unsigned int ms) { ARG_UNUSED(ms); }
static struct bme_bus bus = {.read=mock_read, .write=mock_write, .sleep_ms=mock_sleep};
static void before(void *fixture)
{
 ARG_UNUSED(fixture);
 memset(regs, 0, sizeof(regs)); fail_reg=-1; busy=false;
 regs[0xd0]=0x60;
 /* Deterministic calibration and ADC reference: 25.08 C. */
 uint16_t cal[]={27504,26435,(uint16_t)-1000};
 for(int i=0;i<3;i++){regs[0x88+i*2]=cal[i]&255;regs[0x89+i*2]=cal[i]>>8;}
 regs[0xfa]=0x7e;regs[0xfb]=0xed;regs[0xfc]=0;
}
ZTEST(bme280_registers, test_device_tree)
{
 zassert_true(DT_NODE_HAS_STATUS(DT_NODELABEL(bme280_lab), okay));
 zassert_equal(DT_REG_ADDR(DT_NODELABEL(bme280_lab)), 0x77);
 zassert_true(DT_NODE_HAS_STATUS(DT_BUS(DT_NODELABEL(bme280_lab)), okay));
}
ZTEST(bme280_registers, test_temperature)
{
 struct bme_raw s;int32_t t=0;
 zassert_ok(bme_raw_init(&s,bus));
 zassert_equal(s.t3,-1000);
 zassert_ok(bme_raw_temperature(&s,&t));
 zassert_equal(t,2508);
 zassert_equal(regs[0xf4],0x21);
 zassert_equal(regs[0xe0],0xb6);
}
ZTEST(bme280_registers, test_wrong_id)
{
 struct bme_raw s;regs[0xd0]=0x58;
 zassert_equal(bme_raw_init(&s,bus),-ENODEV);
 zassert_false(s.ready);
}
ZTEST(bme280_registers, test_bus_failure)
{
 struct bme_raw s;int32_t t=1234;
 fail_reg=0x88;zassert_equal(bme_raw_init(&s,bus),-EIO);
 fail_reg=-1;zassert_ok(bme_raw_init(&s,bus));
 fail_reg=0xfa;zassert_equal(bme_raw_temperature(&s,&t),-EIO);
 zassert_equal(t,1234);
 fail_reg=0xf4;zassert_equal(bme_raw_temperature(&s,&t),-EIO);
}
ZTEST(bme280_registers, test_invalid_calibration)
{
 struct bme_raw s;memset(regs+0x88,0,6);
 zassert_equal(bme_raw_init(&s,bus),-EINVAL);
}
ZTEST(bme280_registers, test_timeouts)
{
 struct bme_raw s;int32_t t;
 busy=true;zassert_equal(bme_raw_init(&s,bus),-ETIMEDOUT);
 busy=false;zassert_ok(bme_raw_init(&s,bus));
 busy=true;zassert_equal(bme_raw_temperature(&s,&t),-ETIMEDOUT);
}
ZTEST(bme280_registers, test_skipped_reading)
{
 struct bme_raw s;int32_t t=1234;
 zassert_ok(bme_raw_init(&s,bus));regs[0xfa]=0x80;regs[0xfb]=0;
 zassert_equal(bme_raw_temperature(&s,&t),-ENODATA);zassert_equal(t,1234);
}
ZTEST_SUITE(bme280_registers,NULL,NULL,before,NULL,NULL);
