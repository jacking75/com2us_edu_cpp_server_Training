#pragma once

#include <winsock2.h>
#include <cstdint>

//#include "loguru.cpp"


namespace CQNetLib
{
//TODO ����� �����ϸ� �� ���ֱ� ///////////////////////////////////////////////////////////
//#define MAX_RECVBUFCNT				100				//iocp recv packet pool ���� ���۰���,,
//#define MAX_SENDBUFCNT				100				//iocp send packet pool ���� ���۰���,,
#define MAX_QUEUESIZE				10000           //default queue size in cQueue class
#define MAX_QUEUESENDSIZE			1000
#define MAX_PBUFSIZE				4096			//PacketPool���� ���� �Ѱ��� size
#define MAX_VPBUFSIZE				1024 * 40		//���� ���� size
#define MAX_RINGBUFSIZE				1024 * 100		//Ringbuffer size
#define DEFUALT_BUFCOUNT			10				//���� ���� ����.
//#define MAX_LUA_FILENAME			1024			//��ƿ��� ���Ǵ� ������ ����
#define PACKET_SIZE_LENGTH			2
#define PACKET_TYPE_LENGTH			2
#define MAX_IP_LENGTH				65
#define MAX_PROCESS_THREAD			1
#define MAX_WORKER_THREAD			10
////////////////////////////////////////////////////////////////////////////////////////

	//�޸� ����
	#define SAFE_DELETE(p)       { if(p) { delete (p);     (p)=nullptr; } }
	#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p);   (p)=nullptr; } }
	#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=nullptr; } }
	#define SAFE_RELEASE_ST(p)      { if(p) { (p)->Release(); } }


	enum eServerNetState
	{
		SERVER_INIT,
		SERVER_START,
		SERVER_STOP,
		SERVER_CLOSE
	};


	enum eOperationType
	{
		OP_NONE,
		//Work IOCP operation
		OP_SEND,
		OP_RECV,
		OP_ACCEPT,
		//Process IOCP operation
		OP_CLOSE,		//���� ����ó��
		OP_RECVPACKET,	//���� �� ��Ŷ ó��
		OP_SYSTEM		//�ý��� �޽��� ó��
	};

	enum eConnectionType
	{
		CT_CLIENT = 0,	// Ŭ���̾�Ʈ
		CT_SERVER = 1,		
	};


	enum class ERROR_CODE
	{
		none = 0,

		WSAStartup_faile = 1,
		create_WorkerIOCP_faile = 2,
		create_ProcessIOCP_faile = 3,
		create_WorkerThread_faile = 4,
		create_ProcessThread_faile = 5,
		create_listen_socket_fail = 6,
		listen_socket_set_option_fail = 7,
		listen_socket_bind_fail = 8,
		listen_fail = 9,
		listen_socket_bind_IOCP_fail = 10,
		create_server_listen_socket_fail = 11,
		server_listen_socket_set_option_fail = 12,
		server_listen_socket_bind_fail = 14,
		server_listen_fail = 15,
		server_listen_socket_bind_IOCP_fail = 16,
		udp_socket_WSAStartup_fail = 21,
		udp_socket_create_fail = 22,
		udp_socket_bind_fail = 23,
		udp_socket_WSAEventSelect_fail = 24,

		session_connectTo_create_socket_fail = 31,
		session_connectTo_remote_fail = 32,
		session_connectTo_bind_iocp_fail = 33,
		session_create_socket_fail = 36,
		session_post_accept_socket_fail = 37,
		session_recvPost_not_connected = 41,
		session_recvPost_null_bufer = 42,
		session_recvPost_WSARecv_fail = 43,
		session_sendPost_null_buffer = 46,
		session_sendPost_WSASend_fail = 47,
	};

	
	/////////////////////////////////////////////////
	//Connection������ ���� config����ü
	struct INITCONFIG
	{
		INT32 				nIndex = 0;						//Connection index
		SOCKET			sockListener = INVALID_SOCKET;				//Listen Socket
		INT32 				nRecvBufCnt;
		INT32 				nSendBufCnt = 0;
		INT32 				nRecvBufSize = 0;
		INT32 				nSendBufSize = 0;
		INT32 				nProcessPacketCnt = 0;			//�ִ� ó�� ��Ŷ�� ����
		INT32 				nConnectionPool = 0;			//������ ����� ������ �ִ´�.
		INT32 				nClientPort = 0;
		INT32 				nServerPort = 0;
		INT32 				nWorkerThreadCnt = 0;			//i/o ó���� ���� thread ����
		INT32 				nProcessThreadCnt = 0;			//��Ŷó���� ���� thread ����
		INT32 				nLogInfoLevelFile = 0;			//���� �α� ���� ����
		INT32 				nLogErrorLevelFile = 0;			//���� �α� ���� ����
		INT32 				nLogInfoLevelWindow = 0;		//������ �α� ���� ����
		INT32 				nLogErrorLevelWindow = 0;		//������ �α� ���� ����
		INT32 				nLogInfoLevelDebugWnd = 0;		//�����â �α� ���� ����
		INT32 				nLogErrorLevelDebugWnd = 0;		//�����â �α� ���� ����
		char			szIP[MAX_IP_LENGTH] = { 0, };		//���� ������
		eConnectionType	ConnectionType;				//������ ���� ����		
	};

	typedef struct _OVERLAPPED_EX
	{
		WSAOVERLAPPED			s_Overlapped;
		WSABUF					s_WsaBuf;
		INT32 						s_nTotalBytes;
		UINT32					s_dwRemain;
		char*					s_lpSocketMsg;	// Send/Recv Buffer.
		eOperationType			s_eOperation;
		void*					s_lpConnection;
		_OVERLAPPED_EX(void* lpConnection)
		{
			ZeroMemory(this, sizeof(OVERLAPPED_EX));
			s_lpConnection = lpConnection;
		}
	}OVERLAPPED_EX, *LPOVERLAPPED_EX;
	   
	typedef struct _PROCESSPACKET
	{
		eOperationType	s_eOperationType;
		WPARAM			s_wParam;
		LPARAM			s_lParam;
		
		_PROCESSPACKET()
		{
			Init();
		}
		void Init()
		{
			ZeroMemory(this, sizeof(PROCESSPACKET));
		}

	}PROCESSPACKET, *LPPROCESSPACKET;
	

	// default ��Ŷ ���. �̰� �̿ܿ� �ٸ� ������ ��� ����
	//��Ŷ �⺻ ���
	struct PACKET_BASE
	{
		unsigned short usLength;
		unsigned short usType;
	};

	struct P2P_PACKET_BASE
	{
		unsigned short Length;
		unsigned short Type;
	};

	struct PTPPACKET_BASE : public P2P_PACKET_BASE
	{
		UINT32		   SequenceNo;		//UDP��Ŷ�� ���� ����
		UINT32		   PKey;			//��Ŷ�� ���� ������ PKey
		UINT32		   OtherPKey;		//��Ŷ�� ���� ������ PKey

		PTPPACKET_BASE()
		{
			SequenceNo = 0;
			PKey = 0;
			OtherPKey = 0;
		}
	};
	



////////////////////////////////////////////////////////////////////////////////////////////
//�������� ���̴� ��ũ��

//��ü ī�� ���� ��ũ��
#define SET_NO_COPYING( a ) a(const a &rhs); a &operator=(const a &rhs);

//TODO ���ø����� �ٲٱ�
//�޸� ������ ��ũ��, ���ϰ� ����. ������ ���� ���� �� ReleaseSendPacket���� Ŭ���� �ؾ� �ȴ�.!!!
#define PREPARE_SENDPACKET( stPacket , typePacket , namePacket ) \
	stPacket* namePacket = (stPacket*)PrepareSendPacket( sizeof(stPacket) ); \
	if( nullptr == namePacket ) \
	{\
		return false;\
	}\
	namePacket->usType = typePacket;

//TODO ���ø����� �ٲٱ�
//�޸� ������ ��ũ��, ���ϰ� ����. ������ ���� ���� �� ReleaseSendPacket���� Ŭ���� �ؾ� �ȴ�.!!!
#define PREPARE_SENDPACKET_CONN( sendConn , stPacket , typePacket , namePacket ) \
	stPacket* namePacket = (stPacket*)sendConn->PrepareSendPacket( sizeof(stPacket) ); \
	if( nullptr == namePacket ) \
	{\
		return false;\
	}\
	namePacket->usType = typePacket;


//�޸� ������ ��ũ�� ����, ���ϰ� ����
// ������ ���� ���� �� ReleaseSendPacket���� Ŭ���� �ؾ� �ȴ�.!!!
#define PREPARE_SENDPACKET_V( stPacket , nVPacketSize, typePacket , namePacket ) \
	stPacket* namePacket = (stPacket*)PrepareSendPacket( sizeof(stPacket) + nVPacketSize ); \
	if( nullptr == namePacket ) \
	{\
		return false;\
	}\
	namePacket->usType = typePacket;

//�޸� ���ͼ� Send���� �ϴ� ��ũ��, ���ϰ� ����
#define SEND_BUFFER( dataPacket , sizePacket ) \
	char* pSendBuffer = PrepareSendPacket( sizePacket ); \
	if( nullptr == pSendBuffer ) \
	{\
		unsigned short usType;\
		CopyMemory( &usType , dataPacket + PACKET_SIZE_LENGTH , PACKET_TYPE_LENGTH );\
		return false;\
	}\
	INT32 nPacketLen = sizePacket;\
	CopyMemory( pSendBuffer , &nPacketLen , PACKET_SIZE_LENGTH );\
	CopyMemory( pSendBuffer + PACKET_SIZE_LENGTH , dataPacket + PACKET_SIZE_LENGTH , sizePacket - PACKET_SIZE_LENGTH );\
	SendPost( nPacketLen );



	enum class LogLevel
	{
		Trace = 0,
		Debug,
		Info,
		Warning,
		Error,
		Fatal
	};

	inline char g_LogLevelStr[6][8] = {"Trace", "Debug", "Info", "Warning", "Error", "Fatal" };
	inline void (*LogFuncPtr) (const int, const char* szOutputString, ...);
}