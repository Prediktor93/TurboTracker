
#include "SDCard.h"
/*#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"
#include <stdarg.h>*/


/*#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_types.h"*/

#include "sdmmc_cmd.h"
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
//#include "SD/sdspi_host.h"
#include "esp_log.h"
#include "esp_vfs.h"
#include "config.h"

static const char *TAG = "SDCard";

#define MOUNT_POINT "/sdcard"
#define LOG_FILENAME "log2.txt"


SDCard::SDCard() {
}

SDCard::~SDCard() {
	esp_vfs_fat_sdmmc_unmount();
}


esp_err_t SDCard::Init(){

    // esp_err_t ret;

    // esp_vfs_fat_sdmmc_mount_config_t mount_config = {
    //     .format_if_mount_failed = false,
    //     .max_files = 5,
    //     .allocation_unit_size = 16 * 1024
    // };

    // sdmmc_card_t *card;
    // const char mount_point[] = MOUNT_POINT;
    // sdmmc_host_t host = SDMMC_HOST_DEFAULT();
    // host.max_freq_khz = SDMMC_FREQ_PROBING;
    // host.flags = SDMMC_HOST_FLAG_1BIT;
    // //host.slot = VSPI_HOST;
    // sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();
    // slot_config.width = 1;
    // slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;
    // //slot_config.gpio_cd = GPIO_NUM_33;


    // //gpio_pulldown_dis(GPIO_NUM_2);
    // //gpio_pullup_en(GPIO_NUM_2);

    // //esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    // ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

    // if (ret != ESP_OK) {
    //     if (ret == ESP_FAIL) {
    //         ESP_LOGE(TAG, "Failed to mount filesystem. "
    //                  "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
    //     } else {
    //         ESP_LOGE(TAG, "Failed to initialize the card (%s). "
    //                  "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
    //     }
    //     return ret;
    // }
    // ESP_LOGI(TAG, "Filesystem mounted");

    // // Card has been initialized, print its properties
    // sdmmc_card_print_info(stdout, card);

    // fp = fopen(MOUNT_POINT "/" LOG_FILENAME, "w");
    // this->Write("Probando...\n");




    /*gpio_pullup_en(GPIO_NUM_23);
    gpio_pullup_en(GPIO_NUM_19);
    gpio_pullup_en(GPIO_NUM_18);
    gpio_pullup_en(GPIO_NUM_5);*/

    //gpio_set_direction(GPIO_NUM_5, GPIO_MODE_INPUT);
    //gpio_pullup_en(GPIO_NUM_5);

    esp_err_t ret;

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();
    host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
    host.slot = VSPI_HOST;
    host.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

	esp_vfs_fat_sdmmc_mount_config_t mount_config;
	mount_config.format_if_mount_failed = false;
	mount_config.max_files = 5;
    mount_config.allocation_unit_size = 16 * 1024;


    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = CS_PIN;
    slot_config.host_id = static_cast<spi_host_device_t>(host.slot);


    spi_bus_config_t bus_cfg = {
        .mosi_io_num = MOSI_PIN,
        .miso_io_num = MISO_PIN,
        .sclk_io_num = SCK_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 4000,
    };

    ret = spi_bus_initialize(static_cast<spi_host_device_t>(host.slot), &bus_cfg, SDSPI_DEFAULT_DMA);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize bus.");
        return ret;
    }



    ESP_LOGI(TAG, "Mounting filesystem");
    sdmmc_card_t *card;
    const char mount_point[] = "/sdcard";

    ret = esp_vfs_fat_sdspi_mount(mount_point, &host, &slot_config, &mount_config, &card);

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the CONFIG_EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        return ret;
    }
    ESP_LOGI(TAG, "Filesystem mounted");








/*sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
	sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
	slot_config.gpio_miso = GPIO_NUM_19;
	slot_config.gpio_mosi = GPIO_NUM_23;
	slot_config.gpio_sck  = GPIO_NUM_18;
	slot_config.gpio_cs   = GPIO_NUM_5;


	esp_vfs_fat_sdmmc_mount_config_t mount_config;
	mount_config.format_if_mount_failed = false;
	mount_config.max_files = 5;
	//mount_config.allocation_unit_size= 16*1024;
	// = {
	// 	.format_if_mount_failed = false,
	// 	.max_files = 5,
	// 	.allocation_unit_size = 16 * 1024
	// };

	sdmmc_card_t* card;
	esp_err_t ret = esp_vfs_fat_sdspi_mount("/sdcard", &host, &slot_config, &mount_config, &card);

	if (ret != ESP_OK) {
	if (ret == ESP_FAIL) {
		//ESP_LOGE("SDCARD", "FAILED MOUNTING");
		ESP_LOGE("SDCARD", "Failed to mount filesystem. "
			"If you want the card to be formatted, set format_if_mount_failed = true.");
	} else {
		//ESP_LOGE("SDCARD", " FAILED INIT");
		ESP_LOGE("SDCARD", "Failed to initialize the card (%s). "
			"Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
	}
	return ESP_FAIL;
	}*/


	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);
	return ESP_OK;
}

/*
esp_err_t SDCard::WriteBytes(uint8_t *buf, uint32_t size){
	return OK;
}*/

void SDCard::WriteMock(){
	FILE *fp = fopen("/sdcard/test.txt", "a");
	fprintf(fp, "Hola Mundo!");
	fflush(fp);
	fclose(fp);
	fp = fopen("/sdcard/test.txt", "r");
	if (fp == NULL) {
		ESP_LOGE("SDREAD", "Failed to open file for reading");
		return;
	}
	char line[64];
	fgets(line, sizeof(line), fp);
	fclose(fp);

	char* pos = strchr(line, '\n');
	if (pos) {
		*pos = '\0';
	}
	ESP_LOGI("SDREAD", "Read from file: '%s'", line);

	esp_vfs_fat_sdmmc_unmount();

}

void SDCard::Write(const char* text, ...){

	va_list args;
    if (fp == NULL) {
        return;
    }
	va_start(args,text);
	vfprintf(fp,text,args);
	va_end(args);

	fflush(fp);
    fsync(fileno(fp));
	/*fclose(fp);*/
}

void SDCard::WriteBytes(char* data, int len){
	fwrite(data, len, 1, fp);
	fflush(fp);
}

