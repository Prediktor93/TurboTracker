
#include "imu.h"
#include <unistd.h>
#include <math.h>
#include <esp_timer.h>
#include <inttypes.h>
#include <string.h>

#include "freertos/task.h"

#define    ACCEL_ADDRESS              0x18
#define    GYRO_ADDRESS         	  0x68
#define    MAG_ADDRESS                0x10

void ToEulerAngle(double qw, double qx, double qy, double qz, double& roll, double& pitch, double& yaw){

	//ESP_LOGI("[DEBUG EULER]", "Qs: %.2f %.2f %.2f %.2f\n", qw, qx, qy, qz);

    // roll (x-axis rotation)
    double sinr = +2.0 * (qw * qx + qy * qz);
    double cosr = +1.0 - 2.0 * (qx * qx + qy * qy);
    roll = atan2(sinr, cosr);

    // pitch (y-axis rotation)
    double sinp = +2.0 * (qw * qy - qz * qx);
    if (fabs(sinp) >= 1)
        pitch = copysign(M_PI / 2, sinp); // use 90 degrees if out of range
    else
        pitch = asin(sinp);

    // yaw (z-axis rotation)
    double siny = +2.0 * (qw * qz + qx * qy);
    double cosy = +1.0 - 2.0 * (qy * qy + qz * qz);
    yaw = atan2(siny, cosy);
}

/*
 * USES I2C
 */
Imu::Imu() {
    
    SEq_1 = 1.0f;
    SEq_2 = 0.0f;
    SEq_3 = 0.0f;
    SEq_4 = 0.0f;
    
    time_since_last_speed = 0;
    speed = 0.0f;
    speed_x = 0.0f;
    speed_y = 0.0f;
    speed_z = 0.0f;

}

Imu::~Imu() {}


int Imu::Init(void *connection){
	i2c = (I2C_t*)connection;
	
	/*
	 * Configurar acelerometro TODO
	 */
	//Data_high_bandwith
	//	- 0 bandwidth sin filtro
	//	- 1 bandwidth con filtro
	//Para establecer bandwidth en modo con filtro, setear reg 0x10 (pag 27)
	i2c->writeByte(ACCEL_ADDRESS, 0x10, 0b01011); // Filtro a 62.5 Hz
	i2c->writeByte(ACCEL_ADDRESS, 0x13, 0x0);     // Filtered data

	//Acceleration measurement address (pag 28)
	//	- +-2g, 	0.98mg/LSB 0x3
	//	- +-4g, 	1.95mg/LSB 0x5
	i2c->writeByte(ACCEL_ADDRESS, 0x0F, 0x05);    // +-4g

	CalibrateAccel();

	/*
	 * Configurar Gyro
	 */
	i2c->writeByte(GYRO_ADDRESS, 0x10, 0b0100); //200Hz

	// Configurar magnetometro TODO
	//i2c0.writeByte(MAG_ADDRESS, 0x37, 0x02);
	//i2c0.writeByte(MAG_ADDRESS, 0x0A, 0x01);

	return ESP_OK;
}


int Imu::Read(void *readdata)
{

	imu_output *output_data;
	output_data = (imu_output*) readdata;

	// Temperatura
	/*uint8_t temperature = 0;
	i2c->readByte(ACCEL_ADDRESS, 0x08, &temperature);
	int8_t temp_ = (int8_t)temperature;
	float temp = 23 + (temp_*0.5);*/

	//ESP_LOGI("[Imu::Read]","Temperatura: %.1fºC", temp);

	/*
	 * Giroscopio
	 * Angular rate in (-2000 to 2000º/s)
	 */

	int16_t gyro[3];
	uint8_t gyro_p[6];
	i2c->readBytes(GYRO_ADDRESS, 0x02, 6, gyro_p);

	gyro[0] = gyro_p[1] << 8 | gyro_p[0];
	gyro[1] = gyro_p[3] << 8 | gyro_p[2];
	gyro[2] = gyro_p[5] << 8 | gyro_p[4];


	float gyro_x = (gyro[0] * 2000.0) / 32767.0 ; // - gyro_x_offset
	float gyro_y = (gyro[1] * 2000.0) / 32767.0 ; // - gyro_y_offset
	float gyro_z = (gyro[2] * 2000.0) / 32767.0 ; // - gyro_z_offset
    
    w_x = gyro_x * 3.14159265358979323846 / 180.0;
    w_y = gyro_y * 3.14159265358979323846 / 180.0;
    w_z = gyro_z * 3.14159265358979323846 / 180.0;

	output_data->gyro[0] = gyro_x;
	output_data->gyro[1] = gyro_y;
	output_data->gyro[2] = gyro_z;
	//ESP_LOGI("[Imu::Read_gyro]","X: %.2f Y: %.2f Z: %.2f \n", w_x, w_y, w_z);

	/*
	 * Acelerometro
	 * Valores en mg¿?
	 */

	uint16_t acc[3];
	uint8_t acc_p[6];
	memset(acc_p, 0, 6);
	i2c->readBytes(ACCEL_ADDRESS, 0x2, 6, acc_p);

	acc[0] = (acc_p[1] << 4) | (acc_p[0] >> 4);
	acc[1] = (acc_p[3] << 4) | (acc_p[2] >> 4);
	acc[2] = (acc_p[5] << 4) | (acc_p[4] >> 4);

	if (acc[0] > 2047) {
		acc[0] -= 4096;
	}

	if (acc[1] > 2047) {
		acc[1] -= 4096;
	}

	if (acc[2] > 2047) {
		acc[2] -= 4096;
	}

	a_x = (int16_t)acc[0] * 1.95 / 100.0 + accel_x_offset; 
	a_y = (int16_t)acc[1] * 1.95 / 100.0 + accel_y_offset;
	a_z = (int16_t)acc[2] * 1.95 / 100.0 + accel_z_offset; 

	output_data->accel[0] = a_x;
	output_data->accel[1] = a_y;
	output_data->accel[2] = a_z;

	output_data->totalAccel = sqrt( a_x*a_x + a_y*a_y + a_z*a_z);

	/* Speed */
	/*uint64_t time_now = esp_timer_get_time(); // En us
	double time = (time_now - time_since_last_speed) / 1000000.0;

	//ESP_LOGI("[SPEED]", "Time: %.5f\n", time);

	speed_x += output_data->accel[0] * time;
	speed_y += output_data->accel[1] * time;
	speed_z += output_data->accel[2] * time;

	speed = sqrt(speed_x*speed_x + speed_y*speed_y + speed_z*speed_z);
	output_data->speed = speed;
	time_since_last_speed = time_now;*/

	/* Profundidad */
	output_data->profundidad  = 0; // TODO

	filterUpdate();
	ToEulerAngle(SEq_1, SEq_2, SEq_3, SEq_4, output_data->roll, output_data->pitch, output_data->yaw);

	return ESP_OK;
}

void Imu::filterUpdate()
{
    // Local system variables
    float norm; // vector norm
    float SEqDot_omega_1, SEqDot_omega_2, SEqDot_omega_3, SEqDot_omega_4; // quaternion derrivative from gyroscopes elements
    float f_1, f_2, f_3; // objective function elements
    float J_11or24, J_12or23, J_13or22, J_14or21, J_32, J_33; // objective function Jacobian elements
    float SEqHatDot_1, SEqHatDot_2, SEqHatDot_3, SEqHatDot_4; // estimated direction of the gyroscope error
    // Axulirary variables to avoid reapeated calcualtions
    float halfSEq_1 = 0.5f * SEq_1;
    float halfSEq_2 = 0.5f * SEq_2;
    float halfSEq_3 = 0.5f * SEq_3;
    float halfSEq_4 = 0.5f * SEq_4;
    float twoSEq_1 = 2.0f * SEq_1;
    float twoSEq_2 = 2.0f * SEq_2;
    float twoSEq_3 = 2.0f * SEq_3;

    // Normalise the accelerometer measurement
    norm = sqrt(a_x * a_x + a_y * a_y + a_z * a_z);
    if(norm != 0){
		a_x /= norm;
		a_y /= norm;
		a_z /= norm;
    }
    // Compute the objective function and Jacobian
    f_1 = twoSEq_2 * SEq_4 - twoSEq_1 * SEq_3 - a_x;
    f_2 = twoSEq_1 * SEq_2 + twoSEq_3 * SEq_4 - a_y;
    f_3 = 1.0f - twoSEq_2 * SEq_2 - twoSEq_3 * SEq_3 - a_z;
    J_11or24 = twoSEq_3; // J_11 negated in matrix multiplication
    J_12or23 = 2.0f * SEq_4;
    J_13or22 = twoSEq_1; // J_12 negated in matrix multiplication
    J_14or21 = twoSEq_2;
    J_32 = 2.0f * J_14or21; // negated in matrix multiplication
    J_33 = 2.0f * J_11or24; // negated in matrix multiplication
    // Compute the gradient (matrix multiplication)
    SEqHatDot_1 = J_14or21 * f_2 - J_11or24 * f_1;
    SEqHatDot_2 = J_12or23 * f_1 + J_13or22 * f_2 - J_32 * f_3;
    SEqHatDot_3 = J_12or23 * f_2 - J_33 * f_3 - J_13or22 * f_1;
    SEqHatDot_4 = J_14or21 * f_1 + J_11or24 * f_2;
    // Normalise the gradient
    norm = sqrt(SEqHatDot_1 * SEqHatDot_1 + SEqHatDot_2 * SEqHatDot_2 + SEqHatDot_3 * SEqHatDot_3 + SEqHatDot_4 * SEqHatDot_4);
    SEqHatDot_1 /= norm;
    SEqHatDot_2 /= norm;
    SEqHatDot_3 /= norm;
    SEqHatDot_4 /= norm;
    // Compute the quaternion derrivative measured by gyroscopes
    SEqDot_omega_1 = -halfSEq_2 * w_x - halfSEq_3 * w_y - halfSEq_4 * w_z;
    SEqDot_omega_2 = halfSEq_1 * w_x + halfSEq_3 * w_z - halfSEq_4 * w_y;
    SEqDot_omega_3 = halfSEq_1 * w_y - halfSEq_2 * w_z + halfSEq_4 * w_x;
    SEqDot_omega_4 = halfSEq_1 * w_z + halfSEq_2 * w_y - halfSEq_3 * w_x;
    // Compute then integrate the estimated quaternion derrivative
    SEq_1 += (SEqDot_omega_1 - (beta * SEqHatDot_1)) * deltat;
    SEq_2 += (SEqDot_omega_2 - (beta * SEqHatDot_2)) * deltat;
    SEq_3 += (SEqDot_omega_3 - (beta * SEqHatDot_3)) * deltat;
    SEq_4 += (SEqDot_omega_4 - (beta * SEqHatDot_4)) * deltat;
    // Normalise quaternion
    norm = sqrt(SEq_1 * SEq_1 + SEq_2 * SEq_2 + SEq_3 * SEq_3 + SEq_4 * SEq_4);
    SEq_1 /= norm;
    SEq_2 /= norm;
    SEq_3 /= norm;
    SEq_4 /= norm;

}

/*
 * Hace una media al inicio con los valores tomados durante 5 segundos para tomar
 * como punto de referencia a la hora de medir el resto de variables mas adelante,
 * y asi poder saber la posicion relativa
 */

void Imu::CalibrateAccel(){
	
		//Fast compensation
	i2c->writeByte(ACCEL_ADDRESS, 0x36, 0x80); // set all accel offset compensation registers to zero
	i2c->writeByte(ACCEL_ADDRESS, 0x37, 0x00);  // set offset targets to 0, 0, and 0 g for x, y, z axes

	uint8_t offsetX, offsetY, offsetZ;
	uint8_t Reg_036 = 0;

	//Calibrate X
	i2c->writeByte(ACCEL_ADDRESS, 0x36, 0x20); //Start trigger
	vTaskDelay(pdMS_TO_TICKS(10));

	i2c->readByte(ACCEL_ADDRESS, 0x36, &Reg_036);
	while( !(Reg_036 & 0x10) ){
		i2c->readByte(ACCEL_ADDRESS, 0x36, &Reg_036);
		vTaskDelay(pdMS_TO_TICKS(10));
	}

	//Calibrate Y
	i2c->writeByte(ACCEL_ADDRESS, 0x36, 0x40); //Start trigger
	vTaskDelay(pdMS_TO_TICKS(10));

	i2c->readByte(ACCEL_ADDRESS, 0x36, &Reg_036);
	while( !(Reg_036 & 0x10) ){
		i2c->readByte(ACCEL_ADDRESS, 0x36, &Reg_036);
		vTaskDelay(pdMS_TO_TICKS(10));
	}

	//Calibrate Z
	i2c->writeByte(ACCEL_ADDRESS, 0x36, 0x60); //Start trigger
	vTaskDelay(pdMS_TO_TICKS(10));

	i2c->readByte(ACCEL_ADDRESS, 0x36, &Reg_036);
	while( !(Reg_036 & 0x10) ){
		i2c->readByte(ACCEL_ADDRESS, 0x36, &Reg_036);
		vTaskDelay(pdMS_TO_TICKS(10));
	}

	i2c->readByte(ACCEL_ADDRESS, 0x38, &offsetX);
	i2c->readByte(ACCEL_ADDRESS, 0x39, &offsetY);
	i2c->readByte(ACCEL_ADDRESS, 0x3A, &offsetZ);

	accel_x_offset =  (int8_t)offsetX / 12.8;
	accel_y_offset =  (int8_t)offsetY / 12.8;
	accel_z_offset =  (int8_t)offsetZ / 12.8;

}