#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>

#define ADXL345_MAGIC 'A'
#define ADXL345_SELECT_X _IO(ADXL345_MAGIC, 0)
#define ADXL345_SELECT_Y _IO(ADXL345_MAGIC, 1)
#define ADXL345_SELECT_Z _IO(ADXL345_MAGIC, 2)


int main() {
    int fd;
    char buffer[2];

    // Open the device file
    fd = open("/dev/adxl345-0", O_RDONLY);//  O_RDONLY – Open for reading only.
    if (fd < 0) {
        perror("Failed to open device");
        return 1;
    }

    // Select the X-axis
    if (ioctl(fd, ADXL345_SELECT_X) < 0) {
        perror("Failed to select X-axis");
        close(fd);
        return 1;
    }


    // Read data from the device
    /*[Explanation]
    read (parameters easy , fd:file descriptor represents the file to read from )
    On success → Returns the number of bytes actually read (can be 0 at EOF).
    On failure → Returns -1 and sets errno.
    */
    if (read(fd, buffer, sizeof(buffer)) < 0) {
        perror("Failed to read from device");
        close(fd);
        return 1;
    }

    // Print the X-axis data
    printf("X-axis data: 0x%02X%02X\n", (unsigned char)buffer[0], (unsigned char)buffer[1]);

    // Close the device file
    close(fd);
    return 0;
}