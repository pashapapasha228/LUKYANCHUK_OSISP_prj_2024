#include <libusb-1.0/libusb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "func.h"

#define BUF_SIZE 1024   

libusb_device* find_keyboard(libusb_device** list_all, ssize_t device_count);
void keyboard_test(libusb_device* device);
void print_keyboard_package(int8_t buf[]);

int keyboard_exit_condition(int8_t buf[]);
int is_keyboard(libusb_device* device);
int is_keyboard_short(struct libusb_interface_descriptor interface_descriptor);
