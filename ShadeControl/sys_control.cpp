#include "stdafx.h"
#include <mutex>
#include <iomanip>
#include <ctime>

#pragma comment(lib, "Serial.lib")
#pragma comment(lib, "Controller.lib")
#pragma comment(lib, "libmysql.lib")

std::vector<controller> controller_list; //控制器列表
CSerial port; //串口对象
std::mutex port_mutex;

const std::string floor_name[11] = {
	"三楼", "四楼", "五楼", "六楼", "七楼", "八楼", "九楼",
	"十楼", "十一楼", "十二楼", "十三楼"
};


void InitConsoleWindow()
{
	//	int nCrt = 0;
	FILE* fp;
	//	AllocConsole();
	//	HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
	//	nCrt = _open_osfhandle((intptr_t)(handle), _O_TEXT);
	////	nCrt = _open_osfhandle((long)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
	//	fp = _fdopen(nCrt, "w");
	//	*stdout = *fp;
	//	setvbuf(stdout, NULL, _IONBF, 0);

	AllocConsole();
	freopen_s(&fp, "CONOUT$", "w+t", stdout);
	//freopen("CONIN$", "r+t", stdin);
	//下面是输出与输入
	//	std::cout << "Input:" << std::endl;
	//std::cin >> iTest;
	//fclose(stdout);
	//fclose(stdin);
	//FreeConsole();
}

/********************************************************************
功能：从数据库中加载遮阳百叶信息
参数：无
返回：0-成功，-1失败
********************************************************************/
int load_board()
{
	MYSQL mysql;
	MYSQL_RES * res = nullptr;
	if (nullptr == mysql_init(&mysql))
	{
		std::cout << __func__ << ": mysql_init failed: " << mysql_error(&mysql) << std::endl;
		return -1;
	}
	if (nullptr == mysql_real_connect(&mysql, DB_SERVER,
		DB_USER, DB_PASSWD, DB_NAME, DB_PORT, NULL, 0))
	{
		std::cout << __func__ << ": mysql_real_connect failed: " << mysql_error(&mysql) << std::endl;
		return -2;
	}
	std::cout << __func__ << ": mysql_real_connect success" << std::endl;
	if (0 != mysql_set_character_set(&mysql, "gb2312"))
	{
		std::cout << __func__ << ": mysql_set_character_set gb2312 failed: " << mysql_error(&mysql);
	}
	char sql[1024];
	int len = snprintf(sql, sizeof(sql), "%s", "select * from board order by id asc");
	int rc = mysql_real_query(&mysql, sql, len);
	if (rc != 0)
	{
		std::cout << __func__ << ": mysql_real_query failed: " << mysql_error(&mysql) << std::endl;
		return -3;
	}
	res = mysql_store_result(&mysql);
	if (res == nullptr)
	{
		std::cout << __func__ << ": mysql_store_result failed: " << mysql_error(&mysql) << std::endl;
		return -4;
	}
	unsigned int rows = static_cast<unsigned int>(mysql_num_rows(res));
	if (rows < 1)
	{
		std::cout << __func__ << ": no date in table board " << std::endl;
		return -5;
	}
	controller_list.reserve(rows + 1); //分配空间，要多存储一个地址为0的控制器对象
	unsigned char addr = 0;
	controller con(&port, addr);//地址为0的控制器，用于发广播指令
	con.name = std::string("广播地址");
	controller_list.push_back(con);//
	for (unsigned int i = 0; i < rows; i++)
	{
		MYSQL_ROW row = mysql_fetch_row(res);
		addr = static_cast<unsigned char>(atoi(row[4]));
		con.floor = static_cast<unsigned char>(atoi(row[1]));
		con.pos = static_cast<unsigned char>(atoi(row[2]));
		con.name = std::string(row[3]);
		con.addr = static_cast<unsigned char>(atoi(row[4]));
		con.mode = static_cast<unsigned char>(atoi(row[5]));
		con.angle = static_cast<unsigned char>(atoi(row[6]));
		con.enable = static_cast<unsigned char>(atoi(row[7]));
		controller_list.push_back(con);
	}
	mysql_free_result(res);
	mysql_close(&mysql);

	return 0;
}


void read_thread()
{
	std::cout << "开始轮流读取百叶的状态" << std::endl;
	while (1)
	{
		for (auto &  controller : controller_list) //使用引用，否则对象成员变量修改无效		
		{
			{//加此大括号是为了使sleep期间mutex被释放				
				if (controller.addr == 0)
					continue;
				std::lock_guard<std::mutex> guard(port_mutex);
			//	controller.get_mode();
				controller.get_angle();
			}
			std::this_thread::sleep_for(std::chrono::seconds(5));// C++ 11中提供的线程sleep方法
		}
	}
}

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

void time_test()
{
	using std::chrono::system_clock;
	std::time_t tt = system_clock::to_time_t(system_clock::now());

//	struct std::tm * ptm = std::localtime(&tt);
//	std::localtime_s()
	struct std::tm ptm;
	localtime_s(&ptm, &tt);
	std::cout << "Current time: " << std::put_time(&ptm, "%X") << '\n';
	while (1)
	{


		std::cout << "Waiting for the next minute to begin...\n";
		++ptm.tm_min; ptm.tm_sec = 0;
		std::this_thread::sleep_until(system_clock::from_time_t(mktime(&ptm)));

		std::cout << std::put_time(&ptm, "%X") << " reached!\n";

		//std::string = std::put_time(&ptm, "%X");
	}
}

/************************************************************************************
功能：系统初始化
参数：无
返回：无
************************************************************************************/
void system_init()
{
	InitConsoleWindow();
	if (!port.Open(2, 9600))
	{
		//MessageBox(_T("打开串口失败，请检查串口号是否存在或被占用!"), _T("错误"), MB_ICONERROR | MB_OK);
		std::cout << "打开串口失败，请检查串口号是否存在被被占用!" << std::endl;
		return;
	}
	load_board();
	std::thread th(read_thread);
	th.detach();
	std::thread th2(time_test);
	th2.detach();
	//controller_list.clear();
}