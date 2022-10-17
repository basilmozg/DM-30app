#include <drivers/gpio.h>

int pin_button_setup(const struct gpio_dt_spec *PIN_DI);

int pin_button_interrupt_setup(const struct gpio_dt_spec *PIN_DI, 
                               struct gpio_callback *PIN_DI_cb_data, 
                               void *PIN_DI_event,
                               gpio_flags_t GPIO_cal_direction);

int pin_led_setup(struct gpio_dt_spec *PIN_LED, gpio_flags_t extra_flag);