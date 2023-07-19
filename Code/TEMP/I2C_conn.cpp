#include "I2C_conn.h"
#include "esp_log.h"

#define I2C_NUM I2C_NUM_0

static const char *TAG = "I2C";

I2C_Conn::I2C_Conn(gpio_num_t SDA, gpio_num_t SCL):
    pinSDA(SDA), pinSCL(SCL)
{                       
}

void I2C_Conn::Init()
{

	i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = 21,
		.scl_io_num = 22,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE
	};

	i2c_config.master.clk_speed = I2C_MASTER_FREQ_HZ;

	i2c_param_config(I2C_NUM, &i2c_config);
	i2c_driver_install(I2C_NUM, I2C_MODE_MASTER, 0, 0, 0);

}

void I2C_Conn::Scan() {

	int nDevices = 0;
	printf(LOG_COLOR_W "\n>> I2C scanning ..." LOG_RESET_COLOR "\n");
	for(unsigned int i = 0; i < 127; i++){
		i2c_cmd_handle_t cmd = i2c_cmd_link_create();
		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, ( i << 1 ) | I2C_MASTER_WRITE, true);
		i2c_master_stop(cmd);
		esp_err_t err = i2c_master_cmd_begin(0, cmd, 10 / portTICK_PERIOD_MS);
		i2c_cmd_link_delete(cmd);
		if(err == 0){
			printf(LOG_COLOR_W "- Device found at address 0x%X%s", i, LOG_RESET_COLOR "\n");
			nDevices ++;
		}
	}

	if(nDevices)
		printf(LOG_COLOR_W "- Scan end, found %d devices %s", nDevices, LOG_RESET_COLOR "\n");
	else
		printf(LOG_COLOR_E "- No I2C devices found!" LOG_RESET_COLOR "\n");
}

esp_err_t I2C_Conn::writeByte(uint8_t devAddr, uint8_t regAddr, uint8_t data, int32_t timeout) {
    return writeBytes(devAddr, regAddr, 1, &data, timeout);
}

esp_err_t I2C_Conn::writeBytes(uint8_t devAddr, uint8_t regAddr, size_t length, const uint8_t *data, int32_t timeout) {
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (devAddr << 1) | I2C_MASTER_WRITE, I2C_MASTER_ACK_EN);
    i2c_master_write_byte(cmd, regAddr, I2C_MASTER_ACK_EN);
    i2c_master_write(cmd, (uint8_t*) data, length, I2C_MASTER_ACK_EN);
    i2c_master_stop(cmd);
    esp_err_t err = i2c_master_cmd_begin(port, cmd, (timeout < 0 ? ticksToWait : pdMS_TO_TICKS(timeout)));
    i2c_cmd_link_delete(cmd);
#if defined CONFIG_I2CBUS_LOG_READWRITES
    if (!err) {
        char str[length*5+1];
        for (size_t i = 0; i < length; i++)
            sprintf(str+i*5, "0x%s%X ", (data[i] < 0x10 ? "0" : ""), data[i]);
        I2CBUS_LOG_RW("[port:%d, slave:0x%X] Write %d bytes to register 0x%X, data: %s",
            port, devAddr, length, regAddr, str);
    }
#endif
#if defined CONFIG_I2CBUS_LOG_ERRORS
#if defined CONFIG_I2CBUS_LOG_READWRITES
    else {
#else
    if (err) {
#endif
        I2CBUS_LOGE("[port:%d, slave:0x%X] Failed to write %d bytes to__ register 0x%X, error: 0x%X",
            port, devAddr, length, regAddr, err);
    }
#endif
    return err;
}