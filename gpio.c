/* GPIO utility by jacob psimos 2017 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include "gpio.h"
#include "gpiolib.h"

#define CMD_READ 1
#define CMD_WRITE 2
#define CMD_MODE 3
#define CMD_EXPORT 4
#define CMD_UNEXPORT 5

static const char parse_arguments(const int argc, const char **argv, uint8_t *pin, uint8_t *value, char *mode);

int main(int argc, char **argv){

	if(argc < 2){
		printf("GPIO Utility by Jacob Psimos\n");
		printf("usage: [read, write, mode] options\n");
		printf("\tread pin#\n");
		printf("\twrite pin# 1,0\n");
		printf("\tmode pin# in,out\n");
		printf("\nGPIO version: %s (gpiolib version: %s)\n", VERSION, GPIOLIB_VERSION);
		return 0;
	}

	uint8_t pin;
	uint8_t value;
	char mode;
	char cmd;
	uint8_t result;
	struct gpio_pin gpin;

	if((cmd = parse_arguments(argc, (const char**)argv, &pin, &value, &mode)) == 0){
		return 1;
	}

	memset(&gpin, 0, sizeof(struct gpio_pin));

	if((result = gpiolib_load_pin(&gpin,pin)) & GPIOLIB_ERROR){
		fprintf(stderr, "Unable to load GPIO pin (result %02x)\n", result);
		return 1;
	}

	switch(cmd){
		case CMD_READ:
			if((result = gpiolib_read_pin(&gpin)) & GPIOLIB_SUCCESS){
				printf("%u\n", gpin.value - '0');
			}
		break;
		case CMD_WRITE:
			result = gpiolib_write_pin(&gpin, value);
		break;
		case CMD_MODE:
			result = gpiolib_set_pin_mode(&gpin, mode, 0);
		break;
		case CMD_EXPORT:
			result = gpiolib_export_pin(&gpin);
		break;
		case CMD_UNEXPORT:
			result = gpiolib_unexport_pin(&gpin);
		break;
		default:
			result = GPIOLIB_ERROR;
		break;
	}

	if(result & GPIOLIB_SUCCESS){
		result = gpiolib_free_pin(&gpin);
	}

	if(result & GPIOLIB_ERROR){
		fprintf(stderr, "gpiolib error: %02x\n", result);
		return 1;
	}

	return 0;
}

static const char parse_arguments(const int argc, const char **argv, uint8_t *pin, uint8_t *value, char *mode){
	if(argv == (const char**)NULL || pin == (uint8_t*)NULL || value == (uint8_t*)NULL || mode == (char*)NULL){
		return 0;
	}

	*pin = 0;
	*value = 0;
	*mode = 0;

	int temp = 0;

	if(!strncmp(argv[1], "read", 4)){
		if(argc == 3){
			temp = atoi(argv[2]);
			*pin = (uint8_t)(temp & 0xFF);
			return CMD_READ;
		}else{
			fprintf(stderr, "Missing pin number.\n");
		}
		return 0;
	}

	if(!strncmp(argv[1], "write", 5)){
		if(argc == 4){
			temp = atoi(argv[2]);
			*pin = (uint8_t)(temp & 0xFF);
			*value = argv[3][0] == '1' ? 1 : 0;
			return CMD_WRITE;
		}else{
			fprintf(stderr, "Missing pin number or value.\n");
		}
		return 0;
	}


	if(!strncmp(argv[1], "mode", 4)){
		if(argc == 4){
			temp = atoi(argv[2]);
			*pin = (uint8_t)(temp & 0xFF);
			if(!strncmp(argv[3], "in", 2)){ *mode = 'i'; }
			else if(!strncmp(argv[3], "out", 3)) { *mode = 'o'; }
			else { fprintf(stderr, "Invalid mode option.\n"); return 0; }
			return CMD_MODE;
		}else{
			fprintf(stderr, "Missing pin number or mode.\n");
		}
		return 0;
	}

	if(!strncmp(argv[1], "export", 6)){
		if(argc == 3){
			temp = atoi(argv[2]);
			*pin = (uint8_t)(temp & 0xFF);
			return CMD_EXPORT;
		}else{
			fprintf(stderr, "Missing pin number.\n");
		}
		return 0;
	}

	if(!strncmp(argv[1], "unexport", 8)){
		if(argc == 3){
			temp = atoi(argv[2]);
			*pin = (uint8_t)(temp & 0xFF);
			return CMD_UNEXPORT;
		}else{
			fprintf(stderr, "Missing pin number.\n");
		}
	}

	return 0;
}
