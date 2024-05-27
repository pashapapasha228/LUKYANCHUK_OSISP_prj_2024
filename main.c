#include "interface.h"

int main() {
    ssize_t device_count;
    libusb_context* context = NULL;
    libusb_device** list;

    libusb_init_context(&context, NULL, 0);
    libusb_set_option(context, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_ERROR);

    device_count = libusb_get_device_list(context, &list);

    menu(list, device_count);

    libusb_free_device_list(list, 1);
    libusb_exit(context); 	
    return 0;
}