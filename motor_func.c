/*
*********************************************************************************************************
*                                             MOTOR_FUNC_C
*********************************************************************************************************
*/
#include <stdio.h> 
#include <stdlib.h>
#include <unistd.h> 
#include <stdint.h> 
#include "motor_func.h"
#include "rpi_func.h"

/*
*********************************************************************************************************
*                           RASPBERRY PI MOTOR DAC,BREAK,DIRECTION,ENCODER FUNC
*********************************************************************************************************
*/

/*
* 하드웨어 초기화 함수
* int motor_hw_init(void)
* 입력 값 : 없음
* 반환 값 : 성공 0 / 실패 -1
* 설명 : SPI 관련 핀들은 이미 초기화 되어 있으므로 재설정해주지 않음.
*       여기서는 Break, Direction 핀에 대해 방향,초기값을 설정.
*/
int motor_hw_init(void)
{
    int init_hw_out[4] = {PIN_MOTOR_BREAK_L, PIN_MOTOR_BREAK_R, \
                          PIN_MOTOR_DIRECTION_L, PIN_MOTOR_DIRECTION_R };
    int i,ret;

    /* gpio 핀을 출력으로 설정하고 0으로 출력 초기화*/
    for(i=0; i<sizeof(init_hw_out)/sizeof(int); i++){
        if((ret = rpi_gpio_direction(init_hw_out[i],OUTPUT)) < 0)   break;
        if((ret = rpi_gpio_write(init_hw_out[i],ON)) < 0)          break;
    }
    return ret;
}

/*
* 브래이크 함수 
* int brake_wheel(int wheel_direction, int cmd)
* 입력 값 : wheel_direction ==> LEFT_WHEEL / RIGHT_WHEEL
*         cmd  ==> BREAK_ON / BREAK_OFF   
* 반환 값 : 성공 0 / 실패 -1
*/
int brake_wheel(int wheel_direction, int cmd)
{
    int ret = 0, direction = 0, flag = 0;

    if( (wheel_direction != LEFT_WHEEL) & (wheel_direction != RIGHT_WHEEL) )    return -1;
    if( (cmd != BREAK_ON) & (cmd != BREAK_OFF) )                                return -1;
    
    direction   = (wheel_direction == LEFT_WHEEL) ? PIN_MOTOR_BREAK_L : PIN_MOTOR_BREAK_R;
    flag        = (cmd == BREAK_ON) ? BREAK_ON : BREAK_OFF;

    if((ret = rpi_gpio_write(direction,flag))<0){
        printf("Break Gpio Write Error\n");
        return -1;
    }

#ifdef M_DEBUG
    printf("BREAK %s %s \n", (wheel_direction == LEFT_WHEEL) ? "LEFT WHEEL" : "RIGHT WHEEL",\
                              (cmd == BREAK_ON) ? "ON" : "OFF");
#endif  
    return ret;
}

/*
* 모터 방향 조절 함수
* int set_direction(int wheel_direction, int cmd)
* 입력 값 : wheel_direction ==> LEFT_WHEEL / RIGHT_WHEEL
*         cmd  ==> FORWARD / BACKWARD   
* 반환 값 : 성공 0 / 실패 -1
*/
int set_direction(int wheel_direction, int cmd)
{
    int ret = 0, direction = 0, flag = 0;

    if( (wheel_direction != LEFT_WHEEL) & (wheel_direction != RIGHT_WHEEL) )    return -1;
    if( (cmd != BREAK_ON) & (cmd != BREAK_OFF) )                                return -1;

    direction   = (wheel_direction == LEFT_WHEEL) ? PIN_MOTOR_DIRECTION_L : PIN_MOTOR_DIRECTION_R;
    flag        = (cmd == FORWARD) ? FORWARD : BACKWARD;

    if((ret = rpi_gpio_write(direction,flag))<0){
        printf("Set Direction Gpio Write Error\n");
        return -1;
    }
    
#ifdef M_DEBUG
    printf("SET DIRECTION : %s Move %s \n", (wheel_direction == LEFT_WHEEL) ? "LEFT WHEEL" : "RIGHT WHEEL",\
                                            (cmd == FORWARD) ? "FORWARD" : "BACKWARD");
#endif        

    return ret;
}


/*
* DAC를 통해 모터에 제어 입력 보내는 함수
* int writeDAC(unsigned char addr, unsigned char cmd, unsigned short data)
* 입력 값 : addr ==> DAC_ADDR_LEFT / DAC_ADDR_RIGHT / DAC_ADDR_ALL
*         cmd  ==> DAC_CMD_* 헤더파일 참조.
*         data ==> 보낼 데이터   
* 반환 값 : 성공 보낸 word의 수 / 실패 -1
* 설명 : 모터 디바이스 드라이버 write 함수
*       총 24비트로 이루어짐.
*       C3 C2 C1 C0 A3 A2 A1 A0 D9 D8 D7 D6 D5 D4 D3 D2 D1 D0 XX XX XX XX XX XX 
*       최상위 4비트 : Command / 다음 4비트 : Address / 다음 10비트 : Data / 다음 6비트 : Don't care
*/
int writeDAC(unsigned char addr, unsigned char cmd, unsigned short data)
{
    unsigned char buff[3]={0,}; 
    int ret = 0;

    //임계값 처리
    if(data>DAC_DATA_MAX) data = DAC_DATA_MAX;
    if(data<DAC_DATA_MIN) data = DAC_DATA_MIN;
    
    // DAC data set-up 총 24비트.
    buff[0] = cmd<<4 | addr;  
    buff[1] = (data<<6)>>8; 
    buff[2] = data<<6; 

#ifdef M_DEBUG
    printf("data : %x send_data : %x \n", data, (buff[0]<<16)|(buff[1]<<8)|buff[2]);
#endif

    if((ret = rpi_spi_data_rw(SPI_DAC_CHANNEL , buff, 3)) < 0)
        printf("SPI DATA WRITE ERROR\n");

    return ret;
}

/*
* Encoder 데이터를 읽어오는 함수
* unsigned short encoder_read(int wheel_direction)
* 입력 값 : wheel_direction ==>  LEFT_WHEEL / RIGHT_WHEEL //읽어올 wheel의 방향
* 반환 값 : 읽어온 encoder 데이터의 값/ 실패 -1
*/
unsigned short encoder_read(int wheel_direction)
{
    int ret = -1, en_data = 0;
    unsigned short en_re_data = 0, en_cmd_data = 0;
    unsigned char buf[3] = {0,};
#ifdef E_DEBUG
    int i=0;
#endif

    if(wheel_direction == LEFT_WHEEL)
        ret = rpi_spi_data_rw(SPI_ENC_L_CHANNEL,buf,3);
    else if(wheel_direction == RIGHT_WHEEL)
        ret = rpi_spi_data_rw(SPI_ENC_R_CHANNEL,buf,3);
    else{
        printf("Invalid Argument \n");
        return -1;
    }

    en_data     = ((buf[0] << 16 | buf[1] << 8 | buf[2]) >> 5) & 0x0003ffff;
    en_re_data  = (en_data)>>6;
    en_cmd_data = (en_data)&0x003f;

#ifdef E_DEBUG
    printf("read %d bit\t",ret*sizeof(char));
    for(i=0;i<3;i++)
        printf("%x",buf[i]);
    printf("\nen_data : %x, en_re_data : %x, en_cmd_data : %x \n",en_data,en_re_data,en_cmd_data);
    printf("Encoder State ==> \n\tOCF : %d COF : %d LIN : %d MagINC : %d MagDEC : %d Parity : %d \n", 
                                 (en_cmd_data&0x20)>>5,  // OCF(Offset Compensation Finished ), logic high ==> the finished Offset Compensation Algorithm 
                                 (en_cmd_data&0x10)>>4,  // COF(Cordic Oveerflow), logic high ==> an out of range error in the CORDIC part. when this bit is set, data is invalid
                                 (en_cmd_data&0x08)>>3,  // LIN(Linearity Alarm), logic high ==> the input field generates a critical output linearity. whe this bit is set, data can contain invalid data.
                                 (en_cmd_data&0x04)>>2,  // MagInc : ?
                                 (en_cmd_data&0x02)>>1,  // MagDec : ?
                                 (en_cmd_data&0x01)>>0   // Even Parity : transmission error detection.
                                 );
#endif
    return en_re_data;
}

/*
*********************************************************************************************************
*                                    RASPBERRY PI MOTOR PI CONTROL FUNC
*********************************************************************************************************
*/

/*
* 위치 PI제어 함수 (임시로 작성됨)
* 해당 함수는 Kist 제공된 코드에 임의로 맞춰 작성된 함수임.
* 전달된 코드에서의 전달함수나 모델에 대한 정보가 전달되지 않음으로 가장 기본적인 PI제어룰 구현함. 
* 정확한 제어를 위해서는 리눅스에서는 타이머 인터럽트, 멀티 스레드 등에 재작성 바람. 혹은 RTOS에서 실시간 태스크로 구현 바람.
* while문에서는 정확한 제어는 불가능함.
* void pos_control(int ref_pos, int wheel_direction, int move_direction)
* 입력 값 : ref_pos ==> 원하는 이동 각도 
*         wheel_direction ==> LEFT_WHEEL / RIGHT_WHEEL
*         move_direction ==> FORWARD / BACKWARD
* 반환 값 : 오차 값
*/

int pos_control(int ref_pos, int wheel_direction, int move_direction)
{
    unsigned short cur_encoder=0, err_encoder=0;
    static unsigned short prev_encoder = 0;

    float err_pos = 0, input_dac = 0;
    static float feedback_pos = 0, err_pos_i = 0;

    int mv_direction = move_direction;
    static int i=1;

    //함수 실행시 1번만 실행.
    while(i){
        prev_encoder = encoder_read(wheel_direction);   // 해당 함수 실행시 초기 err_encoder값을 0으로 하기 위함. 
        set_direction(wheel_direction,mv_direction);  // 모터 방향 설정
        i=0;    
    }

    //절대 엔코더 값 읽기
    cur_encoder = encoder_read(wheel_direction);

    //check_over_under_flow 
    //-방향으로 진행시 엔코더의 값이 0xfff --> 0x000으로 엔코더 초기화
    //+방향으로 진행시 엔코더의 값이 0x000 --> 0xfff으로 엔코더 초기화의 경우 연산.
    //ex. -방향 진행시 prev_enc = 0xff0 --> cur_enc = 0x001 일경우 (0x001 + 0xfff) - 0xff0 = 0x011 만큼의 변화가 일어남.
    if( (mv_direction == BACKWARD) & (prev_encoder > cur_encoder + ENCODER_ERR) )              
        cur_encoder += UNIT_ENCODER_RESOLUTION;
    else if( (mv_direction == FORWARD) & (cur_encoder > prev_encoder + ENCODER_ERR) )        
        prev_encoder += UNIT_ENCODER_RESOLUTION;

    //unsigned value로 err_encoder 사용.
    //err_encoder = abs(cur_encoder - prev_encoder);    
    if(cur_encoder>prev_encoder)             err_encoder = cur_encoder - prev_encoder;
    else if(prev_encoder>cur_encoder)        err_encoder = prev_encoder - cur_encoder;

    //prev_encoder값 갱신    
    if(cur_encoder > UNIT_ENCODER_RESOLUTION) cur_encoder -= UNIT_ENCODER_RESOLUTION;
    prev_encoder = cur_encoder;

    //현재 이동 거리(degree) += 엔코더 에러 * 360 / encoder resoultion / gear ratio
    feedback_pos += (float)(err_encoder * 360 / UNIT_ENCODER_RESOLUTION / GEAR_RATIO); 

    //오차 계산
    err_pos = ref_pos - feedback_pos;
    err_pos_i += err_pos * dT;
    if(err_pos < 0){
        mv_direction = (mv_direction == FORWARD) ? BACKWARD : FORWARD;
        set_direction(wheel_direction,mv_direction);
    }

    //PI 제어기 
    input_dac = Kp*err_pos + Ki*err_pos_i;

    //제어입력 DAC로 보내기 
    if(wheel_direction == LEFT_WHEEL)
        writeDAC(DAC_ADDR_LEFT, DAC_CMD_WRUP, (unsigned short)input_dac);
    else if(wheel_direction == RIGHT_WHEEL)
        writeDAC(DAC_ADDR_RIGHT, DAC_CMD_WRUP, (unsigned short)input_dac); 

//현재 PI 제어의 샘플링은 1ms인데 printf문은 block function이므로 사용하지 않기를 권함.
//반드시 사용해야할 경우 100ms 샘플링이상에서 사용을 권함. 하지만 이때는 샘플링 부족으로 err_encoder값을 보장할 수 없음.
#ifdef PI_DEBUG 
    printf("cur_encoder : 0x%x \t",cur_encoder);
    printf("err_encoder : 0x%x \t",err_encoder);
    printf("feedback_pos: %.2f \t",feedback_pos);
    printf("err_pos: %.2f \terr_pos_i: %.2f\t",err_pos,err_pos_i);
    printf("input_dac: 0x%x \n",(unsigned short)input_dac);
#endif
    return (int)err_pos;
}


/*
* 속도 PI제어 함수 (임시로 작성됨)
* 해당 함수는 Kist 제공된 코드에 임의로 맞춰 작성된 함수임.
* 전달된 코드에서의 전달함수나 모델에 대한 정보가 전달되지 않음으로 가장 기본적인 PI제어룰 구현함. 
* 정확한 제어를 위해서는 리눅스에서는 타이머 인터럽트, 멀티 스레드 등에 재작성 바람. 혹은 RTOS에서 실시간 태스크로 구현 바람.
* while문에서는 정확한 제어는 불가능함.
* void vel_control(int ref_vel, int wheel_direction, int move_direction)
* 입력 값 : ref_vel ==> 원하는 이동 속도(여기서는 degree/sec) 
*         wheel_direction ==> LEFT_WHEEL / RIGHT_WHEEL
*         move_direction ==> FORWARD / BACKWARD
* 반환 값 : 오차 값
*/

int vel_control(int ref_vel, int wheel_direction, int move_direction)
{
    unsigned short cur_encoder=0, err_encoder=0;
    static unsigned short prev_encoder = 0;

    float feedback_vel = 0, err_vel = 0;
    static float err_vel_i = 0, input_dac = 0;

    int mv_direction = move_direction;
    static int i=1;

    //함수 실행시 1번만 실행.
    while(i){
        prev_encoder = encoder_read(wheel_direction);   // 해당 함수 실행시 초기 err_encoder값을 0으로 하기 위함. 
        set_direction(wheel_direction,mv_direction);  // 모터 방향 설정
        i=0;    
    }

    //절대 엔코더 값 읽기
    cur_encoder = encoder_read(wheel_direction);

    //check_over_under_flow 
    //-방향으로 진행시 엔코더의 값이 0xfff --> 0x000으로 엔코더 초기화
    //+방향으로 진행시 엔코더의 값이 0x000 --> 0xfff으로 엔코더 초기화의 경우 연산.
    //ex. -방향 진행시 prev_enc = 0xff0 --> cur_enc = 0x001 일경우 (0x001 + 0xfff) - 0xff0 = 0x011 만큼의 변화가 일어남.
    if( (mv_direction == BACKWARD) & (prev_encoder > cur_encoder + ENCODER_ERR) )              
        cur_encoder += UNIT_ENCODER_RESOLUTION;
    else if( (mv_direction == FORWARD) & (cur_encoder > prev_encoder + ENCODER_ERR) )        
        prev_encoder += UNIT_ENCODER_RESOLUTION;

    //unsigned value로 err_encoder 사용.
    //err_encoder = abs(cur_encoder - prev_encoder);    
    if(cur_encoder>prev_encoder)             err_encoder = cur_encoder - prev_encoder;
    else if(prev_encoder>cur_encoder)        err_encoder = prev_encoder - cur_encoder;
    
    //prev_encoder값 갱신        
    if(cur_encoder > UNIT_ENCODER_RESOLUTION) cur_encoder -= UNIT_ENCODER_RESOLUTION;    
    prev_encoder = cur_encoder;

    //순간속도 = (enc * 360 / 4095(Resoultion) / 6.3(Gear ratio)) / 0.001(dT)
    feedback_vel = (float)(err_encoder * 360 / UNIT_ENCODER_RESOLUTION / GEAR_RATIO / dT); 

    //속도 오차 계산
    err_vel = ref_vel - feedback_vel;
    err_vel_i += err_vel * dT;

    //PI 제어기 
    input_dac += Kp*err_vel + Ki*err_vel_i;

    //제어입력 DAC로 보내기 
    if(wheel_direction == LEFT_WHEEL)
        writeDAC(DAC_ADDR_LEFT, DAC_CMD_WRUP, (unsigned short)input_dac);
    else if(wheel_direction == RIGHT_WHEEL)
        writeDAC(DAC_ADDR_RIGHT, DAC_CMD_WRUP, (unsigned short)input_dac); 

//현재 PI 제어의 샘플링은 1ms인데 printf문은 block function이므로 사용하지 않기를 권함.
//반드시 사용해야할 경우 100ms 샘플링이상에서 사용을 권함. 하지만 이때는 샘플링 부족으로 err_encoder값을 보장할 수 없음.
#ifdef PI_DEBUG 
    printf("cur_encoder : %d \t",cur_encoder);
    printf("err_encoder : %d \t",err_encoder);
    printf("feedback_vel : %.2f \t",feedback_vel);
    printf("err_vel: %.2f \terr_vel_i: %.2f\t",err_vel,err_vel_i);
    printf("input_dac: %x \n",(unsigned short)input_dac);
#endif
    return (int)err_vel;
}


/*
* 테스트용 함수. 
* DAC에 써준 값에 의해 돌아가는 중 엔코더 값을 읽어와 데이터 표기
* void pos_speed_printf(int wheel_direction, int move_direction)
* 입력 값 : wheel_direction ==> LEFT_WHEEL / RIGHT_WHEEL
*         move_direction ==> FORWARD / BACKWARD
* 반환 값 : 없음
*/
void pos_speed_printf(int wheel_direction, int move_direction)
{
    unsigned short cur_encoder=0, err_encoder=0;
    static unsigned short prev_encoder = 0;

    float tmp_speed=0,avg_speed=0;
    static float pos = 0, prev_pos=0, T = 0;
    static int i=1;
    
    while(i){
        prev_encoder = encoder_read(wheel_direction);
        i=0;    
    }

    // read encoder value
    cur_encoder = encoder_read(wheel_direction);

    //check_over_under_flow
    if( (move_direction == BACKWARD) & (prev_encoder > cur_encoder + ENCODER_ERR) )
        cur_encoder += UNIT_ENCODER_RESOLUTION;
    else if( (move_direction == FORWARD) & (cur_encoder > prev_encoder + ENCODER_ERR) )
        prev_encoder += UNIT_ENCODER_RESOLUTION;

    //err_encoder 계산
    if(cur_encoder>prev_encoder)
        err_encoder = cur_encoder - prev_encoder;
    else if(prev_encoder>cur_encoder)
        err_encoder = prev_encoder - cur_encoder;
    
    //prev_encoder값 갱신        
    if(cur_encoder > UNIT_ENCODER_RESOLUTION) cur_encoder -= UNIT_ENCODER_RESOLUTION;    
    prev_encoder = cur_encoder;

    //이동거리, 순간속도, 평균 속도 계산
    pos += (float)err_encoder*360/UNIT_ENCODER_RESOLUTION/GEAR_RATIO;
    tmp_speed = (float)(pos - prev_pos)/ dT;
    T += dT;
    avg_speed = pos / T;
    prev_pos = pos; 

    printf("cur_encoder : %d \t",cur_encoder);  
    printf("err_encoder : %d \t",err_encoder);
    printf("Pos : %.2f tmp speed : %.2f avg speed : %.2f \n",pos,tmp_speed,avg_speed);

}
