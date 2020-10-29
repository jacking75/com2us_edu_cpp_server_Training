#pragma once



namespace NServerNetLib
{
	struct ServerConfig
	{
		unsigned short Port;
		int BackLogCount;

		int MaxClientCount;
		int ExtraClientCount; // �����ϸ� �α��ο��� ¥������ MaxClientCount + �������� �غ��Ѵ�.

		short MaxClientSockOptRecvBufferSize;
		short MaxClientSockOptSendBufferSize;
		short MaxClientRecvBufferSize;
		short MaxClientSendBufferSize;

		bool IsLoginCheck;	// ���� �� Ư�� �ð� �̳��� �α��� �Ϸ� ���� ����

		int MaxLobbyCount;
		int MaxLobbyUserCount;
		int MaxRoomCountByLobby;
		int MaxRoomUserCount;
	};

	const int MAX_IP_LEN = 32; // IP ���ڿ� �ִ� ����
	const int MAX_PACKET_BODY_SIZE = 1024; // �ִ� ��Ŷ ���� ũ��

	enum class PACKET_ID : short
	{
		NTF_SYS_CONNECT_SESSION = 2,
		NTF_SYS_CLOSE_SESSION = 3,
		NTF_SYS_RECV_SESSION=4

	};
	struct RecvPacketInfo
	{
		int SessionIndex = 0;
		short PacketId = 0;
		short PacketBodySize = 0;
		char* pRefData = 0;
	};

#pragma pack(push, 1)
	struct PacketHeader
	{
		short TotalSize;
		short Id;
		unsigned char Reserve;
	};

	const int PACKET_HEADER_SIZE = sizeof(PacketHeader);


	struct PktNtfSysCloseSession : PacketHeader
	{
		int SockFD;
	};
#pragma pack(pop)
}
