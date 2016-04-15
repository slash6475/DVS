# DVS: Electronic Devil Stick
This project is an introduction to how to build quickly a native IP-based connected object on tiny platform. 
DVS is a simple electronic board composed of a 8 bits- microcontroller with Zigbee tranceiver, an accelerometer and some leds on child boards with an external button.
The software is implemented on Contiki OS, this micro-kernel dedicated to the Internet Of Thing allows 6LoWPAN networking (uIP stack with RPL for mesh network routing),
file system management to store data on flash or eeprom memories, shell interface over serial line, embedded webserver and more.

A simple Javascript page allows to design programs of RGB led sequences and to load them on the DVS platform over HTTP.
The programs are stored into the DVS file system and play on the leds according to the selected program by the external button.
In addition, the accelerometer and wireless communication can be used to control the leds. 
The wireless communication based on 6LoWPAN and HTTP can be operated by any other DVS platforms or computers.
Based on these features, a lot of scenarios and applications can be imagined. 

# Software
Contiki OS has a software architecture which facilitates the encapsulation of drivers and applications into independent containers.
Hence each component of DVS software is independent which facilitates implementation, modularity and code re-use.
Thanks to the shell over the serial line, it is possible to test the drivers of RGB leds and accelerometers, to manage the Webserver (start/stop) and the file system formating such as presented in following video.

[![ScreenShot](http://duhart-clement.fr/imgs/dvs.png)](https://youtu.be/-myyth4NgPw)

## Shell Interface
Open the serial line over USB interface:
```
minicom -D /dev/ttyUSB0 -b 19200
```
At booting time, different information on Contiki OS and DVS software configurations are provided such as networking, boot sequence, etc.
The command help provides the list of available commands:
```
neopixel test
neopixel ID_led R G B
mpu6050 start | stop
...
help
ps
kill
killall

```
## Programming
The software is fully implemented in C language and cross-platforms (MSP, AVR, ARM, X86, PIC, etc). The compilation must be executed in main project folder.
```
cd contiki-2.6/examples/dvs_main/
```
### Compilation
```
make all TARGET=avr-atmega128rfa1
avr-size --mcu=atmega128rfa1 dvs-main.avr-atmega128rfa1  -C
```

### Preparation
It is preferred to remove fuse and eeprom section to avoid to break fuse configuration and to erase programs stored in memories.
```
avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature dvs-main.avr-atmega128rfa1 dvs-main.hex
```

### Loading
Example to load the program over ISP connector with a JTAG2 device.
```
sudo avrdude -p atmega128rfa1 -c jtag2isp -P usb -Uflash:w:dvs-main.hex
```
### 

## Arduino encapsulation
The DVS uses the accelerometer MPU6050 and the RGB leds Neopixel which have their librairy implemented in Arduino C++. 
Intead of implementing new librairies for Contiki OS, it is possible to link them dynamically at compilation step. 
Build a dvs.ino file with Arduino style and compile it, then remove the main.o and encapsulate it in a dynamic library.
More details in (contiki-2.6/apps/dvs)
```
avr-ar -cvq dvs_arduino.a *.o
```

In DVS Makefile of Contiki:
```
PROJECT_LIBRARIES=../../apps/dvs/libs/dvs_arduino.a
```

# Web Apps
The Web Interface is a simple javascript page which allows to generate and sequence chunk such as presented below. 
The number of program is not limited, however the sum of all sequences of all programs should be lower than file system size (4Kb).
Programs can be deleted individually, a long pressure on external button (> 20s) formats the file system.
```
OPERATION:TIME:RR1GG1BB1RR2GG2BB2 ...
add:0:001000000000000000000000ff0000ff0000000000000000000000000000000000000000ff0000ff0000000000000000000000
```

![alt tag](http://duhart-clement.fr/imgs/dvs-web.png)


# Hardware
![alt tag](http://duhart-clement.fr/imgs/dvs-web.png)
![alt tag](http://duhart-clement.fr/imgs/dvs-web.png)
![alt tag](http://duhart-clement.fr/imgs/dvs-web.png)
