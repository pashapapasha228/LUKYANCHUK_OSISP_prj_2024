#include "mouse.h"

libusb_device* find_mouse(libusb_device** list_all, ssize_t device_count) {
    int rez;
    int indexes[device_count];
    int size_indexes = 0;

    printf("Вывод информации обо всех найденных мышах: \n");
    for(int i = 0; i < device_count; i++) {
        libusb_device* device = list_all[i];

        if(is_mouse(device)) {
            indexes[size_indexes] = i;
            printf("\n%d. ", ++size_indexes);
            print_shot_device_info(device);
            printf("\n");
        }
    }

    if(size_indexes == 0) {
        printf("\nНе подключена ни одна USB-мышь!");
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

void mouse_test(libusb_device* device) {
    uint8_t endpoint;
    libusb_device_handle* device_handle = NULL;
    char buf[BUF_SIZE];
    //int max_size = -1;
    int num_package = 0;
    int transferred;
    int rez;

    if(device == NULL)
        return;

    libusb_open(device, &device_handle);
    //libusb_set_configuration(device_handle, 1);

    if(libusb_kernel_driver_active(device_handle, 0)) {
        libusb_detach_kernel_driver(device_handle, 0);
        printf("\nДрайвер мыши отключен.");
    } else {
        printf("\nДрайвер мыши не подключен. Отключение не требуется.");
    }

    libusb_claim_interface(device_handle, 0);

    endpoint = get_device_endpoint(device, 1);
    printf("\nПолученный эндпоинт: %x", endpoint);
    //max_size = libusb_get_max_packet_size(device, endpoint);

    while(1) {
        memset(buf, 0, sizeof(buf));
        rez = libusb_interrupt_transfer(device_handle, endpoint,
            buf, sizeof(buf), &transferred, 0);

        num_package++;

        if (rez == LIBUSB_SUCCESS) {
            printf("\nПакет %d успешно получен.", num_package);
        }
        else {
            printf("\nОшибка при получении пакета %d: %s", num_package, libusb_strerror(rez));
            break;
        }

        // printf("\nПолучено %d байт", transferred);

        print_mouse_package(buf);

        // printf("\nПакет №%d: ", num_package);
        // for(int i = 0; i < transferred; i++) {
        //      printf("\n\tbuf[%d]: %d", i, buf[i]);
        // }
        // printf("\n");

        if(mouse_exit_condition(buf)) {
            printf("\nБыли нажаты ЛКМ ПКМ СКМ одновременно. Выход из программы.");
            break;
        }
    }

    libusb_release_interface(device_handle, 0);
    libusb_attach_kernel_driver(device_handle, 0);

    libusb_close(device_handle);
}

void print_mouse_package(int8_t buf[]) {
    printf("\n\tНажатые клавишы: ");
    if(buf[1]&0b00000001) printf(" ЛКМ ");
    if(buf[1]&0b00000010) printf(" ПКМ ");
    if(buf[1]&0b00000100) printf(" СКМ ");

    printf("\n\tИзменение координаты X: ");
    if(buf[3]==-1)  printf(" ВЛЕВО ");
    else if(buf[3]==0)  printf(" ВПРАВО ");
    printf(" на %d", abs(buf[2]));

    printf("\n\tИзменение координаты Y: ");
    if(buf[5]==-1)  printf(" ВВЕРХ ");
    else if(buf[5]==0)  printf(" ВНИЗ ");
    printf(" на %d", abs(buf[4]));

    
    printf("\n\tКолесо прокрутки:");
    if(buf[6]==-1)  printf(" ВНИЗ ");
    else if(buf[6]==1)  printf(" ВВЕРХ ");

    printf("\n");
}

// Нажатие на среднюю кнопку мыши
int mouse_exit_condition(int8_t buf[]) {
    return buf[1] == 0b00000111;
}

int is_mouse(libusb_device* device) {

    int rez = 0;
    struct libusb_config_descriptor* config;

    if(device == NULL)
        return 0;

    libusb_get_active_config_descriptor(device, &config);

    for(int j = 0; j < config->interface->num_altsetting; j++) {
        struct libusb_interface_descriptor interface_descriptor;
        interface_descriptor = config->interface->altsetting[j];
        rez = is_mouse_short(interface_descriptor);
        if(rez)
            break;
    }

    libusb_free_config_descriptor(config);

    return rez;
}

int is_mouse_short(struct libusb_interface_descriptor interface_descriptor) {
    if(interface_descriptor.bInterfaceClass == 0x03 // HID-устройство
        && interface_descriptor.bInterfaceSubClass == 0x01 // Устройство подкласса HID
        && interface_descriptor.bInterfaceProtocol == 0x02) { // Мышь
                return 1;
            }

    return 0;
}