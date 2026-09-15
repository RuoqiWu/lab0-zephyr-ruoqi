"""Build, flash, or run one Lab 0 question without ambiguous VS Code contexts."""
import argparse
import os
from pathlib import Path
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]
TARGETS = {
    "2": ("blinky/lab_stages", None, False),
    "2_original": ("blinky/lab_stages", "original.conf", False),
    "3": ("blinky/lab_stages", None, False),
    "4": ("blinky/lab_stages", None, False),
    "5": ("blinky/lab_stages", "part5.conf", False),
    "6_printk": ("blinky", "configs/part6_printk.conf", False),
    "6_log": ("blinky", "configs/part6_log.conf", False),
    "7": ("blinky/tests/SUM_UNIT_TEST", None, True),
    "8_1": ("blinky", None, False),
    "8_2": ("blinky/tests/BME280_TEST", None, True),
}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("question", choices=TARGETS)
    parser.add_argument("action", choices=["build", "flash", "run", "menuconfig"],
                        nargs="?", default="build")
    parser.add_argument("--pristine", action="store_true")
    args = parser.parse_args()
    source, extra, simulated = TARGETS[args.question]
    if args.action == "flash" and simulated:
        parser.error("QEMU tests use 'run', not 'flash'.")
    if args.action == "run" and not simulated:
        parser.error("Hardware applications use 'flash', then the serial monitor.")

    env = os.environ.copy()
    toolchain = Path(env.get("LAB_TOOLCHAIN", "C:/ncs/toolchains/dcbdc366a1"))
    sdk = Path(env.get("ZEPHYR_BASE", "C:/ncs/v3.4.0/zephyr"))
    env["ZEPHYR_BASE"] = str(sdk)
    env.setdefault("ZEPHYR_SDK_INSTALL_DIR", str(toolchain / "opt/zephyr-sdk"))
    env.setdefault("ZEPHYR_TOOLCHAIN_VARIANT", "zephyr/gnu")
    env["PATH"] = os.pathsep.join(str(toolchain / p) for p in
                                    ["opt/bin", "opt/bin/Scripts", "mingw64/bin"]) + os.pathsep + env.get("PATH", "")
    env["CCACHE_DISABLE"] = "1"
    # Nested sysbuild projects discover their own cache directory on Windows.
    # Keep that cache writable and local without changing the user's environment.
    local_cache = ROOT / "blinky/.cache/lab-tools"
    (local_cache / ".cache").mkdir(parents=True, exist_ok=True)
    env["LOCALAPPDATA"] = str(local_cache)
    # SDK ownership can differ from the user running the toolchain.
    count = int(env.get("GIT_CONFIG_COUNT", "0"))
    env["GIT_CONFIG_COUNT"] = str(count + 1)
    env[f"GIT_CONFIG_KEY_{count}"] = "safe.directory"
    env[f"GIT_CONFIG_VALUE_{count}"] = sdk.parent.as_posix() + "/*"
    env.setdefault("NRFUTIL_HOME", str(toolchain / "nrfutil/home"))
    env["PATH"] = str(toolchain / "nrfutil/bin") + os.pathsep + env["PATH"]
    python = toolchain / "opt/bin/python.exe"
    if not python.is_file():
        parser.error("Set LAB_TOOLCHAIN to the installed nRF toolchain directory.")
    build = ROOT / "blinky" / ("build_" + args.question)
    command = [str(python), "-m", "west"]
    if args.action == "build":
        command += ["build", "-d", str(build), str(ROOT / source),
                    "-b", "qemu_cortex_m3" if simulated else "nrf7002dk/nrf5340/cpuapp/ns",
                    "-p", "always" if args.pristine else "auto",
                    "--no-sysbuild" if simulated else "--sysbuild", "--",
                    "-DCMAKE_MAKE_PROGRAM=" + (toolchain / "opt/bin/ninja.exe").as_posix(),
                    "-DUSER_CACHE_DIR=" + (build / ".cache").as_posix()]
        if not simulated:
            domain = "lab_stages" if source.endswith("lab_stages") else "blinky"
            command += [f"-D{domain}_USER_CACHE_DIR=" + (build / ".cache").as_posix()]
        if extra:
            command += ["-DEXTRA_CONF_FILE=" + (ROOT / source / extra).as_posix()]
        if simulated:
            qemu = shutil.which("qemu-system-arm", path=env["PATH"])
            if not qemu:
                qemu = str(Path(env.get("QEMU_BIN_PATH", "D:/Program Files/qemu")) / "qemu-system-arm.exe")
            if not Path(qemu).is_file():
                parser.error("Set QEMU_BIN_PATH to the folder containing qemu-system-arm.exe.")
            command += ["-DQEMU=" + Path(qemu).as_posix()]
    else:
        if not (build / "CMakeCache.txt").is_file():
            parser.error("Build this question first.")
        if args.action == "flash":
            command += ["flash", "-d", str(build)]
        else:
            target_build = build
            if args.action == "menuconfig" and not simulated:
                domain = "lab_stages" if source.endswith("lab_stages") else "blinky"
                target_build = build / domain
            command += ["build", "-d", str(target_build), "-t", args.action]
    print(f"Part {args.question}: {args.action} -> {build}", flush=True)
    print(subprocess.list2cmdline(command), flush=True)
    return subprocess.call(command, cwd=ROOT, env=env)


if __name__ == "__main__":
    sys.exit(main())
