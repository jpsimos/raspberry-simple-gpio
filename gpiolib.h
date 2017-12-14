/* Simple KernelFS GPIO Library */
/* Author Jacob Psimos 2017 */

#ifndef __GPIOLIB_H
#define __GPIOLIB_H

#include <stdint.h>

#define GPIOLIB_VERSION "0.1b"

#define GPIOLIB_BUFFER_SIZE 128
#define GPIOLIB_PATH_PREFIX "/sys/class/gpio"

#define GPIOLIB_SUCCESS 0x01
#define GPIOLIB_NULL 0x02
#define GPIOLIB_IO 0x04
#define GPIOLIB_FD_OPENED 0x08
#define GPIOLIB_FD_CLOSED 0x10
#define GPIOLIB_FD_WRITE 0x20
#define GPIOLIB_FD_READ 0x40
#define GPIOLIB_ERROR 0x80

struct gpio_pin{
	uint8_t pin;
	uint8_t exported;
	char path[128];
	int mode_fd;
	int value_fd;
	int active_low_fd;
	char mode;
	uint8_t value;
	uint8_t active_low;
} __attribute((packed));

uint8_t gpiolib_load_pin_state(struct gpio_pin *pin);
uint8_t gpiolib_export_pin(struct gpio_pin *pin);
uint8_t gpiolib_unexport_pin(struct gpio_pin *pin);
uint8_t gpiolib_set_pin_mode(struct gpio_pin *pin, char mode, const uint8_t ignore_state);
uint8_t gpiolib_write_pin(struct gpio_pin *pin, uint8_t value);
uint8_t gpiolib_read_pin(struct gpio_pin *pin);
uint8_t gpiolib_load_pin(struct gpio_pin *pin, const uint8_t num);
uint8_t gpiolib_free_pin(struct gpio_pin *pin);

#endif
