#ifndef __IMU_H__
#define __IMU_H__

#include "esp_log.h"
#include <sys/unistd.h>
#include <math.h>
#include "i2c.h"

// Del PDF
// System constants
#define deltat 0.1f // sampling period in seconds (shown as 1 ms)
#define gyroMeasError 3.14159265358979f * (5.0f / 180.0f) // gyroscope measurement error in rad/s (shown as 5 deg/s)
#define beta sqrt(3.0f / 4.0f) * gyroMeasError // compute beta

struct imu_output{
	//RAW
	float temp;
	float gyro[3];
	float accel[3];
	float mag[3];
	//Processed
	double pitch; 			//[rad]
	double yaw; 			//[rad]
	double roll; 			//[rad]
	float speed;			//[m/s]
	uint32_t profundidad; 	//[cm]
    float totalAccel;
   
};

class Imu {
private:
	float gyro_x_offset;
	float gyro_y_offset;
	float gyro_z_offset;

	float accel_x_offset;
	float accel_y_offset;
	float accel_z_offset;
    
    float a_x, a_y, a_z; // accelerometer measurements
    float w_x, w_y, w_z; // gyroscope measurements in rad/s

    uint64_t time_since_last_speed;
    float speed;
    float speed_x;
    float speed_y;
    float speed_z;
    
	I2C_t *i2c;

    // Del PDF
    double SEq_1;
    double SEq_2;
    double SEq_3;
    double SEq_4;
    //= 1.0f, SEq_2 = 0.0f, SEq_3 = 0.0f, SEq_4 = 0.0f; // estimated orientation quaternion elements with initial conditions

public:
	Imu();
	virtual ~Imu();

    /**
     * Initialize the sensor with its configuration
    **/
    int Init(void *connection);

    /**
     * Read the data from the sensor. The output data
     * shall be stored in readdata.
     **/
    int Read(void *readdata);

    /**
     * Configure the 'cmd' with the value 'arg'
     **/
    int Config(int cmd, int arg);

    
    void filterUpdate();
    void CalibrateGyro();
    void CalibrateAccel();

    static int8_t ifaceRead(uint8_t dev_id, uint8_t reg_addr, uint8_t *read_data, uint16_t len);
    static int8_t ifaceWrite(uint8_t dev_id, uint8_t reg_addr, uint8_t *read_data, uint16_t len);
    static void ifaceWaitms(uint32_t ms){ usleep(ms*1000); };

    int Close();
};

#endif /* __IMU_H__ */
