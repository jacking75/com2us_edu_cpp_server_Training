#include <WinSock2.h>
#include <Mswsock.h>
#include <Ws2tcpip.h>

#include "ContentsConfig.h"
#include "Exception.h"
#include "OverlappedIOContext.h"
#include "ClientSession.h"
#include "IocpManager.h"
#include "ClientSessionManager.h"
#include "Player.h"

const int CLIENT_BUFSIZE = 65536;


ClientSession::ClientSession() : Session(CLIENT_BUFSIZE, CLIENT_BUFSIZE), mPlayer(this)
{
	memset(&mClientAddr, 0, sizeof(SOCKADDR_IN));
}

ClientSession::~ClientSession()
{
}

void ClientSession::SessionReset()
{
	FastSpinlockGuard criticalSection(mSessionLock);

	mConnected = 0;
	mRefCount = 0;
	memset(&mClientAddr, 0, sizeof(SOCKADDR_IN));

	mRecvBuffer.BufferReset();
	mSendBuffer.BufferReset();
	
	LINGER lingerOption;
	lingerOption.l_onoff = 1;
	lingerOption.l_linger = 0;

	/// no TCP TIME_WAIT
	if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char*)&lingerOption, sizeof(LINGER)))
	{
		printf_s("[DEBUG] setsockopt linger option error: %d\n", GetLastError());
	}
	
	mSocket = WSASocket(AF_INET, SOCK_STREAM, IPPROTO_TCP, NULL, 0, WSA_FLAG_OVERLAPPED);

	m_DisconnectReason = DisconnectReason::DR_NONE;

	mPlayer.PlayerReset();
}

bool ClientSession::PostAccept()
{
	OverlappedAcceptContext* acceptContext = new OverlappedAcceptContext(this);
	DWORD bytes = 0;
	DWORD flags = 0;
	acceptContext->mWsaBuf.len = 0;
	acceptContext->mWsaBuf.buf = nullptr;

	if (FALSE == AcceptEx(*GIocpManager->GetListenSocket(), mSocket, GIocpManager->mAcceptBuf, 0,
		sizeof(SOCKADDR_IN)+16, sizeof(SOCKADDR_IN)+16, &bytes, (LPOVERLAPPED)acceptContext))
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIoContext(acceptContext);
			printf_s("AcceptEx Error : %d\n", GetLastError());

			return false;
		}
	}

	return true;
}

bool ClientSession::AcceptCompletion()
{
	if (1 == InterlockedExchange(&mConnected, 1))
	{
		/// already exists?
		CRASH_ASSERT(false);
		return false;
	}

	bool resultOk = true;
	do 
	{
		if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)GIocpManager->GetListenSocket(), sizeof(SOCKET)))
		{
			printf_s("[DEBUG] SO_UPDATE_ACCEPT_CONTEXT error: %d\n", GetLastError());
			resultOk = false;
			break;
		}

		int opt = 1;
		if (SOCKET_ERROR == setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, (const char*)&opt, sizeof(int)))
		{
			printf_s("[DEBUG] TCP_NODELAY error: %d\n", GetLastError());
			resultOk = false;
			break;
		}

		opt = 0;
		if (SOCKET_ERROR == setsockopt(mSocket, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, sizeof(int)))
		{
			printf_s("[DEBUG] SO_RCVBUF change error: %d\n", GetLastError());
			resultOk = false;
			break;
		}

		int addrlen = sizeof(SOCKADDR_IN);
		if (SOCKET_ERROR == getpeername(mSocket, (SOCKADDR*)&mClientAddr, &addrlen))
		{
			printf_s("[DEBUG] getpeername error: %d\n", GetLastError());
			resultOk = false;
			break;
		}

		HANDLE handle = CreateIoCompletionPort((HANDLE)mSocket, GIocpManager->GetComletionPort(), (ULONG_PTR)this, 0);
		if (handle != GIocpManager->GetComletionPort())
		{
			printf_s("[DEBUG] CreateIoCompletionPort error: %d\n", GetLastError());
			resultOk = false;
			break;
		}

	} while (false);


	if (!resultOk)
	{
		printf_s("[DEBUG][%s] CreateIoCompletionPort error: %d\n", __FUNCTION__, GetLastError());
		return resultOk;
	}

	char clientIP[32] = { 0, };
	inet_ntop(AF_INET, &(mClientAddr.sin_addr), clientIP, 32 - 1);
	printf_s("[DEBUG] Client Connected: IP=%s, PORT=%d\n", clientIP, ntohs(mClientAddr.sin_port));

	if (false == PostRecv())
	{
		printf_s("[DEBUG][%s] PreRecv error: %d\n", __FUNCTION__, GetLastError());
		return false;
	}


	//TEST: ����� ��ġ�� ���� C_LOGIN �ڵ鸵 �� �� �ؾ��ϴ°����� ������ ���� �Ϸ� �������� �׽�Ʈ ����

	//todo: �÷��̾� id�� �������� �÷��̾� ���̺� ��Ȳ�� �°� ������ ���ļ� �ε��ϵ��� 
	static int id = 101;
	++id;
 	//mPlayer.RequestLoad(id);

	printf_s("[DEBUG][%s] Connectd New Session: %I64u\n", __FUNCTION__, mSocket);
	
	return true;
}

void ClientSession::OnDisconnect()
{
	auto dr = GetDisconnectReason();

	char clientIP[32] = { 0, };
	inet_ntop(AF_INET, &(mClientAddr.sin_addr), clientIP, 32 - 1);
	printf_s("[DEBUG] Client Disconnected: Reason=%d IP=%s, PORT=%d \n", dr, clientIP, ntohs(mClientAddr.sin_port));
}

void ClientSession::OnRelease()
{
	GClientSessionManager->ReturnClientSession(this);
}

