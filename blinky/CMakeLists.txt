cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(blinky)

target_sources(app PRIVATE src/main.c)
target_sources_ifdef(CONFIG_LAB_BME280 app PRIVATE bme280/bme280_raw.c bme280/bme280_app.c)

if(CONFIG_SUM_PRINT)
	target_sources(app PRIVATE sum_printk/sum_printk.c)
	target_include_directories(app PRIVATE sum_printk)
elseif(CONFIG_SUM_LOG)
	target_sources(app PRIVATE sum_log/sum_log.c)
	target_include_directories(app PRIVATE sum_log)
endif()
