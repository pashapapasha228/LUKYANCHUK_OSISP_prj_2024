#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <libusb-1.0/libusb.h>

void menu(libusb_device** list, ssize_t device_count);
void menu2(libusb_device** list, ssize_t device_count);
void test_speed(libusb_device* device, bool is_endpoint_in);
uint8_t get_device_endpoint(libusb_device* device, bool is_IN);

bool is_it_flesh(struct libusb_interface_descriptor interface_descriptor);
bool is_it_keyboard(struct libusb_interface_descriptor interface_descriptor);
bool is_it_mouse(struct libusb_interface_descriptor interface_descriptor);

libusb_device* find_usb_flash(libusb_device** list_all, ssize_t device_count);
void print_device_info(libusb_device* device);
void print_shot_device_info(libusb_device* device);

int main() {
    unsigned char buffer_string[256];
    int lenght = 0;

    uint8_t buf[2048];
    
    int buf_size;
    bool is_endpoint_in;
    int num_repet;

    uint8_t endpoint;
    int transferred;
    int rez;

    libusb_context* context = NULL;
    libusb_device** list;

    libusb_init_context(&context, NULL, 0);
    libusb_set_option(context, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_ERROR);

    ssize_t device_count = libusb_get_device_list(context, &list);

    menu(list, device_count);

    libusb_free_device_list(list, 1);
    libusb_exit(context); 	

    return 0;
}

// Одиночный девайс передавать через амперсанд
void menu(libusb_device** list, ssize_t device_count) {
    int rez;

    while(true) {
        rez = -1;

        //printf("\nВведите режим тестирования:");
        printf("\n1. Вывести на экран все подключенные USB-устройства.");
        printf("\n2. Вывести обнаруженные флешки.");
        printf("\n3. Выход из программы.");

        while(rez < 1 || rez > 3) {
            rewind(stdin);
            printf("\nВаш выбор: ");
            scanf("%d", &rez);
        }

        printf("\n");

        switch(rez) {
            case 1:
                for(int i = 0; i < device_count; i++) {
                    printf("\n%d. USB-устройство:", i+1);
                    print_device_info(list[i]);
                }
                break;
            case 2:
                menu2(list, device_count);
                break;
            case 3:
                return;
                break;
        }

        printf("\n");
    }
}

void menu2(libusb_device** list, ssize_t device_count) {
    int rez;

    libusb_device* device = NULL;
    device = find_usb_flash(list, device_count);

    if(device == NULL)
        return;

    while(true) {
        rez = -1;

        printf("\n1. Проверить скорость чтения.");
        printf("\n2. Проверить скорость записи.");
        printf("\n3. Безопастное извлечение флешки.");
        printf("\n4. Выход");

        while(rez < 1 || rez > 4) {
            rewind(stdin);
            printf("\nВаш выбор: ");
            scanf("%d", &rez);
        }

        printf("\n");

        switch(rez) {
            case 1:
                test_speed(device, true);
                break;
            case 2:
                test_speed(device, false);
                break;
            case 3:
                libusb_device_handle* device_handle = NULL; 
                libusb_open(device, &device_handle);
                if(libusb_kernel_driver_active(device_handle, 0)) {
                    libusb_detach_kernel_driver(device_handle, 0);
                    printf("\nДрайвер устройства был отключен.");
                } else {
                    printf("\nДрайвер устройства не был подключен.");
                }
                printf("\nТеперь вы можете безопастно извлечь флешку.");
                libusb_close(device_handle);
                break;
            case 4:
                return;
        }

        printf("\n");
    }
}

void test_speed(libusb_device* device, bool is_endpoint_in) {
    uint8_t buf[2048];
    int buf_size = -1;
    int max_size = -1;
    int num_repet = -1;
    uint8_t endpoint;
    int transferred;
    int rez;

    libusb_device_handle* device_handle = NULL;
    struct timeval tv_start, tv_stop; 
    unsigned long diff_msec;

    libusb_open(device, &device_handle);
    libusb_set_configuration(device_handle, 1);

    if(libusb_kernel_driver_active(device_handle, 0)) {
        libusb_detach_kernel_driver(device_handle, 0);
        printf("\nДрайвер устройства отключен");
    } else {
        printf("\nДрайвер устройства не подключен");
    }

    libusb_claim_interface(device_handle, 0);

        endpoint = get_device_endpoint(device, is_endpoint_in);
        max_size = libusb_get_max_packet_size(device, endpoint);
    printf("\nПолученный эндпоинт: %x", endpoint);
    printf("\nПолученная максимальная длина пакета: %d", max_size);

    while(buf_size < 1 || buf_size > max_size) {
        rewind(stdin);
        printf("\nВведите размер буфера (максимально - %d): ", max_size);
        scanf("%d", &buf_size);
    }

    while(num_repet < 1) {
        rewind(stdin);
        printf("\nВведите кол-во повторений: ");
        scanf("%d", &num_repet);
    }

    gettimeofday(&tv_start, NULL);

    // if(is_endpoint_in) {
    //     rez = libusb_bulk_transfer(device_handle, get_device_endpoint(device, !is_endpoint_in),
    //         buf, buf_size, &transferred, 1000);  
    // }

    for(int i = 0; i < num_repet; i++) {
        rez = libusb_bulk_transfer(device_handle, endpoint, buf, buf_size, &transferred, 1000);
        libusb_clear_halt(device_handle, endpoint);
            
        if (rez == LIBUSB_SUCCESS) {
            printf("\nПакет %d успешно получен. Количество полученных байт: %d\n", i+1, transferred);
        }
        else{
            printf("\nОшибка при получении пакета %d: %s", i+1, libusb_strerror(rez));
        }
    }
    gettimeofday(&tv_stop, NULL);

    diff_msec = (tv_stop.tv_sec - tv_start.tv_sec) * 1000L;
	diff_msec += (tv_stop.tv_usec - tv_start.tv_usec) / 1000L;

    // printf("\n1: %lu", tv_start.tv_usec);
    // printf("\n2: %lu", tv_stop.tv_usec);
    printf("\nДанные переданы за %lu миллисекунд", diff_msec);
    printf("\nСкорость записи: %d байт/с", (num_repet * buf_size * 1000L) / diff_msec);

    libusb_release_interface(device_handle, 0);
    libusb_attach_kernel_driver(device_handle, 0);

    libusb_close(device_handle);
}

// is_IN = true для входного эндпоинта
// is_IN = false для выходного
uint8_t get_device_endpoint(libusb_device* device, bool is_IN) {
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
                libusb_free_config_descriptor(config);
                return endpoint_descriptor.bEndpointAddress;
            }
        }
    }

    libusb_free_config_descriptor(config);
    return 0;
}

bool is_it_flesh(struct libusb_interface_descriptor interface_descriptor) {
    if(interface_descriptor.bInterfaceClass == 0x08 // mass storage class
        && (interface_descriptor.bInterfaceSubClass == 0x06 // SCSI
            || interface_descriptor.bInterfaceSubClass == 0x04) // UFI
        && interface_descriptor.bInterfaceProtocol == 0x50) { // Bulk-Only Transport
                return true;
                //printf("\nЭТО ФЛЕШКА!");
            }

    return false;
}

bool is_it_keyboard(struct libusb_interface_descriptor interface_descriptor) {
    if(interface_descriptor.bInterfaceClass == 0x03 // HID-устройство
        && interface_descriptor.bInterfaceSubClass == 0x01 // Устройство подкласса HID
        && (interface_descriptor.bInterfaceProtocol == 0x01
        || interface_descriptor.bInterfaceProtocol == 0x00)) { // Клавиатура
                return true;
                //printf("\nЭТО КЛАВИАТУРА!");
            }

    return false;
}

bool is_it_mouse(struct libusb_interface_descriptor interface_descriptor) {
    if(interface_descriptor.bInterfaceClass == 0x03 // HID-устройство
        && interface_descriptor.bInterfaceSubClass == 0x01 // Устройство подкласса HID
        && interface_descriptor.bInterfaceProtocol == 0x02) { // Мышь
                return true;
                //printf("\nЭТО МЫШЬ!");
            }

    return false;
}

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

        if(is_it_flesh(interface_descriptor)) printf("\nЭТО ФЛЕШКА");
        if(is_it_mouse(interface_descriptor)) printf("\nЭТО МЫШЬ");
        if(is_it_keyboard(interface_descriptor)) printf("\nЭТО КЛАВИАТУРА");
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

libusb_device* find_usb_flash(libusb_device** list_all, ssize_t device_count) {
    int rez;

    int indexes[device_count];
    int size_indexes = 0;
    
    printf("Вывод информации обо всех найденных флеш-накопителях: \n");

    for(int i = 0; i < device_count; i++) {
        libusb_device* device = list_all[i];

        struct libusb_device_descriptor device_descriptor;
        libusb_get_device_descriptor(device, &device_descriptor);

        if (device_descriptor.bDeviceClass == LIBUSB_CLASS_MASS_STORAGE) {
            indexes[size_indexes] = i;
            printf("\n%d. ", ++size_indexes);
            print_shot_device_info(device);
            printf("\n");
        } else {
            struct libusb_config_descriptor* config;
            libusb_get_active_config_descriptor(device, &config);

            for(int j = 0; j < config->interface->num_altsetting; j++) {
                struct libusb_interface_descriptor interface_descriptor;
                interface_descriptor = config->interface->altsetting[j];

                if(interface_descriptor.bInterfaceClass == LIBUSB_CLASS_MASS_STORAGE
                && (interface_descriptor.bInterfaceSubClass == 0x06 // SCSI
                    || interface_descriptor.bInterfaceSubClass == 0x04)) { // UFI
                        indexes[size_indexes] = i;
                        printf("\n%d. ", ++size_indexes);

                        print_shot_device_info(device);
                        printf("\n");
                }
            }

            libusb_free_config_descriptor(config);
        }
    }

    if(size_indexes == 0) {
        printf("\nНе подключен ни один флеш-накопитель!");
        return NULL;
    }

    rez = -1;

    while(rez < 1 || rez > size_indexes) {
        rewind(stdin);
        printf("\nВаш выбор: ");
        scanf("\n%d", &rez);
    }

    return list_all[indexes[rez-1]];
}