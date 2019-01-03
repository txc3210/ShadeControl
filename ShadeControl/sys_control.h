#pragma once
#include <string>
#include <mysql.h>
#include <iostream>
#include <vector>

#include "../Controller/Controller.h"
#include "Serial.h"

#define DB_NAME		"shade_control" //数据库名称
#define DB_SERVER	"192.168.108.2" //数据库IP地址
#define DB_PORT		3306			//数据库端口号 
#define DB_USER		"root"			//数据库用户名
#define DB_PASSWD	"jns12345"		//数据库密码

extern std::vector<controller> controller_list; //控制器列表
extern const std::string floor_name[11];


int load_board();
void read_thread();
void system_init();
std::string get_current_time();

#define MYSQL_OPEN_SUCCESS		0
#define MYSQL_INIT_ERROR		-1
#define MYSQL_CONNECT_ERROR		-2
class mysql_raii {
public:
	mysql_raii(MYSQL *ptr_mysql, MYSQL_RES ** ptr_mysql_res, const char *db)
	{
		m_ptr_mysql = ptr_mysql;
		m_ptr_mysql_res = ptr_mysql_res;
		if (nullptr == mysql_init(m_ptr_mysql))
		{
			std::cout << __func__ << ": mysql_init failed: " << mysql_error(m_ptr_mysql) << std::endl;
			m_mysql_state = MYSQL_INIT_ERROR;
			return;
		}
		if (nullptr == mysql_real_connect(m_ptr_mysql,
			DB_SERVER, DB_USER, DB_PASSWD, db, DB_PORT, NULL, 0))
		{
			std::cout << __func__ << ": mysql_real_connect failed: " << mysql_error(m_ptr_mysql) << std::endl;
			m_mysql_state = MYSQL_CONNECT_ERROR;
			return;
		}
		m_mysql_state = MYSQL_OPEN_SUCCESS;
		if (0 != mysql_set_character_set(m_ptr_mysql, "gb2312"))
		{
			std::cout << __func__ << ": mysql_set_character_set gb2312 failed: " << mysql_error(m_ptr_mysql);
		}

	}
	~mysql_raii()
	{
		if (*m_ptr_mysql_res != nullptr)
			mysql_free_result(*m_ptr_mysql_res);
		if (m_mysql_state == MYSQL_OPEN_SUCCESS)
			mysql_close(m_ptr_mysql);
	}
private:
	MYSQL * m_ptr_mysql;
	MYSQL_RES ** m_ptr_mysql_res;
	int m_mysql_state;
};