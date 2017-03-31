# raspi-motor
==============================================

##Connectivity Motor & Encoder & raspberry pi3

* KIST board       	--- Raspberry pi3
* J3 - 3 : SPI DI   --- pin 21 (BCM 9) : SPI MISO
* J3 - 4 : SPI DO   --- pin 19 (BCM 10): SPI MOSI
* J3 - 5 : SPI CLK  --- pin 23 (BCM 11): SPI SCLK
* J3 - 6 : SPI ENA1 --- pin 24 (BCM 8) : SPI CS0
* J3 - 7 : SPI ENA2	--- pin 26 (BCM 7) : SPI CS1

* J4 - 4 : DAC_CLK	--- pin 23 (BCM 11): SPI SCLK
* J4 - 5 : DAC_CS 	--- pin 38 (BCM 20): SPI CS2(by overy dtb)
* J4 - 7 : DAC_DAT 	--- pin 19 (BCM 10): SPI MOSI


##How to enable spi bus on raspeberry pi3 & Execute program

(raspberrypi) $ sudo raspi-consig

>enter 5.interfacing option

>>enter 4.spi

>>>enable

(raspberrypi) $ reboot

after reboot,

(raspberrypi) $ lsmod

>check spi_dev

(raspberrypi) $ ls /dev/spi*

>check spidev0.0 and spidev0.1

(raspberrypi) $ git clone https://github.com/WookCheolShin/raspi-motor.git

(raspberrypi) $ cd raspi-motor

(raspberrypi) $ make

(raspberrypi) $ sudo ./3_motor_example.out
  
