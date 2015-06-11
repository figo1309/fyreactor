#include <net/tcpclient.h>
#include <stdio.h>
#include <iostream>

using namespace std;
using namespace fyreactor;

CTCPClient client;

void OnMessage(socket_t sockId, const char* message, uint32_t len)
{
	printf("OnMessage %d \n", sockId);
	client.ReadySendMessage(sockId, message, len);
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
			client.Stop();
			return;
		}
	}
}

void Func1(socket_t sockId)
{
	//printf("#### 5s #### \n");
	std::string str("hello");
	client.ReadySendMessage(sockId, str.c_str(), 5);
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

	socket_t sockId = client.Connect("192.168.30.29", 9999);
	if (sockId == -1)
	{
		printf("connect failed \n");
		return -1;
	}
	client.RegMessageFunc(OnMessage);
	client.RegCloseFunc(OnClose);

	client.GetTimerThread().Register(true, 5, std::bind(&Func1, sockId));
	/*client.GetTimerThread().RegisterByMilSec(false, 2500, Func2);
	client.GetTimerThread().RegisterByMilSec(true, 3200, Func3);*/

	client.Run();

	inputThread.join();
	return 0;
}