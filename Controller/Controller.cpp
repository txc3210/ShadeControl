// Controller.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include <assert.h>
#include "Controller.h"
#include <thread>
#include <chrono>
#include <ctime>

#pragma comment(lib, "Serial.lib")

std::string get_current_time()
{
	char str[20];
	using std::chrono::system_clock;
	std::time_t tt = system_clock::to_time_t(system_clock::now());
	struct std::tm tms;
	localtime_s(&tms, &tt);
	strftime(str, sizeof(str), "%Y-%m-%d %H:%M:%S", &tms);
	return std::string(str);
}

unsigned short crc_16b(unsigned char *pBuf, unsigned short len)
{
	unsigned short uCRC = 0xFFFF;
	int i, j;
	for (i = 0; i<len; i++)
	{
		uCRC ^= (*(pBuf + i));
		for (j = 0; j<8; j++)
		{
			if ((uCRC & 0x0001) == 0x0001)
			{
				uCRC = (uCRC >> 1);
				uCRC ^= 0xA001;
			}
			else
				uCRC = (uCRC >> 1);
		}
	}
	//uCRC = (uCRC >> 8) | (uCRC << 8);
	return uCRC;
}

/*************************************************************
@Function: Turn the ange to motor time
@Param angel: Angle wanted for the board
@Return: Time for the motor to run to the angel
*************************************************************/
unsigned short controller::angle_to_time(unsigned char angle)
{
	float time = (float)angle / 90.0f * MAX_ANGLE_TIME;
	return static_cast<unsigned short>(time);
}

/******************************************************************
@Function: Turn the motor time to board angle
@Param time: Time for the motor to make the board to current angel
@Return: Angle wanted for the board
*******************************************************************/
unsigned char controller::time_to_angle(unsigned short time)
{
	float angle = (float)time / MAX_ANGLE_TIME * 90.0f;
	return static_cast<unsigned char>(angle);
}

/*********************************************************************************************
@Function: Set target angel fo the board
@Param addr: Device address of the controller on RS485 bus
@Param angle: Target angle of board
@Return: 0 for success , <0 values for errors
*********************************************************************************************/
int controller::set_angle(unsigned char angle)
{
	unsigned char send_buf[8];
	unsigned char recv_buf[8];
	if (!port->IsOpened())
		return -1;
	send_buf[0] = addr;
	send_buf[1] = 0x06;
	send_buf[2] = 0xA0;
	send_buf[3] = 0x00;
	unsigned short time = angle_to_time(angle);
	send_buf[4] = static_cast<unsigned char>(time >> 8);
	send_buf[5] = static_cast<unsigned char>(time);
	unsigned short crc = crc_16b(send_buf, 6);
	send_buf[6] = static_cast<unsigned char>(crc); //CRC低字节
	send_buf[7] = static_cast<unsigned char>(crc >> 8);//CRC高字节
	port->Clear();
	int num = port->SendData(send_buf, 8);
	if (num != 8)
		return -2;
	//Sleep(RS485_WAIT_TIME);
	std::this_thread::sleep_for(std::chrono::milliseconds(RS485_WAIT_TIME));//C++延时方法
	if (addr == 0) //广播地址发送数据时没有返回
		return 0;
	num = port->ReadData(recv_buf, 6);
	if (num == 0)
	{
		state = RS485_NO_REPLY;
		return -3;
	}
	else if (num != 6)
		return -4;
	if (recv_buf[0] != addr || recv_buf[1] != 0x06)
		return -5;
	state = RS485_OK;
	return 0;
}

/**************************************************************************
@Function: Get current angle of the board
@Param addr: Device address of the controller on RS485 bus
@Param angle: Point the data to store the angle;
@Return: 0 for success, <0 value for error;
**************************************************************************/
int controller::get_angle()
{
	assert(addr <= 247);
	unsigned char send_buf[6];
	unsigned char recv_buf[6];
	if (!port->IsOpened())
		return -1;
	send_buf[0] = addr;
	send_buf[1] = 0x03;
	send_buf[2] = 0xA0;
	send_buf[3] = 0x00;	
	unsigned short crc = crc_16b(send_buf, 4);
	send_buf[4] = static_cast<unsigned char>(crc);//CRC低字节
	send_buf[5] = static_cast<unsigned char>(crc >> 8);//CRC高字节
	port->Clear();
	int num = port->SendData(send_buf, 6);
	if (num != 6)
		return -2;
	//Sleep(RS485_WAIT_TIME);	
	timestamp = get_current_time();
	std::this_thread::sleep_for(std::chrono::milliseconds(RS485_WAIT_TIME));
	num = port->ReadData(recv_buf, 6);
	if (num == 0)
	{
		state = RS485_NO_REPLY;
		return -3;
	}
	else if (num != 6)
		return -4;
	if (recv_buf[0] != addr || recv_buf[1] != 0x03)
		return -5;
	crc = crc_16b(recv_buf, 4);
	if (crc != (recv_buf[4] + recv_buf[5] * 256))
		return -6;
	unsigned short time = recv_buf[2] * 256 + recv_buf[3];	
	angle = time_to_angle(time);
	state = RS485_OK;
	return 0;
}

/****************************************************************
@Function: Set the mode of controller
@Param addr: Device address of controller on RS485 bus
@Param mode: Mode of controller, can be MODE_AUTO or MODE_MANUAL
@Return: 0 for success , <0 value for errors;
****************************************************************/
int controller::set_mode(unsigned char mode)
{
	unsigned char send_buf[8];
	unsigned char recv_buf[8];
	if (!port->IsOpened())
		return -1;
	send_buf[0] = addr;
	send_buf[1] = 0x06;
	send_buf[2] = 0xA0;
	send_buf[3] = 0x02;
	
	send_buf[4] = 0x00;
	send_buf[5] = mode ;
	unsigned short crc = crc_16b(send_buf, 6);
	send_buf[6] = static_cast<unsigned char>(crc); //CRC低字节
	send_buf[7] = static_cast<unsigned char>(crc >> 8);//CRC高字节
	port->Clear();
	int num = port->SendData(send_buf, 8);
	if (num != 8)
		return -2;
	//Sleep(RS485_WAIT_TIME);
	std::this_thread::sleep_for(std::chrono::milliseconds(RS485_WAIT_TIME));
	if (addr == 0) //广播地址发送数据时没有返回
		return 0;
	num = port->ReadData(recv_buf, 6);
	if (num == 0)
	{
		state = RS485_NO_REPLY;
		return -3;
	}
	else if (num != 6)
		return -4;
	if (recv_buf[0] != addr || recv_buf[1] != 0x06)
		return -5;	
	state = RS485_OK;
	return 0;
}


/**************************************************************************
@Function: Get current angle of the board
@Param addr: Device address of the controller on RS485 bus
@Param angle: Point the data to store the mode;
@Return: 0 for success, <0 value for error;
**************************************************************************/
int controller::get_mode()
{
	assert(addr <= 247);
	unsigned char send_buf[6];
	unsigned char recv_buf[6];
	if (!port->IsOpened())
		return -1;
	send_buf[0] = addr;
	send_buf[1] = 0x03;
	send_buf[2] = 0xA0;
	send_buf[3] = 0x02;
	unsigned short crc = crc_16b(send_buf, 4);
	send_buf[4] = static_cast<unsigned char>(crc); //CRC低字节
	send_buf[5] = static_cast<unsigned char>(crc >> 8);//CRC高字节
	port->Clear();
	int num = port->SendData(send_buf, 6);
	if (num != 6)
		return -2;
	//Sleep(RS485_WAIT_TIME);
	std::this_thread::sleep_for(std::chrono::milliseconds(RS485_WAIT_TIME));
	num = port->ReadData(recv_buf, 6);
	if (num == 0)
	{
		state = RS485_NO_REPLY;
		return -3;
	}
	if (num != 6)
		return -4;
	if (recv_buf[0] != addr || recv_buf[1] != 0x03)
		return -5;
	crc = crc_16b(recv_buf, 4);
	if (crc != (recv_buf[4] + recv_buf[5] * 256))
		return -6;	
	mode = recv_buf[3];
	state = RS485_OK;
	return 0;
}

/***************************************************************************
@Function: Synchronize current angle and mode of controller to server
@Return: 0 for success, -1 when get_angel failed, -2 when get_mode failed
***************************************************************************/
int controller::synchroize()
{
	int ret = 0;
	ret = get_angle();
	if (ret != 0)
		return -1;
	ret = get_mode();
	if (ret != 0)
		return -2;
	return 0;
}

/**********************************************************************
@Function: Set mode and angle of the controller
@Param mode: Mode want to set the controller
@Param angle: Angle want to set the controller
@Return: 0 for success, -1 or -2 for failed
**********************************************************************/
int controller::flush(unsigned char mode, unsigned char angle)
{
	int ret = 0;
	ret = set_mode(mode);
	if (ret != 0)
		return -1;
	ret = set_angle(angle);
	if (ret != 0)
		return -2;
	return 0;
}


