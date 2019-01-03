#pragma once
#include "..\ShadeControl\Serial.h"
#include <string>

// 通过宏来控制是导入还是导出
#ifdef CONTROLLER_EXPORTS
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

#define RS485_WAIT_TIME		200 //RS485通讯等待时间，单位 ms

#define MAX_ANGLE_TIME		680.0f //百叶90度时对应的电机时间
#define MIN_ANGLE_TIME		680.0F //百叶90度时对应的电机时间

#define MAX_ANGLE	90.0f //百叶最大角度
#define MIN_ANGLE	0.0f //百叶最小角度

//控制模式
#define MODE_AUTO		0x00 //自动控制模式
#define MODE_MANUAL		0xFF //手动控制模式

//通讯状态
#define RS485_OK		0x00 //通讯正常
#define RS485_NO_REPLY	0x01 //设备没有响应

class DLL_API controller
{
public:	

	controller(CSerial *com_port, unsigned char dev_addr)
	{
		state = 0;//默认通讯正常
		addr = dev_addr;
		mode = MODE_AUTO;
		angle = 0;
		port = com_port;
		timestamp.reserve(19);
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
	unsigned char floor; //所在楼层号，取值3-13
	unsigned char pos;//楼层安装位置，取值0-3，0为西面，1为南面-1
	unsigned char enable;//使能标志，0为禁止使用，1为可以使用
	std::string name; //百叶名称
	std::string timestamp;//数据更新时间
private:
	CSerial * port; //通讯串口
	unsigned short max_angle_time; //从0到90度时电机动作时间
	unsigned short min_angle_time; //从90到0度时电机动作时间
};
