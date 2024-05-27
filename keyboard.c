#include "keyboard.h"

libusb_device* find_keyboard(libusb_device** list_all, ssize_t device_count) {
    int rez;
    int indexes[device_count];
    int size_indexes = 0;

    printf("Вывод информации обо всех найденных клавиатурах: \n");
    for(int i = 0; i < device_count; i++) {
        libusb_device* device = list_all[i];

        if(is_keyboard(device)) {
            indexes[size_indexes] = i;
            printf("\n%d. ", ++size_indexes);
            print_shot_device_info(device);
            printf("\n");
        }
    }

    if(size_indexes == 0) {
        printf("\nНе подключена ни одна USB-клавиатура!");
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

void keyboard_test(libusb_device* device) {
    uint8_t endpoint;
    libusb_device_handle* device_handle = NULL;
    char buf[8];
    int max_size = -1;
    int num_package = 0;
    int transferred;
    int rez;

    libusb_open(device, &device_handle);
    //libusb_set_configuration(device_handle, 1);

    if(libusb_kernel_driver_active(device_handle, 0)) {
        libusb_detach_kernel_driver(device_handle, 0);
        printf("\nДрайвер клавиатуры отключен.");
    } else {
        printf("\nДрайвер клавиатуры не подключен. Отключение не требуется.");
    }

    libusb_claim_interface(device_handle, 0);

    endpoint = get_device_endpoint(device, 1);
    printf("\nПолученный эндпоинт: %x", endpoint);
    max_size = libusb_get_max_packet_size(device, endpoint);
    printf("\nМаксимальный размер пакета %d", max_size);

    while(1) {
        memset(buf, 0, sizeof(buf));
        rez = libusb_interrupt_transfer(device_handle, endpoint,
            buf, sizeof(buf), &transferred, 0);

        if(keyboard_exit_condition(buf)) {
            printf("\nБыла нажата комбинация CTRL+ESC. Выход из программы.");
            break;
        }

        num_package++;

        // if (rez == LIBUSB_SUCCESS) {
        //     printf("\n\nПакет %d успешно получен.", num_package);
        // }
        // else {
        //     printf("\nОшибка при получении пакета %d: %s", num_package, libusb_strerror(rez));
        //     break;
        // }

        // printf("\nПолучено %d байт", transferred);

        print_keyboard_package(buf);

        // printf("\nПакет №%d: ", num_package);
        // for(int i = 0; i < transferred; i++) {
        //      printf("\n\tbuf[%d]: %d", i, buf[i]);
        // }
        // printf("\n");
    }

    libusb_release_interface(device_handle, 0);
    libusb_attach_kernel_driver(device_handle, 0);

    libusb_close(device_handle);
}

void print_keyboard_package(int8_t buf[]) {
    char c;
    int is_shift = 0;

    printf("\n\tНажатые клавишы: ");
    if(buf[0]&0b00000001) printf(" CTRL ");
    if(buf[0]&0b00000100) printf(" ALT ");
    if(buf[0]&0b00000010) {
        printf(" SHIFT ");
        is_shift = 1;
    } 
    if(buf[0]&0b00010000) printf(" Num Lock ");
    if(buf[0]&0b00100000) printf(" R_CTRL ");
    if(buf[0]&0b10000000) printf(" R_ALT ");
    if(buf[0]&0b01000000) {
        printf(" R_SHIFT ");
        is_shift = 1;
    } 

    for(int i = 1; i < 8; i++) {
        // Буквы
        if(buf[i] >= 4 && buf[i] <= 29) {
            c = buf[i] + 93 - 32*is_shift;
            printf(" %c ", c);
            continue;
        }
        // f1 - f12
        if(buf[i] >= 58 && buf[i] <= 69) {
            printf(" f%d ", buf[i] - 57);
            continue; 
        }

        switch(buf[i]) {
            case 30:
                is_shift?printf(" ! "):printf(" 1 ");
                break;
            case 31:
                is_shift?printf(" @ "):printf(" 2 ");
                break;
            case 32:
                is_shift?printf(" # "):printf(" 3 ");
                break;
            case 33:
                is_shift?printf(" $ "):printf(" 4 ");
                break;
            case 34:
                is_shift?printf(" \% "):printf(" 5 ");
                break;
            case 35:
                is_shift?printf(" ^ "):printf(" 6 ");
                break;
            case 36:
                is_shift?printf(" & "):printf(" 7 ");
                break;
            case 37:
                is_shift?printf(" * "):printf(" 8 ");
                break;
            case 38:
                is_shift?printf(" ( "):printf(" 9 ");
                break;
            case 39:
                is_shift?printf(" ) "):printf(" 0 ");
                break;
            case 40:
                printf(" ENTER ");
                break;
            case 41:
                printf(" ESC ");
                break;
            case 42:
                printf(" BACKSPACE ");
                break;  
            case 43:
                printf(" Tab ");
                break;  
            case 44:
                printf(" Spacebar ");
                break;
            case 45:
                is_shift?printf(" - "):printf(" _ ");
                break; 
            case 46:
                is_shift?printf(" = "):printf(" + ");
                break; 
            case 47:
                is_shift?printf(" [ "):printf(" { ");
                break;   
            case 48:
                is_shift?printf(" ] "):printf(" } ");
                break;    
            case 49:
                is_shift?printf(" \\ "):printf(" | ");
                break;
            case 50:
                is_shift?printf(" ` "):printf(" ~ ");
                break;  
            case 51:
                is_shift?printf(" ; "):printf(" : ");
                break;  
            case 52:
                is_shift?printf(" ' "):printf(" \" ");
                break;
            case 53:
                is_shift?printf(" ` "):printf(" ~ ");
                break; 
            case 54:
                is_shift?printf(" , "):printf(" < ");
                break;
            case 55:
                is_shift?printf(" . "):printf(" > ");
                break;
            case 56:
                is_shift?printf(" / "):printf(" ? ");
                break; 
            case 57:
                printf(" Caps Lock ");
                break; 
            case 70:
                printf(" PrintScreen ");
                break;    
            case 71:
                printf(" Scroll Lock ");
                break;
            case 72:
                printf(" Pause ");
                break;
            case 73:
                printf(" Insert ");
                break;
            case 74:
                printf(" Home ");
                break;
            case 75:
                printf(" PageUp ");
                break;
            case 76:
                printf(" Delete ");
                break;
            case 77:
                printf(" End ");
                break;
            case 78:
                printf(" PageDown ");
                break;
            case 79:
                printf(" RightArrow ");
                break;
            case 80:
                printf(" LeftArrow ");
                break;
            case 81:
                printf(" DownArrow ");
                break;
            case 82:
                printf(" UpArrow ");
                break;
            // подддержка кейпада планируется в будущем          
              
        }
    }
}

// CTRL + ESC
int keyboard_exit_condition(int8_t buf[]) {
    if(buf[1]==41) {
        return 1;
    }

    return 0;
}

int is_keyboard(libusb_device* device) {
    int rez = 0;
    struct libusb_config_descriptor* config;
    libusb_get_active_config_descriptor(device, &config);

    for(int j = 0; j < config->interface->num_altsetting; j++) {
        struct libusb_interface_descriptor interface_descriptor;
        interface_descriptor = config->interface->altsetting[j];
        rez = is_keyboard_short(interface_descriptor);
        if(rez)
            break;
    }

    libusb_free_config_descriptor(config);

    return rez;
}

int is_keyboard_short(struct libusb_interface_descriptor interface_descriptor) {
    if(interface_descriptor.bInterfaceClass == 0x03 // HID-устройство
        && interface_descriptor.bInterfaceSubClass == 0x01 // Устройство подкласса HID
        && (interface_descriptor.bInterfaceProtocol == 0x01
        || interface_descriptor.bInterfaceProtocol == 0x00)) { // Клавиатура
                return 1;
            }

    return 0;
}