#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>  // Using libi2c-dev for SMBus operations
#include <string.h>
#include "rbpi-i2c.h"
#include "data.h"
#include <gpiod.h>
struct gpiod_line *crst;
//char a='A';// Dummy character needed for lbgpiod functions
int main() {
    
    unsigned char device_ID[4] = { 0xE0, 0x00, 0x00, 0x00  }; // Command bytes to send (E0 00 00 00)
    unsigned char isc_enable[4] = { 0xC6, 0x00, 0x00, 0x00  }; // Command bytes to send (C6 00 00 00)
    unsigned char isc_disable[4] = { 0x26, 0x00, 0x00, 0x00  }; // Command bytes to send (26 00 00 00)
    unsigned char sr_read[4] = { 0x3C, 0x00, 0x00, 0x00  }; // Command bytes to send (3C 00 00 00)
    unsigned char set_addr[4] = { 0x46, 0x00, 0x00, 0x00  }; // Command bytes to send (46 00 00 00)
    unsigned char sram_erase[4] = { 0x0E, 0x00, 0x00, 0x00  }; // Command bytes to send (0E 00 00 00)
    unsigned char bitstream_burst[4] = { 0x7A, 0x00, 0x00, 0x00  }; // Command bytes to send (7A 00 00 00)

    unsigned char activation_key[4]={ 0xA4,0xC6,0xF4,0x8A};
    unsigned char read_buf2[2]; // Buffer to store read data
    unsigned char read_buf[4]; // Buffer to store read data
    unsigned char global_write_buf[4];
    int global_length=4;


    i2c_init(&crst);
    gpiod_line_set_value(crst, 1);
    usleep(1000);
    
 //Initial Reset and sending of activation key   
    gpiod_line_set_value(crst, 0);
    usleep(1000);
    i2c_write_byte(4,activation_key);
    usleep(1000);
    gpiod_line_set_value(crst, 1);
    
    usleep(10000);
    
    
   printf("Activation Key Sent:\n");
   for (int i = 0; i < 3; i++) {
       printf("0x%02X ",activation_key[i]);
       printf("\n");
   }
//i2c read device ID Check
    i2c_write_and_read(device_ID,global_length,read_buf,4); //Read Device ID
        printf("Device ID Read: ");
    for (int i = 0; i < 4; i++) {
        printf("0x%02X ", read_buf[i]);
    }
    printf("\n");
    

//isc_enable: Enters Programming Mode
    printf("Device enters programming mode.\n");
    i2c_write_byte(4,isc_enable);
    
    usleep(1000);
//SRAM Erase    
    i2c_write_byte(4,sram_erase);
    sleep(5);

//Status Register Read
    i2c_write_and_read(sr_read,4,read_buf,4); //Read Status Register
        printf("Status Register Read: ");
    for (int i = 0; i < 4; i++) {
        printf("0x%02X ", read_buf[i]);
    }
    printf("\n");
    
    
// Set address

    i2c_write_byte(4,set_addr);
    
// Programming using bitstream burst
    printf("Uploading bitstream using LSC_BITSTREAM_BURST\n");
    i2c_write_long(bitstream_burst,4,g_pucDataArray,163234);
    
// Status Register Read
    i2c_write_and_read(sr_read,4,read_buf,4); //Read Status Register
        printf("Status Register Read: ");
    for (int i = 0; i < 4; i++) {
        printf("0x%02X ", read_buf[i]);
    }
    printf("\n");

//Exits Programming Mode
    i2c_write_byte(4,isc_disable);
    
    printf("Programming Done!\n");
    
    



    // Close the I2C bus
    close(fd);
    return EXIT_SUCCESS;
}
