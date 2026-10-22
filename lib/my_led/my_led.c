#include "my_led.h"
#include <stdio.h>
#include "driver/gpio.h"

#define BLINK_GPIO GPIO_NUM_2

static void configure_led(void)
{
  gpio_reset_pin(BLINK_GPIO);
  /* Set the GPIO as a push/pull output */
  gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);
}

void onOffLed(int onOFF)
{
  configure_led();
  gpio_set_level(BLINK_GPIO, onOFF);
}
