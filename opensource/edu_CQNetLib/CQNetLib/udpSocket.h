#pragma once

#define WIN32_LEAN_AND_MEAN //#define _WINSOCKAPI_
#include <Windows.h>
#include <winsock2.h>
#include <Ws2tcpip.h>

#include "commonDef.h"
#include "log.h"
#include "thread.h"


namespace CQNetLib
{
#ifndef MAX_UDP
#define MAX_UDP 
#define MAX_UDP_RECVBUFSIZE		1024 * 2
#define MAX_UDP_SENDBUFSIZE		1024 * 2
#define MAX_UDP_SENDSIZE		2048 
#endif

	//�⺻��
	#define DEFAULT_SENDREPEAT		1		//UDP��Ŷ�� �߰��� �ҽǵɼ� �ֱ⶧����.. 			

	class UDPSocket : public Thread
	{
	public:
		UDPSocket()
		{
			m_pCurSendBuf = m_szSendBuf;
			ZeroMemory(m_szIp, MAX_IP_LENGTH);
			m_eventUDP = WSACreateEvent();
			InitSocket();
		}

		virtual ~UDPSocket(void)
		{
			OnClose();
			WSACleanup();
			m_bIsWorkerRun = false;
			SetEvent(m_eventUDP);
			DestroyThread();
		}

		virtual void OnProcess()
		{
			WSANETWORKEVENTS wsaNetworkEvents;
			while (m_bIsWorkerRun)
			{
				//////////////////////////////////////////////////////////////////
				//��û�� Overlapped I/O�۾��� �Ϸ�Ǿ����� �̺�Ʈ�� ��ٸ���.
				UINT32 dwObjIdx = WaitForSingleObject(m_eventUDP, INFINITE);
				
				//���� �߻�
				if (WSA_WAIT_FAILED == dwObjIdx)
				{
					LOG(LOG_ERROR_HIGH, "SYSTEM | cUDPSocket::OnProcess() | WSAWaitForMultipleEvents Failed : %d ",GetLastError());
					break;
				}
				
				int nRet = WSAEnumNetworkEvents(m_socket, m_eventUDP, &wsaNetworkEvents);
				if (SOCKET_ERROR == nRet)
				{
					LOG(LOG_ERROR_HIGH, "SYSTEM | cUDPSocket::OnProcess() | WSAEnumNetworkEvents Failed : %d ",GetLastError());
					break;
				}				
				else if (wsaNetworkEvents.lNetworkEvents & FD_READ) //�����Ͱ� �����ߴٴ� �̺�Ʈ �߻�
				{
					if (0 != wsaNetworkEvents.iErrorCode[FD_READ_BIT])
					{
						LOG(LOG_ERROR_HIGH, "SYSTEM | cUDPSocket::OnProcess() | wsaNetworkEvents.iErrorCode Failed : %d ",GetLastError());
						break;
					}
					OnRecv();
				}
			}
		}

		//���� ���� �Լ�
		virtual bool ProcessPacket(char* pMsg, int nLen, char* szIP, unsigned short usPort) = 0;

		ERROR_CODE Start(unsigned short usPort)
		{
			if (auto result = Bind(usPort); result != ERROR_CODE::none)
			{
				return result;
			}
			
			CreateThread(0);
			Run();

			return ERROR_CODE::none;

		}

		ERROR_CODE InitSocket()
		{
			WSADATA		WsaData;
			int nRet = WSAStartup(MAKEWORD(2, 2), &WsaData);
			if (nRet) 
			{
				return ERROR_CODE::udp_socket_WSAStartup_fail;
			}

			// create listen socket.
			m_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
			if (INVALID_SOCKET == m_socket)
			{
				return ERROR_CODE::udp_socket_create_fail;
			}
			
			return ERROR_CODE::none;
		}

		ERROR_CODE Bind(unsigned short usPort)
		{
			SOCKADDR_IN	si_addr;
			si_addr.sin_family = AF_INET;
			si_addr.sin_port = htons(usPort);
			si_addr.sin_addr.s_addr = htonl(INADDR_ANY);
			int nRet = bind(m_socket, (struct sockaddr*) & si_addr, sizeof(SOCKADDR_IN));
			if (SOCKET_ERROR == nRet)
			{
				return ERROR_CODE::udp_socket_bind_fail;
			}

			//������ �̺�Ʈ�� ������ ���ش�.
			nRet = WSAEventSelect(m_socket, m_eventUDP, FD_READ);
			if (SOCKET_ERROR == nRet)
			{
				return ERROR_CODE::udp_socket_WSAEventSelect_fail;
			}
			
			m_usPort = usPort;
			
			return ERROR_CODE::none;

		}

		bool OnRecv()
		{
			SOCKADDR_IN	si_addr;
			int nAddrLen = sizeof(SOCKADDR_IN);
			int nRecvLen = recvfrom(m_socket, m_szRecvBuf, MAX_UDP_RECVBUFSIZE, 0, (sockaddr*)& si_addr, &nAddrLen);

			if (nRecvLen <= 0)
			{
				return true;
			}

			char clientIP[65] = { 0, };
			inet_ntop(AF_INET, &(si_addr.sin_addr), clientIP, 65 - 1);

			if (ProcessPacket(m_szRecvBuf, nRecvLen, clientIP, ntohs(si_addr.sin_port)) == false)
			{
				return false;
			}

			return true;
		}

		bool OnClose()
		{
			//WSAAsyncSelect(m_socket, NULL, 0, 0);
			shutdown(m_socket, SD_BOTH);
			closesocket(m_socket);
			m_nRestLen = 0;
			m_nSendLen = 0;
			m_pCurSendBuf = m_szSendBuf;
			return true;
		}

		bool SendMsg(char *szIP, unsigned short usPort, BYTE byRepeatCnt = DEFAULT_SENDREPEAT, char* pMsg = nullptr, int nMsgLen = 0)
		{
			SOCKADDR_IN	si_addr;
			int nAddrLen = sizeof(SOCKADDR_IN);
			si_addr.sin_family = AF_INET;//si_addr.sin_port = htons(usPort);//si_addr.sin_addr.s_addr = inet_addr(szIP);
			auto ret = inet_pton(AF_INET, szIP, (void*)& si_addr.sin_addr.s_addr); UNREFERENCED_PARAMETER(ret);
			si_addr.sin_port = htons(usPort);

			int nLen = 0;
			int nSendLen = m_nSendLen;
			char* pSendBuf = m_pCurSendBuf;
			
			if (pMsg != nullptr)
			{
				nSendLen = nMsgLen;
				pSendBuf = pMsg;
			}

			//��Ŷ ������ �־��ش�.
			PTPPACKET_BASE* pUDPBase = (PTPPACKET_BASE*)pSendBuf;
			//������ ���� ��Ŷ�� �ƴ϶��
			if (0 == pUDPBase->SequenceNo)
			{
				pUDPBase->SequenceNo = ++m_dwSequenceNo;
			}

			for (int i = 0; i < byRepeatCnt; ++i)
			{
				nLen = sendto(m_socket, pSendBuf, nSendLen, 0, (sockaddr*)& si_addr, nAddrLen);
			}

			if (-1 == nLen)
			{
				return false;
			}

			return true;
		}

		char* PrepareSendBuffer(int nLen) //nLen��ŭ�� ���۸� ���� �ش�. �� MAX_SENDSIZE���� Ŭ���� ����.
		{
			if (nLen > MAX_UDP_SENDSIZE)
			{
				LOG(LOG_ERROR_NORMAL, "SYSTEN | cUDPSocket::InitSocket() | ���� �޼����� �ʹ� ũ��.size[%d]", nLen);
				return nullptr;
			}

			//���۰� �������� �ͼ� ���̻� �Ҵ��� ���ϸ� �ٽ� �����͸� ������ �ű��.
			if ((m_pCurSendBuf + nLen) >= (m_szSendBuf + MAX_UDP_SENDBUFSIZE))
			{
				m_pCurSendBuf = m_szSendBuf;
			}

			m_nSendLen = nLen;
			return m_pCurSendBuf;
		}

		inline unsigned short GetPort() { return m_usPort; }
		inline char*		  GetIp() { return m_szIp; }
		inline SOCKET		  GetSocket() { return m_socket; }

		inline UINT32		  GetSequenceNo() { return m_dwSequenceNo; }
		inline void			  SetSequenceNo(UINT32 dwSequenceNo) { m_dwSequenceNo = dwSequenceNo; }

	private:
		unsigned short		m_usPort = 0;							//��Ʈ
		char				m_szIp[MAX_IP_LENGTH];				//������
		char				m_szOutStr[1024];
		char				m_szSendBuf[MAX_UDP_SENDBUFSIZE]; // ��� ������� �ʱ�
		char				m_szRecvBuf[MAX_UDP_RECVBUFSIZE]; // ��� ������� �ʱ�
		char*				m_pCurSendBuf;						//���� ���� �����Ͱ� �ִ� ��
		SOCKET				m_socket = INVALID_SOCKET;			//������ ����Ǿ� �ִ� ����
		int					m_nRestLen = 0;			//recv���� �����Ͱ� ó�� ���� �ʰ� �����ִ� ����
		int					m_nSendLen = 0;			//send�� ������

		WSAEVENT			m_eventUDP;			//udp���� ��Ŷ�� ���ų� ��������
		bool				m_bIsWorkerRun = true;		//WorkerThread����

		UINT32				m_dwSequenceNo = 0;		//UDP��Ŷ ����


		SET_NO_COPYING(UDPSocket);
	};


}