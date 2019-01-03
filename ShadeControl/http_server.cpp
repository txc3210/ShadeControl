#include <WinSock2.h>
#include <iostream>


int http_server_thread()
{
	WSADATA wsaData;
	int err;

	WORD wVersionRequested = MAKEWORD(1, 1);
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		std::cout << "HttpServer网络通信初始化出错，请重试！" << std::endl;
		return -1;
	}

	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		std::cout << "HttpServer网络通信初始化出错，请重试！" << std::endl;
		return -2;
	}

	if (!GetHttpServerPort(&HttpServerPort))
	{
		std::cout << "从Setting.ini文件中读取HTTP服务器端口号失败!" << std::endl;
		return -21;
	}

	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(HttpServerPort);

	err = bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));
	if (err == SOCKET_ERROR)
	{
		std::cout << "HttpServer绑定端口号失败,请检查端口号是否被占用" << std::endl;
		return -3;
	}
	err = listen(sockSrv, 5);
	if (err == SOCKET_ERROR)
	{
		std::cout << "HttpServer监听端口号失败,请检查端口号是否被占用" << std::endl;
		return -4;
	}

	SOCKADDR_IN addrClient;
	int len = sizeof(SOCKADDR);
	while (1)
	{
		SOCKET sockClient = accept(sockSrv, (SOCKADDR*)&addrClient, &len);
		if (sockClient == INVALID_SOCKET)
		{
			continue;
		}
		std::cout << "有设备接入了Http服务器,等待传输数据" << std::endl;

		if (HttpThreadCount >= 1000)
		{

			//char str[]="{\"Result\":\"ServerBusy\"}";
			//HttpSend(sockClient,str,(int)strlen(str),0);
			shutdown(sockClient, SD_BOTH);
			continue;
		}


		CWinThread *pThread = AfxBeginThread(ThreadHttpClient, (LPVOID)sockClient);
		if (pThread == NULL)//在线用户在多，创建线程失败
		{
			TRACE0("在线用户太多，创建线程失败,关闭此连接!");
			//strRet="Action=ServerBusy*\n";
			//send(*pSocket,strRet,strRet.GetLength(),0);
			shutdown(sockClient, SD_BOTH);
		}
	}
	//	ThreadCount--;
	return 0;
}