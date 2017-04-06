/*
*********************************************************************************************************
*                                              RASPBERRY_PI_FUNCTION.H
*********************************************************************************************************
*/
#ifndef __RPI_FUNC_H__
#define __RPI_FUNC_H__

/*디버그 옵션*/
#define DEBUG

/*
*********************************************************************************************************
*                                              RASPBERRY PI3 DEFINE
*********************************************************************************************************
*/

/* 
* I/O 가상 주소 
* BCM2835 데이터 시트 참조 
*/
#ifndef BCM2708_PERI_BASE
    #define BCM2708_PERI_BASE   0x3F000000
    #define GPIO_BASE       (BCM2708_PERI_BASE + 0x200000) /* GPIO controller */
#endif

#define CLOCK_BASE      	(BCM2708_PERI_BASE + 0x101000) /* CLOCK controller */
#define PWM_BASE        	(BCM2708_PERI_BASE + 0x20C000) /* PWM controller */

//메모리 매핑을 위한 전역 변수
static volatile unsigned int *iom_gpio;
#define BLOCK_SIZE      	(4*1024)

/* 
* GPIO setup macros.
* OUT_GPIO(), SET_GPIO_ALT() 를 설정하기 이전에 반드시 INP_GPIO()를 사용해 초기화를 할 것.
* INP_GPIO(g)       : 입력받은 gpio 핀 g(BCM 기준)에 대하여 입력모드으로 설정한다.
* OUT_GPIO(g)       : 입력받은 gpio 핀 g(BCM 기준)에 대하여 출력모드로 설정한다.
* SET_GPIO_ALT(g,a) : 입력받은 gpio 핀 g(BCM 기준)에 대하여 a에 해당하는 ALT 함수로 설정한다.
* GPIO_SET(g)       : 입력받은 gpio 핀 g(BCM 기준)에 대하여 SET(1)을 설정한다. (출력모드 전용)
* GPIO_CLEAR(g)     : 입력받은 gpio 핀 g(BCM 기준)에 대하여 CLEAR(0)을 설정한다. (출력모드 전용)
* GPIO_READ(g)      : 입력받은 gpio 핀 g(BCM 기준)에 대하여 READ를 수행하여 현재 핀의 값을 반환한다.(입력모드 전용)
*/
#define INP_GPIO(g) 		*(iom_gpio+((g)/10)) 	&= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) 		*(iom_gpio+((g)/10)) 	|=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) 	*(iom_gpio+(((g)/10))) 	|= 	(((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))
#define GPIO_SET(g) 		*(iom_gpio+7+((g)/32)) 	= 	(1<<(((g)%32))) 
#define GPIO_CLEAR(g) 		*(iom_gpio+10+((g)/32)) = 	(1<<(((g)%32)))
#define GPIO_READ(g) 		*(iom_gpio+13+((g)/32)) | 	(1<<(((g)%32)))

#define OUTPUT 1
#define INPUT 0

#define ON 1
#define OFF 0

#define SET 1
#define CLEAR 0

/* 
*********************************************************************************************************
*                                              PIN CONNECTION DEFINE
* 라즈베리파이 핀 설정
* KIST board       	--- Raspberry pi3
* J3 - 3 : SPI DI   --- pin 21 (BCM 9) : SPI MISO
* J3 - 4 : SPI DO   --- pin 19 (BCM 10): SPI MOSI
* J3 - 5 : SPI CLK  --- pin 23 (BCM 11): SPI SCLK
* J3 - 6 : SPI ENA1 --- pin 24 (BCM 8) : SPI CS0
* J3 - 7 : SPI ENA2	--- pin 26 (BCM 7) : SPI CS1
*
* J4 - 4 : DAC_CLK	--- pin 23 (BCM 11): SPI SCLK
* J4 - 5 : DAC_CS 	--- pin 38 (BCM 20) : SPI CS2(by overy dtb)
* J4 - 7 : DAC_DAT 	--- pin 19 (BCM 10): SPI MOSI
*********************************************************************************************************
*/

#define PIN_MOTOR_BREAK_L 5 
#define PIN_MOTOR_BREAK_R 6

#define PIN_MOTOR_DIRECTION_L 13
#define PIN_MOTOR_DIRECTION_R 19

/*
*********************************************************************************************************
*                                              SPI DEFINE MACROS & VARIABLE
*********************************************************************************************************
*/
#define SPI_DAC_SPEED   	1000000  	//1MHz
#define SPI_DAC_SPEED_MAX   50000000 	// DAC 최대 동작 주파수 50MHz
#define SPI_ENC_SPEED 		10000  		//10KHz

#define SPI_DAC_CHANNEL 0
#define SPI_ENC_L_CHANNEL 1
#define SPI_ENC_R_CHANNEL 2

#define SPI_MODE 0
#define SPI_BPW  8
#define SPI_DELAY 0

static uint32_t 		spi_fds[3]		= {0,};
static uint32_t 	    spi_speeds[3]	= {0,}; 
static uint32_t 	    spi_delays[3] 	= {0,}; 
static uint32_t 	    spi_bpws[3]		= {0,}; 

/*
*********************************************************************************************************
*                                              PREDEFINE FUNCTION
*********************************************************************************************************
*/
int rpi_gpio_setup(void);
int rpi_gpio_direction(unsigned int pin_num, unsigned int mode);
int rpi_gpio_alt_func(unsigned int pin_num, unsigned int mode);
int rpi_gpio_write(unsigned int pin_num, unsigned int status);
int rpi_gpio_read(unsigned int pin_num);
int rpi_spi_setup(int channel, int mode, int bits_per_word, int speed, int delay);
int rpi_spi_data_rw(int channel, unsigned char *data, int len);
void rpi_spi_close(void);

#endif
