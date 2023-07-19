#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rcpwm.h"
#include "adc.h"
#include "esp_log.h"
//#include "SDCard.h"
//#include "I2C_conn.h"
#include "Screen.h"
#include "config.h"
#include "i2c.h"
#include "imu.h"

//#include <Adafruit_SSD1306.h>

extern "C" void app_main(void);

static const char* TAG = "Main";


Screen screen;
Imu imu;
//SDCard sdCard;


void Restart()
{
    esp_restart();
}

void Error()
{
    // Parar motor

    // Parpadear
    while(true){
        //LED rojo ON
        vTaskDelay(pdMS_TO_TICKS(500));
        //LED rojo OFF
        vTaskDelay(pdMS_TO_TICKS(500));
    }

}

void Init()
{

    //TODO: hacer un scan al inicio, ver que hay conectado y poner los devices a available segun eso


    printf("Init\n");
    	//ESP_ERROR_CHECK( i2c0.begin(GPIO_NUM_21, GPIO_NUM_22, (gpio_pullup_t)0x1, (gpio_pullup_t)0x1, 400000));
    ESP_ERROR_CHECK( i2c0.begin(SDA_PIN, SCL_PIN, I2C_MASTER_FREQ_HZ));
    i2c0.setTimeout(10);
    //i2c.Init();
    screen.Init();
    imu.Init(&i2c0);

    printf("End\n");

    //sdCard.Init(MISO_PIN, MOSI_PIN, SCK_PIN, CS_PIN);
    //sdCard.Open("/sdcard/test.txt");

}



void app_main(void)
{
    printf("Starting\n");
    //sdCard.Write("Test\n");
    Init();



	imu_output imudata;
    while(1){
        // Read IMU Data
        imu.Read(&imudata);
        /*printf("%.5f     %.5f     %.5f     %.5f     %.5f     %.5f\n",
                imudata.gyro[0],
                imudata.gyro[1],
                imudata.gyro[2],
                imudata.accel[0],
                imudata.accel[1],
                imudata.accel[2]);*/

        /*printf("%.5f     %.5f     %.5f\n",
                imudata.pitch,  //[-1.5,1.5] 1.55 vertical arriba, -1.55 vertical abajo
                imudata.yaw,    //[-3,3]     3 y -3 mirando atras, izda +, dcha -
                imudata.roll);  //[-3,3]     3 y -3 boca abajo   , izda +, dcha -*/

        printf("%.5f\n", imudata.totalAccel);

        //screen.PrintImuGyro(imudata.pitch, imudata.roll);
        screen.PrintImuAccel(imudata.accel[0], imudata.accel[1]);
        
        vTaskDelay(pdMS_TO_TICKS(10));

    }


    screen.Test();
    
    //isFreshStart = (esp_reset_reason() == ESP_RST_POWERON);
    esp_reset_reason_t resetReason;
    resetReason = esp_reset_reason();
    if(resetReason == ESP_RST_POWERON)
    {
        //Led verde
    }
    else if(resetReason == ESP_RST_BROWNOUT)
    {
        //Led rojo
    }


    uint32_t cycle = 0;


    while(true)
    {

        if(cycle % 20 == 0)
        {

        }


        vTaskDelay(pdMS_TO_TICKS(10));
        cycle++;
    }

}
