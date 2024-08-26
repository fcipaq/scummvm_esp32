#include "osystem.h"

//#include "esp_err.h"
#include "esp_log.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include "esp_heap_caps.h"

#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define SD_BASE_PATH "/sd"

#if 0
#define SD_PIN_NUM_MISO  GPIO_NUM_42
#define SD_PIN_NUM_MOSI  GPIO_NUM_2
#define SD_PIN_NUM_CLK   GPIO_NUM_1
#define SD_PIN_NUM_CS    GPIO_NUM_41
#endif

#if 1
#define SD_PIN_NUM_MISO  GPIO_NUM_42  // grün
#define SD_PIN_NUM_MOSI  GPIO_NUM_2  // weiß
#define SD_PIN_NUM_CLK   GPIO_NUM_1  // lila
#define SD_PIN_NUM_CS    GPIO_NUM_NC  // schwarz
#endif

namespace _Esp32 {

static bool _isSDcardmounted = false;

sdmmc_card_t *_card;

bool OSystem_Esp32::initSDcard()
{

    bool ret = false;

    if (_isSDcardmounted)
    {
        printf("sdcard_open: alread mounted.\n");
    }
    else
    {
	    // Options for mounting the filesystem.
	    // If format_if_mount_failed is set to true, SD card will be partitioned and
	    // formatted in case when mounting fails.
	    esp_vfs_fat_sdmmc_mount_config_t mount_config = {
		.format_if_mount_failed = false,
		.max_files = 15,
		.allocation_unit_size = 16 * 1024
	    };

	    // Use settings defined above to initialize SD card and mount FAT filesystem.
	    // Note: esp_vfs_fat_sdmmc/sdspi_mount is all-in-one convenience functions.
	    // Please check its source code and implement error recovery when developing
	    // production applications.

	    // By default, SD card frequency is initialized to SDMMC_FREQ_DEFAULT (20MHz)
	    // For setting a specific frequency, use host.max_freq_khz (range 400kHz - 40MHz for SDMMC)
	    // Example: for fixed frequency of 10MHz, use host.max_freq_khz = 10000;
	    sdmmc_host_t host = SDMMC_HOST_DEFAULT();
	    host.max_freq_khz = 5000;

	    // This initializes the slot without card detect (CD) and write protect (WP) signals.
	    // Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	    sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

	    // Set bus width to use:
	    slot_config.width = 1;

	    // On chips where the GPIOs used for SD card can be configured, set them in
	    // the slot_config structure:
	    slot_config.clk = SD_PIN_NUM_CLK;
	    slot_config.cmd = SD_PIN_NUM_MOSI;
	    slot_config.d0 = SD_PIN_NUM_MISO;

	    // Enable internal pullups on enabled pins. The internal pullups
	    // are insufficient however, please make sure 10k external pullups are
	    // connected on the bus. This is for debug / example purpose only.
	    slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

	    if (esp_vfs_fat_sdmmc_mount(SD_BASE_PATH, &host, &slot_config, &mount_config, &_card) == ESP_OK) {
	      ret = true;
              _isSDcardmounted = true;
	    }
  }

  return ret;
}


void OSystem_Esp32::destroySDcard()
{
    if (!_isSDcardmounted)
    {
        printf("sdcard_close: no sd card currently mounted.\n");
    }
    else
    {
        esp_err_t ret = esp_vfs_fat_sdcard_unmount(SD_BASE_PATH, _card);

        if (ret != ESP_OK)
        {
            printf("sdcard_close: esp_vfs_fat_sdmmc_unmount failed (%d)\n", ret);
            assert(0);
    	}

        _isSDcardmounted = false;
    }
}

} // namespace ESP32
