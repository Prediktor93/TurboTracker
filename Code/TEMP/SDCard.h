/*
 * SDCard.h
 *
 *  Created on: 12 Aug 2018
 *      Author: angel
 */

#ifndef MAIN_PERIPHERALS_SDCARD_H_
#define MAIN_PERIPHERALS_SDCARD_H_

#include "driver/spi_master.h"
#include <stdarg.h>
#include <sys/unistd.h>
#include "driver/i2c.h"

class SDCard {
private:
	spi_device_handle_t spi;
	FILE *fp;
public:
	SDCard();
	~SDCard();

	esp_err_t Init(gpio_num_t GPIO_MISO, gpio_num_t GPIO_MOSI, gpio_num_t GPIO_SCK, gpio_num_t GPIO_CS);
	void Open(char s[30]){ fp = fopen(s, "a");}
	void WriteMock();

	void Write(const char* text, ...);

	void WriteBytes(char* data, int len);

	void flush(){fflush(fp); fsync(fileno(fp));};
};

#endif /* MAIN_PERIPHERALS_SDCARD_H_ */
