#include <libusb-1.0/libusb.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include "func.h"

#define FILENAME "test.bin"

#define KB 1024
#define MB (1024 * 1024)

libusb_device* find_flash(libusb_device** list_all, ssize_t device_count);
void flash_write_test(libusb_device* device, int block_size, int is_block_KB, int file_size, int is_MB);
void flash_read_test(libusb_device* device, int block_size, int is_block_KB, int file_size, int is_MB);

char* get_flash_path(libusb_device* device);
// int is_it_real_path(libusb_device* device, char* path);

int is_flash(libusb_device* device);
int is_flash_short(struct libusb_interface_descriptor interface_descriptor);
