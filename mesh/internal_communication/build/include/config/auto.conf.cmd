deps_config := \
	/home/syxd/esp/esp-idf/components/app_trace/Kconfig \
	/home/syxd/esp/esp-idf/components/aws_iot/Kconfig \
	/home/syxd/esp/esp-idf/components/bt/Kconfig \
	/home/syxd/esp/esp-idf/components/driver/Kconfig \
	/home/syxd/esp/esp-idf/components/esp32/Kconfig \
	/home/syxd/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/home/syxd/esp/esp-idf/components/esp_http_client/Kconfig \
	/home/syxd/esp/esp-idf/components/ethernet/Kconfig \
	/home/syxd/esp/esp-idf/components/fatfs/Kconfig \
	/home/syxd/esp/esp-idf/components/freertos/Kconfig \
	/home/syxd/esp/esp-idf/components/heap/Kconfig \
	/home/syxd/esp/esp-idf/components/libsodium/Kconfig \
	/home/syxd/esp/esp-idf/components/log/Kconfig \
	/home/syxd/esp/esp-idf/components/lwip/Kconfig \
	/home/syxd/esp/esp-idf/components/mbedtls/Kconfig \
	/home/syxd/esp/esp-idf/components/nvs_flash/Kconfig \
	/home/syxd/esp/esp-idf/components/openssl/Kconfig \
	/home/syxd/esp/esp-idf/components/pthread/Kconfig \
	/home/syxd/esp/esp-idf/components/spi_flash/Kconfig \
	/home/syxd/esp/esp-idf/components/spiffs/Kconfig \
	/home/syxd/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/home/syxd/esp/esp-idf/components/vfs/Kconfig \
	/home/syxd/esp/esp-idf/components/wear_levelling/Kconfig \
	/home/syxd/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/home/syxd/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/home/syxd/esp32_test/mesh/internal_communication/main/Kconfig.projbuild \
	/home/syxd/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/home/syxd/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
