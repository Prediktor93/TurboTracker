

//I2C
#define I2C_MASTER_FREQ_HZ 400000
#define SDA_PIN  GPIO_NUM_21
#define SCL_PIN  GPIO_NUM_22

//SDcard
#define MISO_PIN GPIO_NUM_19
#define MOSI_PIN GPIO_NUM_23
#define SCK_PIN  GPIO_NUM_18
#define CS_PIN   GPIO_NUM_5

//Button
#define BUTTON_1_PIN GPIO_NUM_16

//Motors
#define MOTOR_MAX_SPEED       100
#define MOTOR_1_PWM   GPIO_NUM_15
#define MOTOR_1_DIR_1 GPIO_NUM_12
#define MOTOR_1_DIR_2 GPIO_NUM_13
#define MOTOR_2_PWM   GPIO_NUM_2
#define MOTOR_2_DIR_1 GPIO_NUM_14
#define MOTOR_2_DIR_2 GPIO_NUM_27

//Fan
#define FAN_PIN     GPIO_NUM_4

//Sensors
#define NUM_SENSORS         8
#define SENSOR_1            GPIO_NUM_36
#define SENSOR_2            GPIO_NUM_39
#define SENSOR_3            GPIO_NUM_34
#define SENSOR_4            GPIO_NUM_35
#define SENSOR_5            GPIO_NUM_32
#define SENSOR_6            GPIO_NUM_33
#define SENSOR_7            GPIO_NUM_25
#define SENSOR_8            GPIO_NUM_26

#define SENSOR_1_CHANNEL    ADC1_CHANNEL_0
#define SENSOR_2_CHANNEL    ADC1_CHANNEL_3
#define SENSOR_3_CHANNEL    ADC1_CHANNEL_6
#define SENSOR_4_CHANNEL    ADC1_CHANNEL_7
#define SENSOR_5_CHANNEL    ADC1_CHANNEL_4
#define SENSOR_6_CHANNEL    ADC1_CHANNEL_5
#define SENSOR_7_CHANNEL    ADC2_CHANNEL_8
#define SENSOR_8_CHANNEL    ADC2_CHANNEL_9

//Menu
#define DEFAULT_SPEED_MENU_TIME 15