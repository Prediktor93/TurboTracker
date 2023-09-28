#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "adc.h"
#include "esp_log.h"
#include "Screen.h"
#include "config.h"
#include "i2c.h"
#include "SDcard.h"
#include "driver/gpio.h"
#include "buttons.h"
#include "motor.h"
#include "adc.h"
#include <cstring>

//#define SD_AVAILABLE

extern "C" void app_main(void);

static const char* TAG = "Main";

Screen screen;
Buttons bt;
Motor motor;
AdcReader sensors;

#ifdef SD_AVAILABLE
    SDCard sd;
#endif

//Types 
typedef enum {
    START = 0,
    CALIBRATE_SENSORS,
    COUNTDOWN,
    RUN,
    LAST_STATE
} state_machine_t;

//Variables
unsigned int SensorValues[NUM_SENSORS];
unsigned int Black[NUM_SENSORS];
unsigned int White[NUM_SENSORS];
unsigned int Mean[NUM_SENSORS];
bool CalibOK[NUM_SENSORS];

state_machine_t state = START;

int buttonTime = esp_log_timestamp();

//Functions
int CalcLineVal();
float PID(int LineVal);
void Run(float PIDout, int LineVal);

extern QueueHandle_t interuptQueue;

static TaskHandle_t mainCoreHandle = NULL;
static TaskHandle_t sensorCoreHandle = NULL;

void Button_Control_Task(void *params)
{
    int pinNumber, count = 0;
    printf("Task waiting\n");
    while (true)
    {
        if (xQueueReceive(interuptQueue, &pinNumber, portMAX_DELAY))
        {

            int newButtonTime = esp_log_timestamp();

            //Check to avoid multiple unwanted button pressed
            if(newButtonTime - buttonTime > 300){
                if(state == START || state == CALIBRATE_SENSORS || state == RUN){
                    screen.ClearScreen();
                    state = static_cast<state_machine_t>(static_cast<int>(state) + 1);                
                }
                buttonTime = newButtonTime;
                //printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level(BUTTON_1_PIN));
            }
        }
    }
}

void Init()
{

    //TODO: hacer un scan al inicio, ver que hay conectado y poner los devices a available segun eso
    //TODO: meter esta macro a las functiones que devuelven esp_err_t 
    //   ESP_ERROR_CHECK

    printf("Init\n");
    ESP_ERROR_CHECK( i2c0.begin(SDA_PIN, SCL_PIN, I2C_MASTER_FREQ_HZ));
    i2c0.setTimeout(10);
    screen.Init();

    screen.PrintText("SCREEN........OK", 2);

    #ifdef SD_AVAILABLE
        sd.Init();  
        screen.PrintText("SD............OK", 1);
    #endif

    bt.Init();
    screen.PrintText("BUTTON........OK", 3);

    //Button 1
    xTaskCreate(Button_Control_Task, "Button_Control_Task", 2048, NULL, 1, NULL);

    //Motors
    motor.Init();
    screen.PrintText("MOTORS........OK", 4);

    //Fan
    gpio_set_direction(FAN_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(FAN_PIN, 1);
    
    //Sensors
    sensors.Init();

    for(int i=0; i<NUM_SENSORS;i++){
        Black[i]   = 2500;
        White[i]   = 1500;
        CalibOK[i] = false;
    }

    screen.PrintText("SENSORS.......OK", 5);

    printf("End\n"); 
    screen.PrintText("  PRESS BUTTON  ", 7);

}

void sensorThread(void *arg)
{
    ESP_LOGE(TAG, "Iniciando CORE B");
    while(true){
        sensors.ReadSensors(SensorValues);
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void mainThread(void *arg){
    
    ESP_LOGE(TAG, "Iniciando CORE A");

    while(true){

        switch(state){

            case START:
                break;

            case CALIBRATE_SENSORS:

                char text[17];
                char Sw[5];
                char Sb[5];
                char Num[2];

                for(int i=0; i<NUM_SENSORS;i++){
                    if(SensorValues[i] > Black[i]) Black[i] = SensorValues[i];
                    if(SensorValues[i] < White[i]) White[i] = SensorValues[i];
                    if(Black[i] > 2500 && White[i] < 1500){
                         CalibOK[i] = true;
                         Mean[i] = (Black[i] + White[i]) / 2.0;
                    }
                    
                    sprintf(Sw, "%04d", White[i]);
                    sprintf(Sb, "%04d", Black[i]);
                    sprintf(Num, "%01d", i+1);
                    strcpy(text, "S"); strcat(text, Num); strcat(text, " ");               
                    strcat(text, Sw);              
                    strcat(text, " ");           
                    strcat(text, Sb);
                    if(CalibOK[i]) strcat(text, " OK");
                    else           strcat(text, " --");
                    screen.PrintText(text, i);
                }

                break;

            case COUNTDOWN: 
                //screen.Countdown(); -- TODO uncomment
                screen.ClearScreen();
                state=RUN;
            break;

            case RUN: 
                //screen.PrintText("    RUNNING     ", 3);

                int LineVal;
                LineVal = 0;
                
                #ifdef SD_AVAILABLE
                sd.Write("\nT:%d", esp_log_timestamp());
                sd.Write("S:%d;%d;%d;%d;%d;%d;%d;%d ", SensorValues[0], SensorValues[1], SensorValues[2], SensorValues[3],
                                                       SensorValues[4], SensorValues[5], SensorValues[6], SensorValues[7]);
                #endif

                LineVal = CalcLineVal();

                /*char V[5];
                char text2[17];
                sprintf(V, "%04d", LineVal);
                strcpy(text2, "      ");
                strcat(text2, V);  
                screen.PrintText(text2, 4);*/

                float PIDout;
                PIDout = PID(LineVal);    

                #ifdef SD_AVAILABLE   
                    sd.Write("L:%d ", LineVal);
                    sd.Write("P:%d ", PIDout);
                #endif

                Run(PIDout, LineVal);

                break;

            case LAST_STATE: 
            
                motor.SetSpeed(Motor1, 0);
                motor.SetSpeed(Motor2, 0);
                break;

        }

        vTaskDelay(pdMS_TO_TICKS(10));

    }
}

void app_main(void)
{
    printf("Starting\n");
     
    Init();
    
    //TODO - Check if SD esta OK antes de escribir
    #ifdef SD_AVAILABLE
        sd.Open("/sdcard/LOG2.txt");
        sd.Write("== INIT ==\n");
    #endif

    xTaskCreatePinnedToCore(mainThread, "Main_core",   4096, NULL, 10, &mainCoreHandle, 0);
    xTaskCreatePinnedToCore(sensorThread, "Sensor_Core", 4096, NULL, 10, &sensorCoreHandle, 1);
  
}

int CalcLineVal(){

    unsigned int SensorValuesNorm[NUM_SENSORS];
    bool LineFound = false;
    unsigned long SensorWeights = 0;
    unsigned int  SensorTotal   = 0;
    unsigned int  ReturnVal     = 0;
    
    /*for(int i=0; i<NUM_SENSORS;i++){

        if(CalibOK[i]){

            if(SensorValues[i] < White[i]) SensorValues[i] = White[i];
            if(SensorValues[i] > Black[i]) SensorValues[i] = Black[i];

            SensorValuesNorm[i] = (SensorValues[i]-White[i]) * 4095.0 / Black[i];
            if(SensorValuesNorm[i] >= Mean[i]) LineFound = true;

        } else SensorValuesNorm[i] = 0;

        SensorWeights += (unsigned long)SensorValuesNorm[i]*(i+1)*100.0;
        SensorTotal   += SensorValuesNorm[i];

    }*/

    for(int i=0; i<NUM_SENSORS;i++){

        SensorWeights += (unsigned long)SensorValues[i]*(i+1)*100.0;
        SensorTotal   += SensorValues[i];

    }

    ReturnVal = SensorWeights / SensorTotal;
    ReturnVal = ReturnVal - (NUM_SENSORS+1) * 100/2;

    /*if(!LineFound){
        ReturnVal = 300; //Turn left
    }*/

    return ReturnVal; //Returns a value between -350, 350 with the line position

}

//Vars
int proportional = 0;
int proportional_last = 0;
int derivative = 0;
float Kp = 0.6; //0.95
float Kd = 2.5; //2.5

float PID(int LineVal)
{
  proportional = LineVal;

  derivative = proportional - proportional_last;
  proportional_last = proportional;

  return (float)(proportional * Kp + derivative * Kd);
}



void Run(float PIDout, int LineVal){
    int motorL, motorR = 0;

    motorL = 100 - PIDout;
    motorR = 100 + PIDout;
       
    #ifdef SD_AVAILABLE
        sd.Write("ML:%d ", motorL);              
        sd.Write("MR:%d", motorR);
    #endif

    if(motorL > 100)    motorL = 100;
    else if(motorL < 0) motorL = 0;
    if(motorR > 100)    motorR = 100;
    else if(motorR < 0) motorR = 0;

   /*char L[4];
    char R[4];
    char text2[17];
    sprintf(L, "%03d", motorL);
    sprintf(R, "%03d", motorR);
    strcpy(text2, "  ");
    strcat(text2, L);  
    strcat(text2, "        "); 
    strcat(text2, R); 
    screen.PrintText(text2, 6);*/
    
    motor.SetSpeed(Motor1, motorL);
    motor.SetSpeed(Motor2, motorR);
}