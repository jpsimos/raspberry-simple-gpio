/* Simple KernelFS GPIO Library */
/* Author Jacob Psimos 2017 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "gpiolib.h"

#define GPIOLIB_AUTO_EXPORT

static char buffer[GPIOLIB_BUFFER_SIZE];

uint8_t gpiolib_set_pin_mode(struct gpio_pin *pin, char mode, const uint8_t ignore_state){
	if(pin == (struct gpio_pin*)NULL){ return GPIOLIB_NULL | GPIOLIB_ERROR; }

	if(pin->mode_fd <= 0){ return GPIOLIB_ERROR | GPIOLIB_FD_CLOSED | GPIOLIB_FD_WRITE; }

	if(!ignore_state){
		if(pin->mode == mode){
			return GPIOLIB_SUCCESS;
		}
	}

	const char *mode_str = (mode == 'i' ? "in" : "out");
	const int length = (mode == 'i' ? 2 : 3 );

	if((mode = (char)write(pin->mode_fd, mode_str, length)) != -1){
		pin->mode = mode;
		lseek(pin->mode_fd, 0L, SEEK_SET);
		return GPIOLIB_SUCCESS | GPIOLIB_FD_WRITE;
	}

	return GPIOLIB_ERROR | GPIOLIB_FD_WRITE;
}

uint8_t gpiolib_set_pin_active_low(struct gpio_pin *pin, uint8_t active_low){
	if(pin == (struct gpio_pin*)NULL){ return GPIOLIB_NULL | GPIOLIB_ERROR; }

	if(pin->active_low_fd <= 0){ return GPIOLIB_ERROR | GPIOLIB_FD_CLOSED | GPIOLIB_FD_WRITE; }

	active_low = active_low ? '1' : '0';

	if(write(pin->active_low_fd, &active_low, 1) != -1){
		pin->active_low = active_low;
		lseek(pin->active_low_fd, 0L, SEEK_SET);
		return GPIOLIB_SUCCESS | GPIOLIB_FD_WRITE;
	}

	return GPIOLIB_ERROR | GPIOLIB_FD_WRITE;
}

uint8_t gpiolib_write_pin(struct gpio_pin *pin, uint8_t value){
	if(pin == (struct gpio_pin*)NULL){ return GPIOLIB_NULL | GPIOLIB_ERROR; }

	if(pin->value_fd <= 0){ return GPIOLIB_ERROR | GPIOLIB_FD_CLOSED | GPIOLIB_FD_WRITE; }

	value = value ? '1' : '0';

	if(write(pin->value_fd, &value, 1) != -1){
		pin->value = value;
		lseek(pin->value_fd, 0L, SEEK_SET);
		return GPIOLIB_SUCCESS | GPIOLIB_FD_WRITE;
	}

	return GPIOLIB_ERROR | GPIOLIB_FD_WRITE;
}

uint8_t gpiolib_read_pin(struct gpio_pin *pin){
	if(pin == (struct gpio_pin*)NULL){ return GPIOLIB_NULL | GPIOLIB_ERROR; }

	if(pin->value_fd <= 0){ return GPIOLIB_ERROR | GPIOLIB_FD_CLOSED | GPIOLIB_FD_READ; }

	if(read(pin->value_fd, &pin->value, 1) != -1){
		lseek(pin->value_fd, 0L, SEEK_SET);
		return GPIOLIB_SUCCESS | GPIOLIB_FD_READ;
	}

	return GPIOLIB_ERROR | GPIOLIB_FD_READ;
}

uint8_t gpiolib_export_pin(struct gpio_pin *pin){
	if(pin == (struct gpio_pin*)NULL){ return GPIOLIB_NULL | GPIOLIB_ERROR; }

	snprintf(buffer, GPIOLIB_BUFFER_SIZE, GPIOLIB_PATH_PREFIX "/gpio%u", pin->pin);

	if(pin->exported || !access(buffer, F_OK)){
		return GPIOLIB_SUCCESS | GPIOLIB_IO;
	}

	int fd = open(GPIOLIB_PATH_PREFIX "/export", O_WRONLY);
	int length = 0;
	if(fd > -1){
		length = snprintf(buffer, 4, "%u", pin->pin);
		write(fd, buffer, length);
		close(fd);
		pin->exported = 1;
		return GPIOLIB_SUCCESS | GPIOLIB_IO | GPIOLIB_FD_WRITE;
	}
	return (GPIOLIB_ERROR | GPIOLIB_IO);
}

uint8_t gpiolib_unexport_pin(struct gpio_pin *pin){
	if(pin == (struct gpio_pin*)NULL){ return GPIOLIB_NULL | GPIOLIB_ERROR; }

   int fd = open(GPIOLIB_PATH_PREFIX "/unexport", O_WRONLY);
   int length = 0;
   if(fd > -1){
      length = snprintf(buffer, 4, "%u", pin->pin);
      write(fd, buffer, length);
      close(fd);
		pin->exported = 0;
      return GPIOLIB_SUCCESS | GPIOLIB_IO | GPIOLIB_FD_WRITE;
   }
   return (GPIOLIB_ERROR | GPIOLIB_IO);
}

uint8_t gpiolib_load_pin_state(struct gpio_pin *pin){
	if(pin == (struct gpio_pin*)NULL){ return GPIOLIB_NULL | GPIOLIB_ERROR; }

	if(!(pin->mode_fd > 0 && pin->value_fd > 0 && pin->active_low_fd > 0)){
		return GPIOLIB_ERROR | GPIOLIB_FD_READ | GPIOLIB_IO;
	}

	int result = 0;

	result |= read(pin->value_fd, &pin->value, 1);
	result |= read(pin->mode_fd, &pin->mode, 1);
	result |= read(pin->active_low_fd, &pin->active_low, 1);

	if((result & -1) == -1){
		return GPIOLIB_ERROR | GPIOLIB_FD_READ | GPIOLIB_IO;
	}

	lseek(pin->value_fd, 0L, SEEK_SET);
	lseek(pin->mode_fd, 0L, SEEK_SET);
	lseek(pin->active_low_fd, 0L, SEEK_SET);

	return GPIOLIB_SUCCESS | GPIOLIB_FD_READ | GPIOLIB_IO;
}

uint8_t gpiolib_load_pin(struct gpio_pin *pin, const uint8_t num){
	if(pin == (struct gpio_pin*)NULL || num > 48){ return GPIOLIB_NULL | GPIOLIB_ERROR; }

	pin->pin = num;
	snprintf(pin->path, GPIOLIB_BUFFER_SIZE, GPIOLIB_PATH_PREFIX "/gpio%u", num);

	if(!access(pin->path, F_OK)){
		pin->exported = 1;
	}
	#ifdef GPIOLIB_AUTO_EXPORT
	else{
		if(gpiolib_export_pin(pin) & GPIOLIB_ERROR){
			return GPIOLIB_ERROR | GPIOLIB_IO;
		}
	}
	#endif

	if(!pin->mode_fd){
		snprintf(buffer, GPIOLIB_BUFFER_SIZE, GPIOLIB_PATH_PREFIX "/gpio%u/direction", num);
		pin->mode_fd = open(buffer, O_RDWR | O_SYNC);
	}

	if(!pin->value_fd){
		snprintf(buffer, GPIOLIB_BUFFER_SIZE, GPIOLIB_PATH_PREFIX "/gpio%u/value", num);
		pin->value_fd = open(buffer, O_RDWR | O_SYNC);
	}

	if(!pin->active_low_fd){
		snprintf(buffer, GPIOLIB_BUFFER_SIZE, GPIOLIB_PATH_PREFIX "/gpio%u/active_low", num);
		pin->active_low_fd = open(buffer, O_RDWR | O_SYNC);
	}

	if(!(pin->mode_fd > 0 && pin->value_fd > 0 && pin->active_low_fd > 0)){
		gpiolib_free_pin(pin);
		return GPIOLIB_ERROR | GPIOLIB_FD_CLOSED | GPIOLIB_IO;
	}

	return gpiolib_load_pin_state(pin);
	return GPIOLIB_SUCCESS | GPIOLIB_IO;
}

uint8_t gpiolib_free_pin(struct gpio_pin *pin){
	if(pin == (struct gpio_pin*)NULL){ return GPIOLIB_NULL | GPIOLIB_ERROR; }

	if(pin->mode_fd > 0){
		close(pin->mode_fd);
	}

	if(pin->value_fd > 0){
		close(pin->value_fd);
	}

	if(pin->active_low_fd > 0){
		close(pin->active_low_fd);
	}

//	return gpiolib_unexport_pin(pin);
	return GPIOLIB_SUCCESS | GPIOLIB_FD_CLOSED;
}



