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

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

extern "C" void app_main(void);

static const char* TAG = "Main";

Screen screen;
//SDCard sd;
Buttons bt;
Motor motor;
AdcReader sensors;

//Types 
typedef enum {
    INITIAL_TEST = 0,
    CALIBRATE_SENSORS,
    COUNTDOWN,
    RUN,
    LAST_STATE
} state_machine_t;

//Variables
//static bool ButtonPressed = false;
unsigned int SensorValues[NUM_SENSORS];
unsigned int Black[NUM_SENSORS];
unsigned int White[NUM_SENSORS];
unsigned int Mean[NUM_SENSORS];
bool CalibOK[NUM_SENSORS];

bool initDone = false;
state_machine_t state = INITIAL_TEST;

int buttonTime = esp_log_timestamp();

//Functions
int CalcLineVal();
float PID(int LineVal);
void Run(float PIDout, int LineVal);

extern QueueHandle_t interuptQueue;

void LED_Control_Task(void *params)
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
                if(state == INITIAL_TEST || state == CALIBRATE_SENSORS || state == RUN){
                    screen.ClearScreen();
                    state = static_cast<state_machine_t>(static_cast<int>(state) + 1);                
                }
                buttonTime = newButtonTime;
                //printf("GPIO %d was pressed %d times. The state is %d\n", pinNumber, count++, gpio_get_level(BUTTON_1_PIN));
            }

            //ButtonPressed = true;
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

    //sd.Init();
    bt.Init();
    screen.PrintText("BUTTON........OK", 3);

    //Button 1
    xTaskCreate(LED_Control_Task, "LED_Control_Task", 2048, NULL, 1, NULL);

    //Motors
    motor.Init();
    screen.PrintText("MOTORS........OK", 4);

    //Fan - TODO descomentar
    //gpio_set_direction(FAN_PIN, GPIO_MODE_OUTPUT);
    //gpio_set_level(FAN_PIN, 1);
    
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



void app_main(void)
{
    printf("Starting\n");
    
    //Init();

    //TODO - Check if SD esta OK antes de escribir
    //sd.Open("/sdcard/LOG2.txt");
    //sd.Write("PRUEBAAA 2\n");

    while(true){

        switch(state){

            case INITIAL_TEST:
                if(!initDone){
                    Init();
                    initDone = true;
                }

                break;

            case CALIBRATE_SENSORS:
                sensors.ReadSensors(SensorValues);

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

            //TODO - Si se pulsa el boton y no esta ese sensor a OK, darlo por malo (media de los dos de los lados?)
            //TODO - Contar las vueltas

            case COUNTDOWN: 
                //screen.Countdown(); -- TODO uncomment
                screen.ClearScreen();
                state=RUN;
            break;

            case RUN: 
                //screen.PrintText("    RUNNING     ", 3);
                sensors.ReadSensors(SensorValues);
                int LineVal;
                LineVal = CalcLineVal();

                //TODO - Not necessary
                /*char V[5];
                char text2[17];
                sprintf(V, "%04d", LineVal);
                strcpy(text2, "      ");
                strcat(text2, V);  
                screen.PrintText(text2, 4);*/

                float PIDout;
                PIDout = PID(LineVal);
                Run(PIDout, LineVal);

                //Run2(LineVal);

                break;

            case LAST_STATE: 
            
                motor.SetSpeed(Motor1, 0);
                motor.SetSpeed(Motor2, 0);
                break;

        }

        vTaskDelay(pdMS_TO_TICKS(10));

    }
}

int CalcLineVal(){

    unsigned int SensorValuesNorm[NUM_SENSORS];
    bool LineFound = false;
    unsigned long SensorWeights = 0;
    unsigned int  SensorTotal   = 0;
    unsigned int  ReturnVal     = 0;
    
    for(int i=0; i<NUM_SENSORS;i++){

        if(CalibOK[i]){

            if(SensorValues[i] < White[i]) SensorValues[i] = White[i];
            if(SensorValues[i] > Black[i]) SensorValues[i] = Black[i];

            SensorValuesNorm[i] = (SensorValues[i]-White[i]) * 4095.0 / Black[i];
            if(SensorValuesNorm[i] >= Mean[i]) LineFound = true;

        } else SensorValuesNorm[i] = 0;

        SensorWeights += (unsigned long)SensorValuesNorm[i]*(i+1)*100.0;
        SensorTotal   += SensorValuesNorm[i];

    }

    /*if(!LineFound){
        ReturnVal = 100; //Turn right
    }else{*/
        ReturnVal = SensorWeights / SensorTotal;
    //}

    ReturnVal = ReturnVal - (NUM_SENSORS+1) * 100/2;

    return ReturnVal; //Returns a value between -350, 350 with the line position

}

//Vars
int proportional = 0;
int proportional_last = 0;
long integral = 0;
long integral_max = 100;
int derivative = 0;
int PIDmax = 100;
float Kp = 0.4;
float Ki = 0;
float Kd = 10;

float PID(int LineVal)
{
  proportional = LineVal;

  /*integral = integral + proportional;
  if(abs(integral) > integral_max)
  {
    if(integral > 0)
      integral = integral_max;
    else
      integral = -integral_max;
  }*/

  derivative = proportional - proportional_last;
  proportional_last = proportional;

  return (float)(proportional * Kp /*+ integral * Ki*/ + derivative * Kd);
}



void Run(float PIDout, int LineVal){
    int motorL, motorR = 0;

    /*if(PIDout > PIDmax)
        PIDout = PIDmax;
    else if(PIDout < -PIDmax)
        PIDout = -PIDmax;*/

    /*if(abs(LineVal) < 20 ){
        motorL = 100;
        motorR = 100;
    }else{*/
        motorL = 50 - PIDout;
        motorR = 50 + PIDout;
    //}

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
    
    motor.SetSpeed(Motor1, motorL/3); //TODO: ponerlo a tope
    motor.SetSpeed(Motor2, motorR/3);
}