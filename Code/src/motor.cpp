#include "motor.h"
#include "config.h"
#include <stdio.h>

static const char *TAG = "MOTOR";

Motor::Motor(){}

//Due to a wiring error, Motor 2 direction cannot be changed

void Motor::Init()
{
    //Configure Motor 1 GPIO
    gpio_config_t motor1_1;
    motor1_1.intr_type = GPIO_INTR_DISABLE;
    motor1_1.mode = GPIO_MODE_OUTPUT;
    motor1_1.pin_bit_mask = 1 << MOTOR_1_DIR_1;
    motor1_1.pull_down_en =  GPIO_PULLDOWN_DISABLE;
    motor1_1.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&motor1_1);

    gpio_set_level(MOTOR_1_DIR_1, 0);

    gpio_config_t motor1_2;
    motor1_2.intr_type = GPIO_INTR_DISABLE;
    motor1_2.mode = GPIO_MODE_OUTPUT;
    motor1_2.pin_bit_mask = 1 << MOTOR_1_DIR_2;
    motor1_2.pull_down_en =  GPIO_PULLDOWN_DISABLE;
    motor1_2.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&motor1_2);

    gpio_set_level(MOTOR_1_DIR_2, 1);

    //Configure Motor 2 GPIO
    gpio_config_t motor2_1;
    motor2_1.intr_type = GPIO_INTR_DISABLE;
    motor2_1.mode = GPIO_MODE_OUTPUT;
    motor2_1.pin_bit_mask = 1 << MOTOR_2_DIR_1;
    motor2_1.pull_down_en =  GPIO_PULLDOWN_DISABLE;
    motor2_1.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&motor2_1);

    gpio_set_level(MOTOR_2_DIR_1, 1);

    gpio_config_t motor2_2;
    motor2_2.intr_type = GPIO_INTR_DISABLE;
    motor2_2.mode = GPIO_MODE_OUTPUT;
    motor2_2.pin_bit_mask = 1 << MOTOR_2_DIR_2;
    motor2_2.pull_down_en =  GPIO_PULLDOWN_DISABLE;
    motor2_2.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&motor2_2);

    gpio_set_level(MOTOR_2_DIR_2, 0);

    //gpio_set_direction(MOTOR_1_DIR_1, GPIO_MODE_OUTPUT);
    //gpio_set_direction(MOTOR_1_DIR_2, GPIO_MODE_OUTPUT);

    //gpio_set_direction(MOTOR_2_DIR_1, GPIO_MODE_OUTPUT);
    //gpio_set_direction(MOTOR_2_DIR_2, GPIO_MODE_OUTPUT);

    //gpio_set_level(MOTOR_1_DIR_1, 1);
    //gpio_set_level(MOTOR_1_DIR_2, 0);

    //gpio_set_level(MOTOR_2_DIR_1, 0);
    //gpio_set_level(MOTOR_2_DIR_2, 1);

    //Configure Motor PWM 1
    mcpwm_config_t pwm_conf;
    pwm_conf.frequency    = 10000;
    pwm_conf.cmpr_a       = 0;
    pwm_conf.cmpr_b       = 0;
    pwm_conf.counter_mode = MCPWM_UP_COUNTER;
    pwm_conf.duty_mode    = MCPWM_DUTY_MODE_0;

    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, MOTOR_1_PWM);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_conf);

    //Configure Motor PWM 2
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, MOTOR_2_PWM);
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_conf);

    //Save motor configs
    Motor1_conf.unit  = MCPWM_UNIT_0;
    Motor1_conf.timer = MCPWM_TIMER_0;
    Motor1_conf.op    = MCPWM_OPR_A;
    Motor1_conf.a1    = MOTOR_1_DIR_1;
    Motor1_conf.a2    = MOTOR_1_DIR_2;
    
    Motor2_conf.unit  = MCPWM_UNIT_0;
    Motor2_conf.timer = MCPWM_TIMER_1;
    Motor2_conf.op    = MCPWM_OPR_A;
    Motor2_conf.a1    = MOTOR_2_DIR_1;
    Motor2_conf.a2    = MOTOR_2_DIR_2;
}

void Motor::SetSpeed(Motor_num motor, int speed){

    //mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, abs(speed));
    if(speed >= -100 && speed <= 100){
        //(speed >= 0) ? DirectionForward(motor) : DirectionBackward(motor);
        if(motor == Motor1)
            mcpwm_set_duty(Motor1_conf.unit, Motor1_conf.timer, Motor1_conf.op, abs(speed));
        else   
            mcpwm_set_duty(Motor2_conf.unit, Motor2_conf.timer, Motor2_conf.op, abs(speed));
    }
}

void Motor::Stop(Motor_num motor){
    if(motor == Motor1){
        gpio_set_level(Motor1_conf.a1, 0);
        gpio_set_level(Motor1_conf.a2, 0);
    }else{                
        gpio_set_level(Motor2_conf.a1, 0);
        gpio_set_level(Motor2_conf.a2, 0);
    }
}

void Motor::Brake(Motor_num motor){
    if(motor == Motor1){
        gpio_set_level(Motor1_conf.a1, 1);
        gpio_set_level(Motor1_conf.a2, 1);
    }else{                
        gpio_set_level(Motor2_conf.a1, 1);
        gpio_set_level(Motor2_conf.a2, 1);
    }
}

void Motor::DirectionForward(Motor_num motor) {
    // Changes the GPIOs level as to change the direction the motor will spin, so it goes forwards
    if(motor == Motor1){
        gpio_set_level(Motor1_conf.a1, 0);
        gpio_set_level(Motor1_conf.a2, 1);
    }else{                
        gpio_set_level(Motor2_conf.a1, 0);
        gpio_set_level(Motor2_conf.a2, 1);
    }
}

void Motor::DirectionBackward(Motor_num motor) {
    // Changes the GPIOs level as to change the direction the motor will spin, so it goes forwards
    if(motor == Motor1){
        gpio_set_level(Motor1_conf.a1, 1);
        gpio_set_level(Motor1_conf.a2, 0);
    }else{                
        gpio_set_level(Motor2_conf.a1, 1);
        gpio_set_level(Motor2_conf.a2, 0);
    }
}