#define I2C_speed       100000  //100 kbps
void I2C_init();
void I2C_wait();
void I2C_start();
void I2C_restart();
void I2C_stop();
void I2C_write(unsigned char a);
void I2C_Send_ACK(void);
void I2C_Send_NACK(void);
char I2C_read(unsigned char ACK);

