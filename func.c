#include "func.h"

void print_device_info(libusb_device* device) {
    unsigned char buffer_string[256];
    int lenght = 0;

    libusb_device_handle* device_handle = NULL; 

    struct libusb_device_descriptor device_descriptor;
    libusb_get_device_descriptor(device, &device_descriptor);

    printf("\n\tНомер версии USB: %x", device_descriptor.bcdUSB);
    printf("\n\tТип дескриптора: %x", device_descriptor.bDescriptorType);
    printf("\n\tКод класса: %x", device_descriptor.bDeviceClass);
    printf("\n\tИдентификатор производителя (VID): %x", device_descriptor.idVendor);
    printf("\n\tИдентификатор продукта (PID): %x", device_descriptor.idProduct);
        
    libusb_open(device, &device_handle);
        
    lenght = libusb_get_string_descriptor_ascii(device_handle, device_descriptor.iManufacturer, buffer_string, 256);
    printf("\n\tОписание производителя: %s", buffer_string);

    lenght = libusb_get_string_descriptor_ascii(device_handle, device_descriptor.iProduct, buffer_string, 256);
    printf("\n\tОписание продукта: %s", buffer_string);

    lenght = libusb_get_string_descriptor_ascii(device_handle, device_descriptor.iSerialNumber, buffer_string, 256);
    printf("\n\tСерийный номер продукта: %s", buffer_string);

    struct libusb_config_descriptor* config;
    libusb_get_active_config_descriptor(device, &config);

    printf("\n\tКоличество поддерживаемых интерфейсов: %x", config->bNumInterfaces);
    printf("\n\tИдентификатор конфигурации: %x", config->bConfigurationValue);

    for(int j = 0; j < config->interface->num_altsetting; j++) {
        struct libusb_interface_descriptor interface_descriptor;
        interface_descriptor = config->interface->altsetting[j];

        printf("\n\t\t Тип дескриптора: %x", interface_descriptor.bDescriptorType);
        printf("\n\t\t Код класса интерфейса: %x", interface_descriptor.bInterfaceClass);
        printf("\n\t\t Номер интерфейса: %x", interface_descriptor.bInterfaceNumber);
        printf("\n\t\t Код подкласса интерфейса: %x", interface_descriptor.bInterfaceSubClass);
        printf("\n\t\t Код протокола: %x", interface_descriptor.bInterfaceProtocol);
        printf("\n\t\t Количество эндпоинтов интерфейса: %x", interface_descriptor.bNumEndpoints);

        for(int k = 0; k < interface_descriptor.bNumEndpoints; k++) {
            struct libusb_endpoint_descriptor endpoint_descriptor;
            endpoint_descriptor = interface_descriptor.endpoint[k];

            printf("\n\t\t\t Адрес эндпоинта: %x", endpoint_descriptor.bEndpointAddress);
            printf("\n\t\t\t Тип: %x", endpoint_descriptor.bmAttributes);
            printf("\n\t\t\t Максимальный размер пакета: %d", endpoint_descriptor.wMaxPacketSize);
            printf("\n\t\t\t Интервал опроса конечной точки: %x", endpoint_descriptor.bInterval);
        }

        //if(is_it_flesh(interface_descriptor)) printf("\nЭТО ФЛЕШКА");
        //if(is_mouse_short(interface_descriptor)) printf("\nЭТО МЫШЬ");
        //if(is_it_keyboard(interface_descriptor)) printf("\nЭТО КЛАВИАТУРА");
    }

    libusb_free_config_descriptor(config);
    libusb_close(device_handle);

    printf("\n");
}

void print_shot_device_info(libusb_device* device) {
    unsigned char buffer_string[256];
    int lenght = 0;
    
    libusb_device_handle* device_handle = NULL; 

    struct libusb_device_descriptor device_descriptor;
    libusb_get_device_descriptor(device, &device_descriptor);

    libusb_open(device, &device_handle);

    lenght = libusb_get_string_descriptor_ascii(device_handle, device_descriptor.iManufacturer, buffer_string, 256);
    printf("%s ", buffer_string);

    lenght = libusb_get_string_descriptor_ascii(device_handle, device_descriptor.iProduct, buffer_string, 256);
    printf("%s ", buffer_string);

    libusb_close(device_handle);
}

// Относительно хоста
// is_IN = true для входного эндпоинта 
// is_IN = false для выходного
uint8_t get_device_endpoint(libusb_device* device, int is_IN) {
    uint8_t endpoint = 0;
    struct libusb_config_descriptor* config;
    libusb_get_active_config_descriptor(device, &config);

    for(int j = 0; j < config->interface->num_altsetting; j++) {
        struct libusb_interface_descriptor interface_descriptor;
        interface_descriptor = config->interface->altsetting[j];

        for(int k = 0; k < interface_descriptor.bNumEndpoints; k++) {
            struct libusb_endpoint_descriptor endpoint_descriptor;
            endpoint_descriptor = interface_descriptor.endpoint[k];

            if((endpoint_descriptor.bEndpointAddress > 0x80 && is_IN)
            || (endpoint_descriptor.bEndpointAddress < 0x10 && !is_IN)) {
                endpoint = endpoint_descriptor.bEndpointAddress;
            }
        }
    }

    libusb_free_config_descriptor(config);
    return endpoint;
}