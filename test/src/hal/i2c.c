/*
this is a mock

FILE ptr
write message to csv


*/

#include <stdio.h>
#include <stdint.h>

FILE *fp;

void i2c_init(uint8_t bus, uint8_t scl_pin, uint8_t sda_pin)
{
    fp = fopen("test_output.csv", "a+");
    fprintf(fp, "i2c,i2c_init,bus=%d,sda_pin=%d,scl_pin\n", bus, sda_pin, scl_pin);
    fclose(fp);
}
void i2c_deinit(uint8_t bus)
{
    fp = fopen("test_output.csv", "a+");
    fprintf(fp, "i2c,i2c_deinit,bus=%d\n", bus);
    fclose(fp);
}
void i2c_write(uint8_t bus, uint8_t address, uint8_t *data, uint8_t length)
{
    fp = fopen("test_output.csv", "a+");
    fprintf(fp, "i2c,i2c_write,bus=%d,address=%d,data=%d,length=%d\n", bus, address, data, length);
    fclose(fp);
}
void i2c_read(uint8_t bus, uint8_t address, uint8_t *data, uint8_t length)
{
    fp = fopen("test_output.csv", "a+");
    fprintf(fp, "i2c,i2c_read,bus=%d,address=%d,data=%d,length=%d\n", bus, address, data, length);
    fclose(fp);
}