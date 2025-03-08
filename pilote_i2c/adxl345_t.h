#define DATAX0 0x32 // X-axis low byte
#define DATAX1 0x33 // X-axis high byte
#define DATAY0 0x34 // Y-axis low byte
#define DATAY1 0x35 // Y-axis high byte
#define DATAZ0 0x36 // Z-axis low byte
#define DATAZ1 0x37 // Z-axis high byte

#define ADXL345_MAGIC 'A' // Unique magic number for the driver

// IOCTL commands
#define ADXL345_SELECT_X _IO(ADXL345_MAGIC, 0) // Select X-axis
#define ADXL345_SELECT_Y _IO(ADXL345_MAGIC, 1) // Select Y-axis
#define ADXL345_SELECT_Z _IO(ADXL345_MAGIC, 2) // Select Z-axis