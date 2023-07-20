#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "rcpwm.h"
#include "adc.h"
#include "esp_log.h"
#include "Screen.h"
#include "config.h"
#include "i2c.h"
#include "imu.h"
#include "SDcard.h"
#include "driver/gpio.h"
#include "buttons.h"

extern "C" void app_main(void);

static const char* TAG = "Main";

Screen screen;
Imu imu;
SDCard sd;
Buttons bt;

void Restart()
{
    esp_restart();
}


extern QueueHandle_t interuptQueue;

void LED_Control_Task(void *params)
{
    int pinNumber, count = 0;
    printf("Task waiting\n");
    while (true)
    {
        if (xQueueReceive(interuptQueue, &pinNumber, portMAX_DELAY))
        {
            printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level(BUTTON_1_PIN));
        }
    }
}

void Init()
{

    //TODO: hacer un scan al inicio, ver que hay conectado y poner los devices a available segun eso


    printf("Init\n");
    ESP_ERROR_CHECK( i2c0.begin(SDA_PIN, SCL_PIN, I2C_MASTER_FREQ_HZ));
    i2c0.setTimeout(10);
    screen.Init();
    imu.Init(&i2c0);
    sd.Init();
    bt.Init();

    //Button 1
    xTaskCreate(LED_Control_Task, "LED_Control_Task", 2048, NULL, 1, NULL);


    printf("End\n");

}



void app_main(void)
{
    printf("Starting\n");
    
    Init();

    sd.Open("/sdcard/LOG2.txt");
    sd.Write("PRUEBAAA\n");

	imu_output imudata;


    while(1){

        /*if (gpio_get_level(BUTTON_1_PIN) == 1)
        {  
            printf("PRESSED!\n");    
        } */


        // Read IMU Data
        imu.Read(&imudata);

        //Print imu accel
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
