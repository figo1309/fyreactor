#include <net/tcpserver.h>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace fyreactor;

CTCPServer server;

void OnAccept(socket_t sockId)
{
	printf("OnAccept %d \n", sockId);
}

void OnMessage(socket_t sockId, const char* message, uint32_t len)
{
	server.ReadySendMessage(sockId, message, len);
}

void OnClose(socket_t sockId)
{
	printf("OnClose %d \n", sockId);
}

void GetInput()
{
	std::string strInput;

	while (true)
	{
		std::cin >> strInput;

		if (strcmp(strInput.c_str(), "Stop") == 0)
		{
			server.Stop();
			return;
		}
	}
}

void Func1()
{
	printf("#### 5s #### \n");
}

void Func2()
{
	printf("#### 2500ms #### \n");
}

void Func3()
{
	printf("#### 3200ms #### \n");
}

int main()
{
	std::thread inputThread(GetInput);

	server.Listen("192.168.30.29", 9999);
	server.RegAcceptFunc(OnAccept);
	server.RegMessageFunc(OnMessage);
	server.RegCloseFunc(OnClose);

	/*server.GetTimerThread().Register(false, 5, Func1);
	server.GetTimerThread().RegisterByMilSec(false, 2500, Func2);
	server.GetTimerThread().RegisterByMilSec(true, 3200, Func3);*/

	server.Run();

	inputThread.join();
	return 0;
}