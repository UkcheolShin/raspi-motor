/*
*********************************************************************************************************
*                                              MOTOR_FUNCTION.H
*********************************************************************************************************
*/
#ifndef __MOTOR_H__
#define __MOTOR_H__

/*디버그 옵션
* M_DEBUG DAC 관련 정보 print
* E_DEBUG Encdoer 관련 정보 print
* PI_DEBUG PI제어 관련 정보 printf (사용 하지 않길 권장).
*/
#define M_DEBUG
#define E_DEBUG
#define PI_DEBUG

/*
*********************************************************************************************************
*                                  KIST DAC DEFINE MACROS & VARIABLE
*********************************************************************************************************
*/

/*
* 디바이스 : DAC LTC2632 CTS8-HZ10
* 프로토콜 : SPI
* 입력 전압 : 4.5 ~ 5.5V
* 총 24비트로 이루어짐.
* 하위 6비트는 Don't Care
* C3 C2 C1 C0 A3 A2 A1 A0 D9 D8 D7 D6 D5 D4 D3 D2 D1 D0 XX XX XX XX XX XX 
* 최상위 4비트 : Command / 다음 4비트 : Address / 다음 10비트 : Data / 다음 6비트 : Don't care
* 자세한 사항은 데이터시트 참조.
*/

//DAC Command codes
#define DAC_CMD_WR_REG			0x0 	// Write to Input Register
#define DAC_CMD_UP 				0x1 	// Update(Power-up) DAC Register n
#define DAC_CMD_WRUP_ALL		0x2 	// Write to Input Register n, Update(Power-Up) All
#define DAC_CMD_WRUP			0x3 	// Write & Update DAC Register n
#define DAC_CMD_POWER_DOWN		0x4		// Power Down n
#define DAC_CMD_POWER_DOWN_ALL	0x5		// Power Down Chep (ALL DAC's and Reference)
#define DAC_CMD_SET_IN_REF		0x6		// Select Internal Reference 
#define DAC_CMD_SET_EX_REF		0x7		// Select External Reference(Power Down Internal Reference)
#define DAC_CMD_NO_OP			0xf 	// No Operation

// DAC Address codes
#define	DAC_ADDR_LEFT	0x0 // DAC A
#define DAC_ADDR_RIGHT	0x1 // DAC B
#define DAC_ADDR_ALL	0xf // ALL DAC

// DAC Data Range
// 10bit DAC 0~1023
#define DAC_DATA_MIN 	0x000 //0 실험적으로는 0x160 이상 주어야 바퀴를 움직일수 있는 토크가 발생.
#define DAC_DATA_MAX 	0x3ff //1023

#define DAC_MODE    0
#define BREAK_MODE	1

#define RIGHT_WHEEL 1
#define LEFT_WHEEL 	0

#define FORWARD 	1
#define BACKWARD 	0

#define BREAK_ON  	0
#define BREAK_OFF 	1

/*
*********************************************************************************************************
*                               KIST ENACODER DEFINE MACROS & VARIABLE
* 전달된 코드의 주석만으로는 정확한 정보 전달 및 파악에 어려움이 있어 실험을 진행함에 있어 임의로 작성된 값임.
* 수정한 값은 따로 주석 처리가 되어 있으므로 확인 후 수정하여 실험 진행하시면 됩니다.
*********************************************************************************************************
*/
#define dT 0.001
#define PI 3.141592
//#define GEAR_RATIO 6.3
#define GEAR_RATIO 10   
#define UNIT_ENCODER_RESOLUTION 4095
// 엔코더로 측정된 미소 변위량은 
// Diff_pos_degree = enc * 360 / Resoultion / Gear ratio
// Diff_pos_radian = Diff_pos_degree * 3.14 / 180
// Diff_vel_degree = Diff_pos_degree / dT
// Diff_vel_rpm = Diff_vel_degree * 60(1 minite) * 3.14 / 180
// Diff_vel_rpm = enc * 2 * PI * 60(1Min) / 4095(Resoultion) / 6.3(Gear ratio) / 0.001(dT)
//#define RPM_CONST 2.3251488095238095238095238  // 60(min) / RESOULTION / GEAR Ratio / dT	
#define Kp  3.5 
#define Ki  0.5
#define ENCODER_ERR 0x002 // 엔코더 오차가 0.006 degree이지만 10비트로 표현되므로 임의적으로 1step으로 설정함.


/*
*********************************************************************************************************
*                                              PREDEFINE FUNCTION
*********************************************************************************************************
*/
int motor_hw_init(void);
int brake_wheel(int wheel_direction, int cmd);
int set_direction(int wheel_direction, int cmd);
int writeDAC(unsigned char addr, unsigned char cmd, unsigned short data);
unsigned short encoder_read(int wheel_direction);
int pos_control(int ref_pos, int wheel_direction, int move_direction);
int vel_control(int ref_vel, int wheel_direction, int move_direction);
void pos_speed_printf(int wheel_direction, int move_direction);
#endif
