#include "adc.h"
#include "config.h"

#include "esp_adc_cal.h"
#include "esp_log.h"
#include <string.h>

static const char *TAG = "ADC";

int NumIter = 5;

AdcReader::AdcReader()
{
}

void AdcReader::Init()
{

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(SENSOR_1_CHANNEL, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(SENSOR_2_CHANNEL, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(SENSOR_3_CHANNEL, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(SENSOR_4_CHANNEL, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(SENSOR_5_CHANNEL, ADC_ATTEN_DB_11);
    adc1_config_channel_atten(SENSOR_6_CHANNEL, ADC_ATTEN_DB_11);

    adc2_config_channel_atten(SENSOR_7_CHANNEL, ADC_ATTEN_DB_11);
    adc2_config_channel_atten(SENSOR_8_CHANNEL, ADC_ATTEN_DB_11);
    
}

int AdcReader::ReadAdc1(adc1_channel_t channel)
{
    return adc1_get_raw(channel);
}

int AdcReader::ReadAdc2(adc2_channel_t channel)
{
    int rawValue = 0;
    adc2_get_raw(channel, ADC_WIDTH_BIT_12, (int*)&rawValue);
    return rawValue;
}

 void AdcReader::ReadSensors (unsigned int* value)
 {

    memset(value, 0, sizeof(value)*8);

    for(uint8_t i =0; i < NumIter; i++){
        value[0] += adc1_get_raw(SENSOR_1_CHANNEL);
        value[1] += adc1_get_raw(SENSOR_2_CHANNEL);
        value[2] += adc1_get_raw(SENSOR_3_CHANNEL);
        value[3] += adc1_get_raw(SENSOR_4_CHANNEL);
        value[4] += adc1_get_raw(SENSOR_5_CHANNEL);
        value[5] += adc1_get_raw(SENSOR_6_CHANNEL);
        value[6] += ReadAdc2(SENSOR_7_CHANNEL);
        value[7] += ReadAdc2(SENSOR_8_CHANNEL);     
    }

    for(int i=0; i<NUM_SENSORS;i++){
        value[i] /= NumIter;
    }


    /*value[0] = adc1_get_raw(SENSOR_1_CHANNEL);
    value[1] = adc1_get_raw(SENSOR_2_CHANNEL);
    value[2] = adc1_get_raw(SENSOR_3_CHANNEL);
    value[3] = adc1_get_raw(SENSOR_4_CHANNEL);
    value[4] = adc1_get_raw(SENSOR_5_CHANNEL);
    value[5] = adc1_get_raw(SENSOR_6_CHANNEL);
    value[6] = ReadAdc2(SENSOR_7_CHANNEL);
    value[7] = ReadAdc2(SENSOR_8_CHANNEL);*/
 }