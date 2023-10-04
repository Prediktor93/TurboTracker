#ifndef __ADC_H__
#define __ADC_H__

//#include <driver/adc.h>
#include <esp_adc/adc_continuous.h>
#include "driver/adc.h"
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "config.h"

class AdcReader{
public:
    AdcReader();

    void Init();
    int  ReadAdc1 (adc1_channel_t channel);
    int  ReadAdc2 (adc2_channel_t channel);
    void ReadSensors ();
    void GetSensorData(unsigned int SVal[NUM_SENSORS]);

private:

    adc_channel_t channelAdc;
    uint32_t adcNumber;

    SemaphoreHandle_t semaphore;
    unsigned int SensorVal[NUM_SENSORS];

};



#endif //__ADC_H__