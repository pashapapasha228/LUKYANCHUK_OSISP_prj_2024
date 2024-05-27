#include "flash.h"

libusb_device* find_flash(libusb_device** list_all, ssize_t device_count) {
    int rez;
    int indexes[device_count];
    int size_indexes = 0;

    printf("Вывод информации обо всех найденных флешках: \n");
    for(int i = 0; i < device_count; i++) {
        libusb_device* device = list_all[i];

        if(is_flash(device)) {
            indexes[size_indexes] = i;
            printf("\n%d. ", ++size_indexes);
            print_shot_device_info(device);
            printf("\n");
        }
    }

    if(size_indexes == 0) {
        printf("\nНе подключена ни одна USB-флешка!");
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

// Докинуть возможность изменения размера блока отправки
void flash_write_test(libusb_device* device, int block_size, int is_block_MB, int file_size, int is_MB) {
    char file_path[64];
    unsigned char* buffer;
    ssize_t bytes_write;

    char* mount_point;

    struct timeval tv_start, tv_stop; 
    unsigned long diff_msec;

    unsigned long long int bytes_written = 0;
    unsigned long long int bytes_block = block_size * (is_block_MB ? MB : KB);
    unsigned long long int bytes_to_write = file_size * ((is_MB) ? MB : KB);
    int amount_blocks = (int) (bytes_to_write / bytes_block);

    if(bytes_to_write < bytes_block)
        return;

    if(device == NULL)
        return;

    printf("\nРазмер файла: %d ", file_size);
    is_MB ? printf("MБ") : printf("KБ");
    printf("\nРазмер отправляемого блока для данной флешки: %d ", block_size);
    is_block_MB ? printf("MБ") : printf("KБ");
    printf("\nКоличество блоков для отправки: %d", amount_blocks);

    buffer = (unsigned char*) calloc(bytes_block, sizeof(unsigned char));
    memset(buffer, 1, bytes_block);

    mount_point = get_flash_path(device);

    snprintf(file_path, sizeof(file_path), "%s/%s", mount_point, FILENAME);
    printf("\nПуть к файлу: %s", file_path);

    // printf("Началась запись данных на флешку...");
    int file_fd = open(file_path, O_CREAT | O_WRONLY, 0644);
    if (file_fd == -1) {
        perror("\nОшибка открытия файла");
        return;
    }

    if (posix_fadvise(file_fd, 0, 0, POSIX_FADV_DONTNEED) != 0) {
        perror("Error disabling file cache");
        return;
    }

    gettimeofday(&tv_start, NULL);

    for(int i=0; i < amount_blocks; i++) {
        bytes_write = write(file_fd, buffer, bytes_block);
        bytes_written += bytes_write;

        // double proc = (double) bytes_written / bytes_to_write;
        // printf("\nПакет %d записан. Прогресс: %.2f", i+1, proc*100);
        // printf("\nПакет %d записан.", i);
    }
 
    fsync(file_fd);
    close(file_fd);

    gettimeofday(&tv_stop, NULL);

    diff_msec = (tv_stop.tv_sec - tv_start.tv_sec) * 1000L;
	diff_msec += (tv_stop.tv_usec - tv_start.tv_usec) / 1000L;

    printf("\nДанные переданы за %lu миллисекунд", diff_msec);
    printf("\nСкорость записи: %d Kбайт/с", (bytes_to_write * 1000L) / (diff_msec * KB));

    free(buffer);
    remove(file_path);
}

char* get_flash_path(libusb_device* device) {
    FILE* fp;
    char line[256];
    char mount_point[256];
    char* rez;

    fp = fopen("/proc/mounts", "r");
    if (fp == NULL) {
        perror("Не удалось открыть /proc/mounts\n");
        return NULL;
    }

    // Ищем строки, содержащие информацию о флешках
    while(fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, "/dev/sd") != NULL) {
            // Извлекаем точку монтирования
            sscanf(line, "%*s %s", mount_point);

           
            printf("\nФлешка смонтирована в: %s\n", mount_point);
            break;
        }
    }

    rez = strdup(mount_point);

    return rez;
}

// int is_it_real_path(libusb_device* device, char* path) {
//     struct libusb_device_descriptor ldesc;
//     int vendor, product;

//     int fd;
//     struct usb_device_descriptor desc;

//     // Открываем устройство /dev/sda
//     fd = open(path, O_RDONLY);
//     if (fd < 0) {
//         printf("Ошибка при открытии устройства %s\n", path);
//         return 0;
//     }

//     // Получаем информацию об USB-устройстве
//     if (ioctl(fd, USB_REQ_GET_DESCRIPTOR, &desc) < 0) {
//         perror("Ошибка при получении информации об USB-устройстве\n");
//         close(fd);
//         return 0;
//     }
//     close(fd);

//     // Выводим номер шины и адрес USB-устройства
//     printf("\nVID: %d\n", desc.idVendor);
//     printf("\nPID: %d\n", desc.idProduct);
    
//     libusb_get_device_descriptor(device, &ldesc);
//     printf("\nLVID: %d\n", ldesc.idVendor);
//     printf("\nLPID: %d\n", ldesc.idProduct);

//     return (desc.idVendor == ldesc.idVendor) && (desc.idProduct == ldesc.idProduct);
// }

void flash_read_test(libusb_device* device, int block_size, int is_block_MB, int file_size, int is_MB) {
    char file_path[64];
    unsigned char* buffer;
    ssize_t bytes_write;

    char* mount_point;

    struct timeval tv_start, tv_stop; 
    unsigned long diff_msec;

    unsigned long long int bytes_written = 0;
    unsigned long long int bytes_block = block_size * (is_block_MB ? MB : KB);
    unsigned long long int bytes_to_write = file_size * ((is_MB) ? MB : KB);
    int amount_blocks = (int) (bytes_to_write / bytes_block);

    if(bytes_to_write < bytes_block)
        return;

    if(device == NULL)
        return;

    printf("\nРазмер файла: %d ", file_size);
    is_MB ? printf("MБ") : printf("KБ");
    printf("\nРазмер получаемого блока для данной флешки: %d ", block_size);
    is_block_MB ? printf("MБ") : printf("KБ");
    printf("\nКоличество блоков для получения: %d", amount_blocks);

    buffer = (unsigned char*) calloc(bytes_block, sizeof(unsigned char));
    memset(buffer, 1, bytes_block);

    mount_point = get_flash_path(device);

    snprintf(file_path, sizeof(file_path), "%s/%s", mount_point, FILENAME);
    printf("\nПуть к файлу: %s", file_path);
    
    printf("\nНачало записи файла для дальнейшего чтения на скорость...");

    int file_fd = open(file_path, O_CREAT | O_WRONLY, 0644);
    if (file_fd == -1) {
        perror("\nОшибка открытия файла");
        return;
    }

    for(int i=0; i < amount_blocks; i++) {
        bytes_write = write(file_fd, buffer, bytes_block);
        bytes_written += bytes_write;

        // double proc = (double) bytes_written / bytes_to_write;
        // printf("\nПакет %d записан. Прогресс: %.2f", i+1, proc*100);
        // printf("\nПакет %d записан.", i);
    }
 
    fsync(file_fd);
    close(file_fd);

    int file_fd2 = open(file_path, O_RDONLY);
    if (file_fd2 == -1) {
        perror("\nОшибка открытия файла");
        return;
    }

    if (posix_fadvise(file_fd, 0, 0, POSIX_FADV_DONTNEED) != 0) {
        perror("Error disabling file cache");
        return;
    }

    printf("\nЗапись завершена. Началось чтение из файла...");
    bytes_write = 0;

    gettimeofday(&tv_start, NULL);

    for(int i=0; i < amount_blocks; i++) {
        bytes_write = read(file_fd, buffer, bytes_block);
        bytes_written += bytes_write;

        // double proc = (double) bytes_written / bytes_to_write;
        // printf("\nПакет %d записан. Прогресс: %.2f", i+1, proc*100);
        // printf("\nПакет %d записан.", i);
    }

    gettimeofday(&tv_stop, NULL);

    diff_msec = (tv_stop.tv_sec - tv_start.tv_sec) * 1000L;
	diff_msec += (tv_stop.tv_usec - tv_start.tv_usec) / 1000L;

    printf("\nДанные получены за %lu миллисекунд", diff_msec);
    printf("\nСкорость чтения: %d Kбайт/с", (bytes_to_write * 1000L) / (diff_msec * KB));

    free(buffer);
    remove(file_path);
}

int is_flash(libusb_device* device) {
    int rez = 0;
    struct libusb_config_descriptor* config;

    if(device == NULL)
        return 0;

    libusb_get_active_config_descriptor(device, &config);

    for(int j = 0; j < config->interface->num_altsetting; j++) {
        struct libusb_interface_descriptor interface_descriptor;
        interface_descriptor = config->interface->altsetting[j];
        rez = is_flash_short(interface_descriptor);
        if(rez)
            break;
    }

    libusb_free_config_descriptor(config);

    return rez;
}

int is_flash_short(struct libusb_interface_descriptor interface_descriptor) {
    if(interface_descriptor.bInterfaceClass == 0x08 // mass storage class
        && (interface_descriptor.bInterfaceSubClass == 0x06 // SCSI
            || interface_descriptor.bInterfaceSubClass == 0x04) // UFI
        && interface_descriptor.bInterfaceProtocol == 0x50) { // Bulk-Only Transport
                return 1;
            }

    return 0;
}