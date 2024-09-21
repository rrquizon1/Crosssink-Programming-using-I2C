#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <i2c/smbus.h>  // Using libi2c-dev for SMBus operations
#include <string.h>
#include "rbpi-i2c.h"
#include <gpiod.h>

#define I2C_BUS "/dev/i2c-1"  // Replace with your I2C bus path
#define DEVICE_ADDR 0x40      // Replace with your I2C device address

int fd;

char a='A';// Dummy character needed for lbgpiod functions
int pull_up=0; //to enable pull up. It is always enabled by default

/* configure_output() sets the pin to output used when sending data. Since this is I2C master SCL is always output. SDA changes when writing or reading.
 * */
int configure_output(struct gpiod_line *line, const char *consumer, int value)
{
    if (gpiod_line_request_output(line, consumer, value) < 0)
    {
        perror("Request line as output failed");
        return -1;
    }

    return 0;
}


int i2c_init(struct gpiod_line **cs){
   struct gpiod_chip *chip;
   int offset_cs = 21; // Replace with your GPIO pin number for CS
   chip = gpiod_chip_open("/dev/gpiochip4"); // Replace 4 with the appropriate chip number
   
    *cs= gpiod_chip_get_line(chip, offset_cs);

    if (!chip) {
        perror("Open chip failed");
        return 1;
    }

       if (!*cs) {
        perror("Get line failed");
        gpiod_chip_close(chip);
        return 1;
    }
    
   configure_output(*cs, &a, 0);
   // usleep(1000);
  // configure_output(*cs, &a, 1);
   
   fd = open(I2C_BUS, O_RDWR);
        // Open the I2C bus

    if (fd < 0) {
        perror("Failed to open the I2C bus");
        return EXIT_FAILURE;
    }
    
    

    // Set the I2C slave address
    if (ioctl(fd, I2C_SLAVE, DEVICE_ADDR) < 0) {
        perror("Failed to acquire bus access and/or talk to slave");
        close(fd);
        return EXIT_FAILURE;
    }

   if (ioctl(fd, I2C_TIMEOUT, 10000) < 0) {
        perror("Failed to set I2C timeout");
        close(fd);
        return 1;
    }
  
    
}


int i2c_write_byte(int length, unsigned char *data_bytes){
    struct i2c_msg msgs[1];
    int num_msgs = 1;
    
    // First message: Write command bytes (without stop)
    msgs[0].addr = DEVICE_ADDR;
    msgs[0].flags = 0;  // No stop
    msgs[0].len = length;
    msgs[0].buf = data_bytes;
    
    struct i2c_rdwr_ioctl_data ioctl_data;
    ioctl_data.msgs = msgs;
    ioctl_data.nmsgs = num_msgs;

    // Perform combined write and read operation
    if (ioctl(fd, I2C_RDWR, &ioctl_data) < 0) {
        perror("Failed to perform write I2C transaction");
       // close(fd);
        return EXIT_FAILURE;
    }
    
} 

int i2c_read_byte(int length, unsigned char *a_ByteRead, int send_ack){
    struct i2c_msg msgs[1];
    int num_msgs = 1;
    
    // Second message: Read 4 bytes of data
    msgs[0].addr = DEVICE_ADDR;
    msgs[0].flags = I2C_M_RD;  // Read flag
    msgs[0].len = length;
    msgs[0].buf = a_ByteRead;


    struct i2c_rdwr_ioctl_data ioctl_data;
    ioctl_data.msgs = msgs;
    ioctl_data.nmsgs = num_msgs;

    // Perform combined write and read operation
    if (ioctl(fd, I2C_RDWR, &ioctl_data) < 0) {
        perror("Failed to perform read I2C transaction");
       // close(fd);
        return EXIT_FAILURE;
    }
    
    
}
int i2c_write_and_read(unsigned char *write_data, int write_len, unsigned char *read_data, int read_len) {
    struct i2c_msg msgs[2];
    struct i2c_rdwr_ioctl_data ioctl_data;

    // Prepare the write message
    msgs[0].addr = DEVICE_ADDR;
    msgs[0].flags = 0;  // Write
    msgs[0].len = write_len;
    msgs[0].buf = write_data;

    // Prepare the read message
    msgs[1].addr = DEVICE_ADDR;
    msgs[1].flags = I2C_M_RD;  // Read
    msgs[1].len = read_len;
    msgs[1].buf = read_data;

    // Prepare the ioctl data structure
    ioctl_data.msgs = msgs;
    ioctl_data.nmsgs = 2;

    // Perform the combined write/read operation
    if (ioctl(fd, I2C_RDWR, &ioctl_data) < 0) {
        perror("Failed to perform combined I2C transaction");
        return -1;
    }

    return 0;}


int i2c_write_long(unsigned char *write_command, int writecomm_len, unsigned char *write_data, int writedata_len) {
  struct i2c_rdwr_ioctl_data ioctl_data;  

if (writedata_len<=32 ){
    struct i2c_msg msgs[2];
    
    unsigned char *buffers[2] = {write_command, write_data};
    int lengths[2] = {writecomm_len, writedata_len};
    // Prepare the I2C messages using a loop
    for (int i = 0; i < 2; i++) {
        msgs[i].addr = DEVICE_ADDR;
        msgs[i].flags = 0;  // Write
        msgs[i].len = lengths[i];
        msgs[i].buf = buffers[i];
    }

     ioctl_data.msgs = msgs;
    ioctl_data.nmsgs = 2;
    
}

else {
    int chunk_size = 4096;  // Define the chunk size in bytes
    int num_bytes_command = writecomm_len;  // Length in bytes for command data
    int num_bytes_data = writedata_len;  // Length in bytes for data
    int total_len_bytes = num_bytes_command + num_bytes_data;  // Total length in bytes
    int num_chunks = (total_len_bytes + chunk_size - 1) / chunk_size;  // Calculate number of chunks

    struct i2c_msg msgs[num_chunks];  // Array to hold I2C messages

    // Combine write_command and write_data into a single buffer  
    unsigned char combined_buffer[total_len_bytes];
    memcpy(combined_buffer, write_command, num_bytes_command);  // Copy command data
    memcpy(combined_buffer + num_bytes_command, write_data, num_bytes_data);  // Copy data

    // Prepare the I2C messages using a loop
    for (int i = 0; i < num_chunks; i++) {
        msgs[i].addr = DEVICE_ADDR;
        msgs[i].flags = 0;  // Write

        // Calculate the size of the current chunk
        int current_chunk_size = (i == num_chunks - 1) ? (total_len_bytes % chunk_size) : chunk_size;
        if (current_chunk_size == 0) current_chunk_size = chunk_size;  // Handle case where the last chunk is exactly chunk_size bytes

        msgs[i].len = current_chunk_size;
        msgs[i].buf = &combined_buffer[i * chunk_size];  // Point to the start of the current chunk in the buffer
        
        // Print the contents of msgs[i].buf

    }

    // Prepare the ioctl data structure
    ioctl_data.msgs = msgs;
    ioctl_data.nmsgs = num_chunks;
   // printf("Num of Chunks: %d ",num_chunks);
    
}

    // Perform the combined write operation
    if (ioctl(fd, I2C_RDWR, &ioctl_data) < 0) {
        perror("Failed to perform combined I2C transaction");
        return -1;
    }

    return 0;
}

