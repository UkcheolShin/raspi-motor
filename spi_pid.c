/*
* (raspberry pi3) $ sudo raspi-config
*   > enter 5. Interfacing Options
*   > enable p4 SPI
*   > reboot
* (raspberry pi3) $ ls /dev/spi*
*   > check spidev0.0 or spidev0.1
* (raspberry pi3) $ wget https://raw.githubusercontent.com/opennetworklinux/linux-3.8.13/master/Documentation/spi/spidev_test.c
* (raspberry pi3)
*/
#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <stdint.h> 
#include <signal.h>
#include "rpi_func.h"
#include "motor_func.h"

static void pabort(const char *s)
{
    perror(s);
    abort();
}

void signalHandler(int signo)
{
    rpi_spi_close();
    pabort("SIGINT");
}

int main(void) { 
    int ret,i=0,dac=0;

    //SIGINT 시그널을 받으면 signalhandler를 실행하도록 설정
    signal(SIGINT,signalHandler);
	if((ret = rpi_gpio_setup()) < 0)
        pabort("<1>Hardware init error");
    else 
        printf("<1>Hardware init done..\n");

    //하드웨어 초기화(GPIO 설정)
    if ((ret = motor_hw_init()) < 0)
		pabort("<2>Motor init error");
    else 
        printf("<2>Motor init done..\n");

    //SPI 초기화
    if((ret = rpi_spi_setup(SPI_DAC_CHANNEL,SPI_MODE,SPI_BPW,SPI_DAC_SPEED,SPI_DELAY))<0)
        pabort("<3>DAC setup error");
    else
        printf("<3>DAC setup done..\n");

    if((ret = rpi_spi_setup(SPI_ENC_L_CHANNEL,SPI_MODE,SPI_BPW,SPI_ENC_SPEED,SPI_DELAY))<0)
        pabort("<4>Left Encoder setup error");
    else
        printf("<4>Left Encoder setup done..\n");

    if((ret = rpi_spi_setup(SPI_ENC_R_CHANNEL,SPI_MODE,SPI_BPW,SPI_ENC_SPEED,SPI_DELAY))<0)
        pabort("<5>Right Encoder setup error");
    else
        printf("<5>Right Encoder setup done..\n");
#if 0
    while(1){
        printf("input value \n");
        scanf("%x",&dac);
        
        set_direction(LEFT_WHEEL,FORWARD);
        writeDAC(DAC_ADDR_ALL,DAC_CMD_WRUP,dac);
    }
#endif

#if 1
//PI 제어 테스트
     set_direction(LEFT_WHEEL,FORWARD);
    while(i < 2000){
    
        // 속도 제어
        //speed_control(20,LEFT_WHEEL,FORWARD);
        // 각도 제어
        pos_control(360,LEFT_WHEEL,FORWARD);

    //    encoder_read(LEFT_WHEEL);
        i++;
        usleep(1000);
    }
#endif
// 엔코더 읽어오기.
#if 0
    set_direction(LEFT_WHEEL,FORWARD);
    writeDAC(DAC_ADDR_ALL,DAC_CMD_WRUP,0x200);
    while(i < 2000){
      // 프린트 확인
        pos_speed_printf(LEFT_WHEEL,FORWARD);
        i++;
        usleep(1000);
    }
    writeDAC(DAC_ADDR_ALL,DAC_CMD_WRUP,0x10);
#endif
    rpi_spi_close();
	return 0;
}	
