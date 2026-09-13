# ESE5180: Lab 0 Zephyr

| Team Member Name | Email Address                |
| ---------------- | ---------------------------- |
| Ruoqi Wu         | neorqi@engineering.upenn.edu |

**GitHub Repository URL:** https://github.com/RuoqiWu/lab0-zephyr-ruoqi.git

## 1. Sample Header

## 2. Sample Second Header


## 4. Kconfig


1. **What are the levels of log statements?**

   Zephyr provides four main logging levels: Error (`LOG_ERR`), Warning (`LOG_WRN`), Information (`LOG_INF`), and Debug (`LOG_DBG`). The corresponding numeric levels are 1, 2, 3, and 4. Level 0 disables logging.

2. **What is the difference between `prj.conf` and `menuconfig`?**

   `prj.conf` is a persistent, text-based file that defines the application's desired Kconfig settings. `menuconfig` is an interactive interface for viewing and changing Kconfig options and their dependencies. Menuconfig changes normally affect the generated build configuration and should be copied into `prj.conf` if they need to remain after a pristine build.

3. **How do you check that the symbols in `prj.conf` are set after building? Why?**

   After building, the symbols can be checked in the generated `build/blinky/zephyr/.config` file. This file contains the final resolved configuration after application, board, dependency, and default settings have been combined. Checking it confirms that the requested options were enabled without being changed or disabled by dependencies.