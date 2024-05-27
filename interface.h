#include "mouse.h"
#include "flash.h"
#include "keyboard.h"
#include "func.h"
#include <stdio.h>
#include <stdlib.h>
#include <libusb-1.0/libusb.h>

void menu(libusb_device** list, ssize_t device_count);

void flash_menu(libusb_device* device);
//void menu2(libusb_device** list, ssize_t device_count);

