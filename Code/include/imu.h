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
	double pitch; 			//[rad] [-1.5,1.5] 1.55 vertical arriba, -1.55 vertical abajo
	double yaw; 			//[rad] [-3,3]     3 y -3 mirando atras, izda +, dcha -
	double roll; 			//[rad] [-3,3]     3 y -3 boca abajo   , izda +, dcha
	float speed;			//[m/s]
	uint32_t profundidad; 	//[cm]
    float totalAccel;
   
};

class Imu {
private:

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


    void filterUpdate();
    void CalibrateAccel();

};

#endif /* __IMU_H__ */
