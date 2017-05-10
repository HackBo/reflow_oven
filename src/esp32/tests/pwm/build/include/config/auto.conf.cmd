deps_config := \
	/home/sac/esp/esp-idf/components/bt/Kconfig \
	/home/sac/esp/esp-idf/components/esp32/Kconfig \
	/home/sac/esp/esp-idf/components/ethernet/Kconfig \
	/home/sac/esp/esp-idf/components/freertos/Kconfig \
	/home/sac/esp/esp-idf/components/log/Kconfig \
	/home/sac/esp/esp-idf/components/lwip/Kconfig \
	/home/sac/esp/esp-idf/components/mbedtls/Kconfig \
	/home/sac/esp/esp-idf/components/openssl/Kconfig \
	/home/sac/esp/esp-idf/components/spi_flash/Kconfig \
	/home/sac/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/sac/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/sac/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/sac/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)


$(deps_config): ;
