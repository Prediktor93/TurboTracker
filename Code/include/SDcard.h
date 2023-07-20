
#ifndef _SDCARD_H_
#define _SDCARD_H_

#include "driver/spi_master.h"
#include <stdarg.h>
#include <sys/unistd.h>

class SDCard {
private:
	spi_device_handle_t spi;
	FILE *fp;
public:
	SDCard();
	~SDCard();

	esp_err_t Init();
	void Open(char s[30]){ fp = fopen(s, "a");}
	void WriteMock();

	void Write(const char* text, ...);

	void WriteBytes(char* data, int len);

	void flush(){fflush(fp); fsync(fileno(fp));};
};

#endif /* _SDCARD_H_ */
