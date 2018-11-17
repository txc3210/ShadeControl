#pragma once
#include "..\ShadeControl\Serial.h"

#define RS485_WAIT_TIME		200 //RS485通讯等待时间，单位 ms

#define MAX_ANGLE_TIME		680.0f //百叶90度时对应的电机时间
#define MIN_ANGLE_TIME		680.0F //百叶90度时对应的电机时间

#define MAX_ANGLE	90.0f //百叶最大角度
#define MIN_ANGLE	0.0f //百叶最小角度

//控制模式
#define MODE_AUTO		0x00 //自动控制模式
#define MODE_MANUAL		0xFF //手动控制模式

//通讯状态
#define RS485_OK		0xF0 //通读正常
#define RS485_OFFLINE	0xF1 //设备没有响应

class controller
{
public:	
	controller(CSerial *com_port, unsigned char dev_addr)
	{
		state = 0;//默认通讯正常
		addr = dev_addr;
		mode = MODE_AUTO;
		angle = 0;
		port = com_port;
	}
	~controller() {};
	int set_angle(unsigned char angle);
	int get_angle();
	int set_mode(unsigned char mode);
	int get_mode();	
	int synchroize();

	int flush(unsigned char angle, unsigned char mode);

private:
	unsigned short angle_to_time(unsigned char angle);
	unsigned char time_to_angle(unsigned short time);

public:
	unsigned char state; //通讯状态
	unsigned char addr; //RS485地址
	unsigned char mode; //运行模式
	unsigned char angle; //角度，取值0-90
	
private:
	CSerial * port; //通讯串口
	unsigned short max_angle_time; //从0到90度时电机动作时间
	unsigned short min_angle_time; //从90到0度时电机动作时间
};
