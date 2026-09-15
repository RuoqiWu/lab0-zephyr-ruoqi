# Lab 0: builds by question

All named build directories are under `blinky/`. Source files are tracked;
generated build directories are ignored by Git. Rebuild them rather than renaming
or committing CMake caches. Existing source code and older build directories are
retained.

| Question | Build directory | Behavior / source |
| --- | --- | --- |
| 2 original rate | `build_2_original` | LED1 toggles every 1000 ms; `lab_stages` |
| 2 | `build_2` | LED1 toggles every 2000 ms; `lab_stages` |
| 3 | `build_3` | Same Blinky program, for West command demonstration |
| 4 | `build_4` | Same Blinky program, for Kconfig inspection |
| 5 | `build_5` | Poll Button1 and toggle LED2 using custom aliases |
| 6 printk | `build_6_printk` | Existing main + printk sum; BME280 thread disabled |
| 6 Logger | `build_6_log` | Existing main + Logger sum; BME280 thread disabled |
| 7 | `build_7` | `tests/SUM_UNIT_TEST`, QEMU, 3 tests |
| 8.1 | `build_8_1` | Existing full hardware application with BME280 |
| 8.2 | `build_8_2` | `tests/BME280_TEST`, QEMU, 7 tests |

Part 1 needs three separate board targets and is not replaced by these nRF-only
builds. Part 2.1 asks for a commit; the two-second demonstration is Part 2.2.
The 2000 ms delay is between toggles (a full ON/OFF cycle is 4000 ms).

## VS Code

Use **Terminal > Run Task** and choose `Lab: Build question`,
`Lab: Flash hardware question`, or `Lab: Run QEMU question`, then select the
question. These tasks provide both the correct source directory and build
directory; they do not depend on the nRF extension's currently selected context.
For Part 4 choose `Lab: Part 4 menuconfig` after building Part 4.
The workspace task file is already installed locally. Since this repository
ignores `.vscode/`, a tracked copy is provided in `scripts/vscode-tasks.json`;
on a fresh checkout, copy it to `.vscode/tasks.json` to restore the same menus.

Build tasks only compile. Flash tasks explicitly replace the firmware on the
connected board. To restore the final sensor demonstration, flash Part `8_1`.
QEMU tasks print in Terminal; hardware output uses the application serial port
(currently COM20, 115200 baud). After opening the port, reset the board.

## Terminal

From the repository root in PowerShell (each command is a separate operation):

```powershell
& 'C:/ncs/toolchains/dcbdc366a1/opt/bin/python.exe' scripts/lab.py 2 build
& 'C:/ncs/toolchains/dcbdc366a1/opt/bin/python.exe' scripts/lab.py 2 flash
& 'C:/ncs/toolchains/dcbdc366a1/opt/bin/python.exe' scripts/lab.py 7 run
& 'C:/ncs/toolchains/dcbdc366a1/opt/bin/python.exe' scripts/lab.py 8_2 run
```

Change the question argument to any entry in the table. Use `build --pristine`
only when a clean rebuild is needed. The helper sets up the installed toolchain
for its subprocess; overrides are `LAB_TOOLCHAIN`, `ZEPHYR_BASE`, and
`QEMU_BIN_PATH`. It does not change persistent environment settings.

For Part 3, use West directly in an nRF terminal to demonstrate the requirement:

```powershell
west build -d blinky/build_3 blinky/lab_stages -b nrf7002dk/nrf5340/cpuapp/ns --sysbuild
west flash -d blinky/build_3
```

For the two tests after building:

```powershell
west build -d blinky/build_7 -t run
west build -d blinky/build_8_2 -t run
```

Wait for `PROJECT EXECUTION SUCCESSFUL` before capturing the test output.
If QEMU remains open after completion, press Ctrl+A, release, then X to exit.

## Part 2 code

`blinky/lab_stages/src/main.c` uses the original `led0` alias for Parts 2–4,
configures it as an output, toggles it, and sleeps for
`CONFIG_LAB_BLINK_INTERVAL_MS` (2000 ms). For Part 5 the same source selects
`led5180` and `button5180` and polls the button instead. Earlier Parts 6–8 source
files remain unchanged.
