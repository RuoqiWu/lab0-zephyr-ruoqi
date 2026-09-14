# Part 7: sum_log Ztest

This independent application compiles the existing `../../sum_log/sum_log.c`.
It does not compile or modify the LED/button application's `main.c`.
Three test cases check nine sums: positive inputs, negative/mixed-sign inputs,
and zero/cancellation. Assertions check the returned value, not printed text.

## Run on the laptop

Run from the repository root in an nRF Connect terminal:

```powershell
west build -d blinky/tests/SUM_UNIT_TEST/build blinky/tests/SUM_UNIT_TEST -b qemu_cortex_m3 --no-sysbuild
west build -d blinky/tests/SUM_UNIT_TEST/build -t run
```

The first command builds; the second launches QEMU and executes the tests.

On this development machine QEMU is installed at `D:\Program Files\qemu`.
To configure this location explicitly (including when the cache previously
contained `QEMU-NOTFOUND`), run:

```powershell
$env:QEMU_BIN_PATH = 'D:\Program Files\qemu'
west build -d blinky/tests/SUM_UNIT_TEST/build --cmake -- "-DQEMU=D:/Program Files/qemu/qemu-system-arm.exe"
west build -d blinky/tests/SUM_UNIT_TEST/build -t run
```
If QEMU is installed under `C:\Program Files\qemu`, set its path before building:

```powershell
$env:QEMU_BIN_PATH = 'C:\Program Files\qemu'
Test-Path "$env:QEMU_BIN_PATH\qemu-system-arm.exe"
```

Use the actual installation path. Setting this variable does not install QEMU.
Alternatively, use Twister to discover, build, run, and report the tests:

On Windows, this SDK's Twister computes a relative path between the test source
and Zephyr. With this repository on E: and the SDK on C:, discovery raises
`ValueError: path is on mount 'E:', start on mount 'C:'`. Use the `west build`
and `-t run` commands above for this layout; `--no-detailed-test-id` does not
bypass the failing path calculation. The Twister command below requires the
test source and SDK to be on the same drive with this SDK version.

```powershell
west twister -T blinky/tests/SUM_UNIT_TEST -p qemu_cortex_m3 --inline-logs -v --outdir blinky/tests/SUM_UNIT_TEST/twister-out
```

For 7.2, capture actual output showing the three test cases passing and the final
`PROJECT EXECUTION SUCCESSFUL` result (or Twister's passed summary). A successful
build alone does not mean the tests passed. The intentional example error/warning
logs come from Part 6; assertions and the runner summary determine test failures.

## Optional hardware build

```powershell
west build -d blinky/tests/SUM_UNIT_TEST/build-board blinky/tests/SUM_UNIT_TEST -b nrf7002dk/nrf5340/cpuapp/ns --sysbuild
```

The board target uses slashes in this SDK. The test overlay reuses the application's
direct TF-M boot layout. This command only builds; flashing tests would replace
the running Part 6 firmware, and is not needed for laptop testing.

## Think questions

| Option | Purpose |
| --- | --- |
| `CONFIG_ZTEST=y` | Enables Ztest and its test runner. |
| `CONFIG_ZTEST_ASSERT_VERBOSE=2` | Enables verbose assertion reporting, including successful assertions. |
| `CONFIG_LOG=y` | Enables logging used by the original `sum_log()` implementation. |
| `CONFIG_LOG_MODE_IMMEDIATE=y` | Emits test logs synchronously before the simulator exits. |

**Where is main()?** Zephyr's Ztest framework provides `main()` and invokes its
default `test_main()` to run registered tests. `ZTEST_SUITE` registers the suite,
and `ZTEST` registers each test case, so our test source needs no custom `main()`.

| Command | Role | Preferred scenario |
| --- | --- | --- |
| `west build` | Compiles one application for one board; does not discover tests from `testcase.yaml`. | Developing one test application and fixing compiler errors. |
| `west build -t run` | Runs the configured simulator for that build. | Quickly executing a single test binary locally. |
| `west twister` | Reads test YAML, selects scenarios/platforms, builds, runs supported targets, and generates reports. | Multiple suites/platforms, regression testing, and CI. |

References: [Ztest](https://docs.zephyrproject.org/latest/develop/test/ztest.html),
[Twister](https://docs.zephyrproject.org/latest/develop/twister/index.html),
[west build/run](https://docs.zephyrproject.org/latest/develop/west/build-flash-debug.html).
