This an I2C SRAM programming of Crosslink using Raspebrry pi 5. 

This example uses lgpiod library so makesure to install it: sudo apt-get install gpiod libgpiod-dev.

The instructions used for programming for this example is taken from FPGA-TN-02014-1.7 ([CrossLink Programming and Configuration User Guide](https://www.latticesemi.com/view_document?document_id=51655)). See instructions below from the document. 
![image](https://github.com/user-attachments/assets/ec2d95c2-9c9e-4000-b2ba-651be49ea8ed)

Source files:
main.c- contains the main programming procedure

data.c- contains the bitstream data to be sent, you can update this part to send the bitstream of your Crosslink Device.

rbpi-i2c.c- contains the functions used for I2C transactions.

Makefile- compiles the project

You can modify the g_int and g_pucDataArray with your generated hexadedimal bitstream. With this I used the default project from Crosslink Masterlink Board. 
![image](https://github.com/user-attachments/assets/c415ffa9-2ae6-4c53-ad1c-6164d2c0192f)

Main.c follows  the instruction from FPGA-TN-02014-1.7 
![image](https://github.com/user-attachments/assets/e3d7b5fd-7b60-464b-93db-fd7112a193ad)

Sample Waveform transaction:

![image](https://github.com/user-attachments/assets/40f27825-bb74-41f6-a47d-d9714e7b5361)


![image](https://github.com/user-attachments/assets/6727cef4-7f22-4781-97ab-06c76168859b)



