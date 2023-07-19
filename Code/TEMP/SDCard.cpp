/*
 * SDCard.cpp
 *
 *  Created on: 12 Aug 2018
 *      Author: angel
 */

#include "SDCard.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include <stdio.h>
#include <string.h>
#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_err.h"
#include "esp_log.h"
#include "driver/sdmmc_host.h"
#include "driver/sdspi_host.h"
#include "sdmmc_cmd.h"
#include <stdarg.h>


SDCard::SDCard() {
	// TODO Auto-generated constructor stub

}

SDCard::~SDCard() {
	// TODO Auto-generated destructor stub

	esp_vfs_fat_sdmmc_unmount();
}


esp_err_t SDCard::Init(gpio_num_t GPIO_MISO, gpio_num_t GPIO_MOSI, gpio_num_t GPIO_SCK, gpio_num_t GPIO_CS){
	sdmmc_host_t host = SDSPI_HOST_DEFAULT();
	host.max_freq_khz = SDMMC_FREQ_HIGHSPEED;
	sdspi_slot_config_t slot_config = SDSPI_SLOT_CONFIG_DEFAULT();
	slot_config.gpio_miso = GPIO_MISO;
	slot_config.gpio_mosi = GPIO_MOSI;
	slot_config.gpio_sck  = GPIO_SCK;
	slot_config.gpio_cs   = GPIO_CS;


	esp_vfs_fat_sdmmc_mount_config_t mount_config;
	mount_config.format_if_mount_failed = false;
	mount_config.max_files = 5;
	//mount_config.allocation_unit_size= 16*1024;
	/*= {
		.format_if_mount_failed = false,
		.max_files = 5,
		.allocation_unit_size = 16 * 1024
	};*/

	sdmmc_card_t* card;
	esp_err_t ret = esp_vfs_fat_sdmmc_mount("/sdcard", &host, &slot_config, &mount_config, &card);

	if (ret != ESP_OK) {
	if (ret == ESP_FAIL) {
		ESP_LOGE("SDCARD", "FAILED MOUNTING");
		/*ESP_LOGE("SDCARD", "Failed to mount filesystem. "
			"If you want the card to be formatted, set format_if_mount_failed = true.");*/
	} else {
		ESP_LOGE("SDCARD", " FAILED INIT");
		/*ESP_LOGE("SDCARD", "Failed to initialize the card (%s). "
			"Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));*/
	}
	return ESP_FAIL;
	}

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
	va_start(args,text);
	vfprintf(fp,text,args);
	va_end(args);

	fflush(fp);
	/*fclose(fp);*/
}

void SDCard::WriteBytes(char* data, int len){
	fwrite(data, len, 1, fp);
	fflush(fp);
}

