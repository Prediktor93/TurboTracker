#ifndef __MOTOR_H__
#define __MOTOR_H__

#include "driver/mcpwm.h"
#include "driver/gpio.h"

typedef struct {
	mcpwm_unit_t      unit;
    mcpwm_timer_t     timer;
    mcpwm_generator_t op;
    gpio_num_t        a1;
    gpio_num_t        a2;
} motor_config_t;

enum Motor_num { 
    Motor1 = 1, 
    Motor2
};

class Motor{
public:
    Motor();

    void Init();

    void SetSpeed(Motor_num motor, int speed); //Speed value range [-100,100]

    // The motor will slowly stop because of inertia
    void Stop(Motor_num motor);

    // The motor will immediately stop
    void Brake(Motor_num motor);

private:

    motor_config_t Motor1_conf;
    motor_config_t Motor2_conf;

    //Changes the input pins value (Motor spin).
    void DirectionForward(Motor_num motor);
    void DirectionBackward(Motor_num motor);

};



#endif //__MOTOR_H__