#ifndef __I2C_CONN_H__
#define __I2C_CONN_H__

#include "driver/i2c.h"

class I2C_Conn{
public:
    I2C_Conn(gpio_num_t SDA, gpio_num_t SCL);

    void Init();
    void Scan();

private:

    gpio_num_t pinSDA, pinSCL;
    i2c_config_t i2c_config;
};


#endif //__I2C_CONN_H__