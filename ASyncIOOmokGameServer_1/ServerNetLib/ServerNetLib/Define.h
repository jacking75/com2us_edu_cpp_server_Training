#pragma once
#include <string>
#include <WinSock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>

namespace NServerNetLib
{
	struct ServerConfig
	{
		unsigned short Port;
		int BackLogCount;
		int MaxClientCount;
		int ExtraClientCount;

		short MaxClientRecvBufferSize;
		short MaxClientSendBufferSize;

		int MaxRoomCountByLobby;
		int MaxRoomUserCount;

		std::string RedisAddress;
		int RedisPortNum;

		bool IsLoginCheck;

	};

	const int MAX_IP_LEN = 32; // IP ���ڿ� �ִ� ����
	const int MAX_PACKET_BODY_SIZE = 1024; // �ִ� ��Ŷ ���� ũ��
	const int PACKET_DATA_BUFFER_SIZE = 8096;

	enum class PACKET_ID : short
	{
		NTF_SYS_CONNECT_SESSION = 2,
		NTF_SYS_CLOSE_SESSION = 3,
		NTF_SYS_RECV_SESSION=4

	};

	struct RecvPacketInfo
	{
		UINT32 SessionIndex = 0;
		UINT16 PacketId = 0;
		UINT16 PacketBodySize = 0;
		char* pRefData = 0;
	};

	const UINT32 MAX_SOCK_RECVBUF = 256;	// ���� ������ ũ��
	const UINT32 MAX_SOCK_SENDBUF = 4096;	// ���� ������ ũ��
	const UINT64 RE_USE_SESSION_WAIT_TIMESEC = 3;


	enum class IOOperation
	{
		ACCEPT,
		RECV,
		SEND
	};

	//WSAOVERLAPPED����ü�� Ȯ�� ���Ѽ� �ʿ��� ������ �� �־���.
	struct stOverlappedEx
	{
		WSAOVERLAPPED m_wsaOverlapped;		//Overlapped I/O����ü
		WSABUF		m_wsaBuf;				//Overlapped I/O�۾� ����
		IOOperation m_eOperation;			//�۾� ���� ����
		UINT32 SessionIndex = 0;
	};


}
