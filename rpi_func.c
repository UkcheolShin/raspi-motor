/*
*********************************************************************************************************
*                                      RASPBERRY_PI_FUNC_C
*********************************************************************************************************
*/
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <stdint.h> 
#include <sys/mman.h>
#include <fcntl.h> 
#include <sys/ioctl.h>
#include <linux/spi/spidev.h> 
#include "rpi_func.h"

/*
*********************************************************************************************************
*                                      RASPBERRY PI GPIO FUNC
*********************************************************************************************************
*/

/* 
* gpio mapping
* int motor_hw_init(void)
* 입력 값 : 없음
* 반환 값 : 성공 0 / 실패 -1
* 설명 : 라즈베리파이의 메모리 디바이스 파일을 열고 그에 대한 메모리 매핑을 하여 gpio에 접근 가능토록 함 
*/
int rpi_gpio_setup(void)
{
    int mem_fd; 
    int ret = 0;

	//BCM2835 메모리 디바이스 파일 오픈		
	if ((mem_fd = open ("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC)) < 0){
		printf("mem open error\n");						   
		return -1;
	}

    //GPIO_BASE부터 BLOCK_SIZE만큼의 동적 메모리 할당하고 시작 주소 반환.
    if((iom_gpio = (unsigned int *)mmap(0, BLOCK_SIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mem_fd, GPIO_BASE)) == NULL){
		printf("mmap error\n");
		return -1;
    }
    
    return 0;
}

/* 
* 핀의 입/출력 설정 함수
* int rpi_gpio_direction(unsigned int pin_num, unsigned int mode)
* 입력 값 : pin_num ==> 제어하기 위한 핀 번호(BCM 기준)
*         mode ==> INPUT / OUTPUT 
* 반환 값 : 성공 0 / 실패 -1
* 설명 : 입력받은 GPIO핀(pin_num)에 대하여 mode에 맞게 입력,출력 설정 
*/
int rpi_gpio_direction(unsigned int pin_num, unsigned int mode)
{
    /* 총 40개의 GPIO를 다룰 수 있음 */
    if(pin_num > 40) return -1;
    /* 하나의 GPIO에 대한 기능은 3개의 bit로 표현됨 */
    if(mode > 7) return -1;
 
    switch(mode){
        INP_GPIO(pin_num);
        case INPUT :
            break;
        case OUTPUT :
            OUT_GPIO(pin_num);
            break;
    }
    return 0;
}

/* 
* 핀의 함수 설정 함수
* int rpi_gpio_alt_func(unsigned int pin_num, unsigned int mode)
* 입력 값 : pin_num ==> 제어하기 위한 핀 번호(BCM 기준)
*         mode ==> INPUT / OUTPUT 
* 반환 값 : 성공 0 / 실패 -1
* 설명 : 입력받은 GPIO핀(pin_num)에 대하여 mode에 맞게 ,ALT모드 설정 
*/
int rpi_gpio_alt_func(unsigned int pin_num, unsigned int mode)
{
    /* 총 40개의 GPIO를 다룰 수 있음 */
    if(pin_num > 40) return -1;
    /* 하나의 GPIO에 대한 기능은 3개의 bit로 표현됨 */
    if(mode > 7) return -1;
 
    SET_GPIO_ALT(pin_num,mode);
 
    return 0;
}

/* 
* 출력 모드에서 출력 상태를 설정
* int rpi_gpio_write(unsigned int pin_num, unsigned int status)
* 입력 값 : pin_num ==> 제어하기 위한 핀 번호(BCM 기준)
*         status ==> ON / OFF 
* 반환 값 : 성공 0 / 실패 -1
* 설명 : 입력받은 GPIO핀(pin_num)의 출력 상태를 status에 따라 ON : 1 , OFF : 0으로 설정 
*/
int rpi_gpio_write(unsigned int pin_num, unsigned int status)
{
    if(pin_num > 40)                    return -1;
    if(status != OFF && status != ON)   return -1;

     /* status 값에 따라 set과 clear 중 하나를 선택 할 수 있음 */
    if(status == OFF) GPIO_CLEAR(pin_num); 
    else if(status == ON) GPIO_SET(pin_num);
    return 0;
}

/* 
* 입력 모드에서의 입력된 값 읽음
* int rpi_gpio_read(unsigned int pin_num)
* 입력 값 : pin_num ==> 제어하기 위한 핀 번호(BCM 기준)
* 반환 값 : 현재 핀의 상태
* 설명 : 입력받은 GPIO핀(pin_num)의 입력 상태를 반환.
*/
int rpi_gpio_read(unsigned int pin_num)
{
    if(pin_num > 40)                    return -1;
    return GPIO_READ(pin_num);
}


/*
*********************************************************************************************************
*                                      RASPBERRY PI SPI FUNC
* SPI DATA WRITE
* SPI 통신은 spi_ioc_transfer 구조체라는 일관된 인터페이스를 
* 통해 SPI Master/Slave 간 통신을 한다.
* Data를 통해 지정된 채널의 값을 읽어오고 그대로 이용하여 rx_buf로서 이용한다.
* SPI_IOC_MESSAGE(1) Command는 데이터를 써넣는 매크로인 _IOW로 
* 치환된다. 자세한 내용은 Linux/include/uapi/linux/spi/spidev.h을 참고
* ioctl 함수를 통해 쓰기 명령과 버퍼의 주소를 드라이버로 보낸다.
*********************************************************************************************************
*/

/* 
* spi 통신을 위한 설정
* int rpi_spi_setup(int channel, int mode, int bits_per_word, int speed, int delay)
* 입력 값 : channel ==> 설정하고자하는 spi 채널. 현재 0~4까지 존재.
          mode ==> spi 모드 이하 모든 입력 값들은 <linux/spi/spidev.h> 참조
          bits_per_word ==> 1워드당 비트수. 
          speed ==> spi 통신 속도 
          delay ==> 딜레이
* 반환 값 : 성공 0 / 실패 -1
* 설명 : spi 통신 설정
*/
int rpi_spi_setup(int channel, int mode, int bits_per_word, int speed, int delay)
{
	char  fName[128];
	int   spi_channel = channel & 0x3;
	int   spi_mode    = mode & 0x3;  
	int   spi_bpw     = bits_per_word; 
	int   spi_delay   = delay ; 
	int   spi_speed   = speed;   
	int   ret;

	//spidev 파일 오픈
    sprintf (fName, "/dev/spidev0.%d", spi_channel) ;
	if((spi_fds[spi_channel] = open (fName , O_RDWR)) < 0 ){
   	   printf("spi open error\n"); 
   	   return -1;
	}

    //SPI 환경 설정 
	if((ret = ioctl (spi_fds[spi_channel] , SPI_IOC_WR_MODE, &spi_mode))<0){
		printf("spi ioctl error\n");
		return -1; 
	}
	if((ret = ioctl (spi_fds[spi_channel] , SPI_IOC_WR_BITS_PER_WORD , &spi_bpw))<0){
		printf("spi ioctl error\n");
		return -1; 
	}    
	if((ret = ioctl (spi_fds[spi_channel] , SPI_IOC_WR_MAX_SPEED_HZ, &spi_speed))<0){
		printf("spi ioctl error\n");
		return -1; 
	}

#ifdef DEBUG
    //SPI 설정된 값 읽어오기 
    ioctl(spi_fds[spi_channel], SPI_IOC_RD_MODE, &spi_mode);
    ioctl(spi_fds[spi_channel], SPI_IOC_RD_BITS_PER_WORD, &spi_bpw);
    ioctl(spi_fds[spi_channel], SPI_IOC_RD_MAX_SPEED_HZ, &spi_speed);
    
    printf("spi DAC \n");
    printf("spi mode: %d\n", spi_mode);
    printf("bits per word: %d\n", spi_bpw);
    printf("max speed: %d Hz (%d KHz)\n", spi_speed, spi_speed/1000);
#endif

    //전역변수에 값 저장.
   	spi_bpws[spi_channel]     	= bits_per_word; 
	spi_delays[spi_channel]   	= delay ; 
	spi_speeds[spi_channel]   	= speed;   

    return 0;
}

/* 
* spi 파일 닫기
* void rpi_spi_close(void)
* 입력 값 : 없음
* 반환 값 : 없음
*/
void rpi_spi_close(void)
{
    close(spi_fds[0]);
    close(spi_fds[1]);
    close(spi_fds[2]);
}

/* 
* spi 데이터 읽기/쓰기
* int rpi_spi_data_rw(int channel, unsigned char *data, int len) 
* 입력 값 : channel ==> 쓰고 읽고자 하는 spi 채널. 현재 0~4까지 존재.
          data ==> 입력하고자 하는 데이터
          len ==> 데이터의 길이(bpw 기준)
* 반환 값 : 쓰고 읽은 데이터의 길이(bpw 기준)
* 설명 : spi 데이터 읽기/쓰기
*/
int rpi_spi_data_rw(int channel, unsigned char *data, int len) 
{
    struct spi_ioc_transfer spi = {0,}; 
    
    channel             &= 0x3 ; 
    spi.tx_buf          = (unsigned long)data ; 
    spi.rx_buf          = (unsigned long)data ;      
    spi.len             = len ;  
    spi.delay_usecs     = spi_delays[channel]; 
    spi.speed_hz        = spi_speeds[channel] ; 
    spi.bits_per_word   = spi_bpws[channel] ; 
    
    return ioctl (spi_fds[channel], SPI_IOC_MESSAGE(1), &spi) ; 
}
