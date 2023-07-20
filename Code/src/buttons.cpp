#include "buttons.h"
#include "esp_log.h"
#include "config.h"
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/gpio.h"

static const char *TAG = "BUTTON_MGR";

QueueHandle_t interuptQueue;

void IRAM_ATTR gpio_interrupt_handler(void *args){
    int pinNumber = (int)args;
    xQueueSendFromISR(interuptQueue, &pinNumber, NULL);
}


Buttons::Buttons(){}

void Buttons::Init()
{

    //Configure Button GPIO
    esp_rom_gpio_pad_select_gpio(BUTTON_1_PIN);
    gpio_set_direction(BUTTON_1_PIN, GPIO_MODE_INPUT);
    gpio_pulldown_en(BUTTON_1_PIN);
    gpio_pullup_dis(BUTTON_1_PIN);
    gpio_set_intr_type(BUTTON_1_PIN, GPIO_INTR_POSEDGE);

    //Stablish interrupt
    interuptQueue = xQueueCreate(10, sizeof(int));

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_1_PIN, gpio_interrupt_handler, (void *)BUTTON_1_PIN);

}

