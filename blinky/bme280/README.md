# Part 8: BME280 register temperature acquisition

The application reads registers directly through Zephyr I2C calls; it does not
use the built-in BME280 sensor driver. Part 6 addition and Part 5 button/LED
behavior remain in main.c. A separate thread samples temperature every 2 seconds.

Sequence: read chip ID (0xD0, must be 0x60), soft reset (0xE0=0xB6), wait for
NVM copying to finish (0xF3), read factory T1/T2/T3 calibration (0x88..0x8D),
skip humidity (0xF2=0), trigger temperature x1 forced measurement (0xF4=0x21),
wait for conversion, burst-read 0xFA..0xFC and apply Bosch integer compensation.
The result is printed with two decimal places in Celsius. Integer formatting
avoids requiring floating-point printf support. Errors never produce a fake
or stale temperature. Initialization is retried after communication failures.

## 8.1 Hardware

I2C1: SCL=P1.14, SDA=P1.15, address=0x77, 100 kHz. Check the module's supply
and logic voltage requirements before use; VDD on the DK is not necessarily
3.3 V. Common ground is required. A response at 0x76 is diagnosed explicitly
rather than silently changing the device tree address.

TF-M secure UART is disabled for this application because UART1 and I2C1 share
an instance. Application console output is currently on COM20 at 115200 baud on this
machine (previously COM16; USB re-enumeration changed the number). The original direct-boot Flash layout is retained.

From the repository root in an nRF Connect terminal:

```powershell
west build -d blinky/build_8_1 blinky -b nrf7002dk/nrf5340/cpuapp/ns --sysbuild
west flash -d blinky/build_8_1
```

Open Serial Monitor before reset. After about 5.5 seconds, the program prints
chip ID/calibration followed by temperature, or an explicit I2C/init error.
Capture actual temperature output for 8.1; no reference test value is a hardware
measurement. To build the app without temperature acquisition, override
`CONFIG_LAB_BME280=n` in an extra configuration file.

## 8.2 Laptop tests

The tests use mocked register reads/writes and execute the same production
initialization and compensation functions. They cover a reference conversion,
wrong chip ID, failed I2C transactions, bad calibration, stuck status flags,
and skipped temperature data. A device-tree test checks the shared sensor node,
its 0x77 address, and enabled parent bus. Its bus is a QEMU mock; this does not
verify physical wiring or prove that the Nordic GPIOs are connected correctly.
The hardware build separately resolves the actual I2C1/pinctrl configuration.

```powershell
west build -d blinky/build_8_2 blinky/tests/BME280_TEST -b qemu_cortex_m3 --no-sysbuild -- "-DQEMU=D:/Program Files/qemu/qemu-system-arm.exe"
west build -d blinky/build_8_2 -t run
```

Take a screenshot of the actual PASS summary for 8.2. Use Ctrl+A then X if QEMU
remains open after PROJECT EXECUTION SUCCESSFUL. Twister in this SDK has the
previously identified C:/E: cross-drive limitation; west build/run avoids it.

Register definitions and compensation reference:
[Bosch BME280 datasheet](https://www.bosch-sensortec.com/media/boschsensortec/downloads/datasheets/bst-bme280-ds002.pdf).

## Verified results

Hardware: chip ID 0x60, T1=28587, T2=27106, T3=50; five consecutive
readings 24.33, 24.32, 24.33, 24.33, 24.32 C.
[Captured UART output](../../img/part8/COM20-hardware.txt).

QEMU: seven tests passed, zero failed.
[Captured test output](../../img/part8/ztest-output.txt).
Screenshots for submission still need to be captured from the terminal.
