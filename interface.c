#include "interface.h"

void menu(libusb_device** list, ssize_t device_count) {
    int rez;

    while(1) {
        rez = -1;

        //printf("\nВведите режим тестирования:");
        printf("\n1. Вывести на экран все подключенные USB-устройства.");
        printf("\n2. Переход к работе с флешками.");
        printf("\n3. Переход к работе с мышами.");
        printf("\n4. Переход к работе с клавиатурами.");
        printf("\n5. Выход из программы.");

        while(rez < 1 || rez > 5) {
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
                flash_menu(find_flash(list, device_count));
                break;
            case 3:
                mouse_test(find_mouse(list, device_count));
                break;
            case 4:
                keyboard_test(find_keyboard(list, device_count));
                break;
            case 5:
                return;
                break;
        }

        printf("\n");
    }
}

void flash_menu(libusb_device* device) {
    int rez;
    int is_MB;
    int is_block_MB;
    int size;
    int size_block;

    if(device == NULL)
        return;
    
    while(1) {
        rez = -1;
        is_MB = -1;
        is_block_MB = -1;
        size = -1;
        size_block = -1;

        printf("\n1. Тест скорости записи");
        printf("\n2. Тест скорости чтения");
        printf("\n3. Выход");

        while(rez < 1 || rez > 3) {
            rewind(stdin);
            printf("\nВаш выбор: ");
            scanf("%d", &rez);
        }

        if(rez == 3)
            return;

        printf("\nВвод единиц измерения файла");
        printf("\n1. КБ");
        printf("\n2. МБ");

        while(is_MB < 1 || is_MB > 2) {
            rewind(stdin);
            printf("\nВаш выбор: ");
            scanf("%d", &is_MB);
        }
        is_MB--;

        printf("\nВведите размер файла (");
        is_MB? printf("MБ"):printf("KБ");
        printf("): ");

        while(size < 1) {
            rewind(stdin);
            printf("\n: ");
            scanf("%d", &size);
        }

        printf("\nВвод единиц измерения блока");
        printf("\n1. KБ");
        printf("\n2. MБ");

        while(is_block_MB < 1 || is_block_MB > 2) {
            rewind(stdin);
            printf("\nВаш выбор: ");
            scanf("%d", &is_block_MB);
        }
        is_block_MB--;

        printf("\nВведите размер блока (");
        is_block_MB? printf("MБ"):printf("KБ");
        printf("): ");

        while(size_block < 1) {
            rewind(stdin);
            printf("\n: ");
            scanf("%d", &size_block);
        }

        switch(rez) {
            case 1:
                flash_write_test(device, size_block, is_block_MB, size, is_MB);
                break;
            case 2:
                flash_read_test(device, size_block, is_block_MB, size, is_MB);
                break;
            case 3:
                return;
                break;
        }
    }
}