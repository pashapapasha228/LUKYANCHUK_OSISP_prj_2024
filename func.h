#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

void print_device_info(libusb_device* device);
void print_shot_device_info(libusb_device* device);
uint8_t get_device_endpoint(libusb_device* device, int is_IN);