# Illuminator
This project is called PCB Illuminator. It is firmware for count down timer. 
The timer turns on and off the illumination lamp relay after a set time. There are 4 presets to save the most frequently used times.
## Hardware
* MCU: ATMEGA328P
* Encoder with button
* I2C OLED display 128x64
* LED
* Passive buzzer
* Other (relay, transistors BC337, resistors, capacitors, crystal, etc...)
* PCB: 1.0.2 rev.12
## Software
The firmware is developed for the ATMEGA328P microcontroller, and you can also run it on the Arduino UNO with ATMEGA328P.
It is divided into separate .ino sketch and other .h and .cpp files. In the /library directory there are libraries that need to be expanded to the "/libraries" subfolder in the Arduino folder. Before uploading, select the Arduino UNO board type in the Arduino IDE.
## Author
(c) mateusko O.A.M.D.G, see LICENSE.txt
