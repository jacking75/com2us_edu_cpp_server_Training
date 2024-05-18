#include <iostream>

//#include "ConnectionManager.h"
#include "chatServer.h"

#pragma warning(push)
#pragma warning(disable: 4996)
#include <loguru.cpp> //���ο��� Win32 API�� ��Ʈ��ũ �Լ��� �������� �ʵ��� �ϴ� �� ����. �׷��� ��Ʈ��ũ ���̺귯�� �ڿ� ������ �ؾ� �Ѵ�
#pragma warning(pop)

int main(int argc, char* argv[])
{
	loguru::init(argc, argv);
	//loguru::add_file("everything.log", loguru::Append, loguru::Verbosity_MAX);
	LOG_F(INFO, "Hello log file!");

	//void (*CQNetLib::LogFuncPtr)(const int eLogInfoType, const char* szOutputString, ...) = [](const int eLogInfoType, const char* szOutputString, ...) {}
	CQNetLib::LogFuncPtr = [](const int eLogInfoType, const char* szOutputString, ...) {
		va_list	argptr;
		va_start(argptr, szOutputString);

		char szLogMsg[256] = { 0, };
		_vsnprintf_s(szLogMsg, 256, szOutputString, argptr);
		va_end(argptr);

		printf("%s", szLogMsg);
	};
	
	CQNetLib::INITCONFIG InitConfig;
	InitConfig.nServerPort = 32452;
	InitConfig.nRecvBufCnt = 10;
	InitConfig.nRecvBufSize = 1024;
	InitConfig.nProcessPacketCnt = 1000;
	InitConfig.nSendBufCnt = 10;
	InitConfig.nSendBufSize = 1024;
	InitConfig.nWorkerThreadCnt = 2;
	InitConfig.nProcessThreadCnt = 1;


	ChatServer server;
	server.Init(InitConfig, 10);
	server.ServerStart(InitConfig);

	//ConnectionManager connectionMgr;
	//connectionMgr.CreateConnection(InitConfig, 10, &server);
	
	//LOG(LOG_INFO_LOW, "���� ����..");
	std::cout << "Ű�� ������ ����..." << std::endl;
	
	while (true)
	{
		std::string inputCmd;
		std::getline(std::cin, inputCmd);
		break;
		/*if (inputCmd == "kill")
		{
			break;
		}*/
	}

	
	return 0;
}

