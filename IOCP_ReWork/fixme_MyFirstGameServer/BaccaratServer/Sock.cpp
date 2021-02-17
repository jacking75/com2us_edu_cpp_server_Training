// Sock.cpp: implementation of the CSock class.
//
//////////////////////////////////////////////////////////////////////

#include <process.h>
#include <winsock2.h>
#include <mswsock.h>
#include "protocol_baccarat.h"
#include "server.h"
#include "serverprocess.h"
#include "room.h"
#include "game.h"
#include "database.h"
#include "packetcoder.h"
#include "Serverutil.h"
#include "Sock.h"



int InitSocketIO()
{
	// ������ �ʱ�ȭ�� ���� ������ �����.
	if( !InitWinSock() ) return 0;	

	// IOCP ��Ʈ�� �����.
	if( CreateServerIOCP() == E_FAIL ) return 0;

	// Ŭ���̾�Ʈ ���ؽ�Ʈ�� �ִ� ���� �ο� ��ŭ �����.
	ServerContext.sc = new SOCKETCONTEXT[ ServerContext.iMaxUserNum ];
	if( ServerContext.sc == NULL ) return 0;

	// ��ũ ����Ʈ�� ��带 �ִ� ���� �ο� ��ŭ �����.
	ServerContext.pn = new OBJECTNODE[ ServerContext.iMaxUserNum ];
	if( ServerContext.pn == NULL ) return 0;

	// Ŭ���̾�Ʈ ���ؽ�Ʈ�� ������� �ʱ�ȭ �Ѵ�.
	if( !InitSocketContext( ServerContext.iMaxUserNum ) ) return 0;

	// ���ϰ� ���õ� �����带 �����ϰ� �����Ѵ�.
	if( !CreateSocketThread() ) return 0;
	
	return 1;
}


int InitWinSock()
{
	WSADATA			wd = { 0 };
	SOCKADDR_IN		addr;
	int				dummy;
	
	dummy = WSAStartup( MAKEWORD( 2, 2 ), &wd );
	if( dummy != 0 ) return E_FAIL;

	ServerContext.sockListener = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if( ServerContext.sockListener == INVALID_SOCKET )	return E_FAIL;
	
	dummy = 1;
	if( setsockopt( ServerContext.sockListener, SOL_SOCKET, SO_REUSEADDR, (char*)&dummy, sizeof(dummy) ) < 0 )
		return 0;

	addr.sin_family			= AF_INET;
	addr.sin_addr.s_addr	= INADDR_ANY;
	addr.sin_port			= htons( (short)ServerContext.iPortNum );
	dummy					= bind( ServerContext.sockListener, (sockaddr*)&addr, sizeof(addr) );
	if( dummy == SOCKET_ERROR ) return 0;

	dummy	= listen( ServerContext.sockListener, 512 );
	if( dummy == SOCKET_ERROR ) return 0;

	return 1;
}


int CreateServerIOCP()
{
	// Ŭ���̾�Ʈ�� ��Ŷ �۽ſ� ���õ� IOCP
	ServerContext.hIocpWorkTcp = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );
	if( ServerContext.hIocpWorkTcp == NULL ) return 0;

	// Ŭ���̾�Ʈ�� ���Ӱ� ���õ� IOCP
	ServerContext.hIocpAcpt = CreateIoCompletionPort( INVALID_HANDLE_VALUE, NULL, 0, 1 );
	if( ServerContext.hIocpAcpt == NULL ) return 0;
	
	CreateIoCompletionPort( (HANDLE)ServerContext.sockListener, ServerContext.hIocpAcpt, 0, 1 );

	return 1;
}


// Ŭ���̾�Ʈ�� ���� �� ������ �������� Ŭ���̾�Ʈ ���ؽ�Ʈ�� �ʱ�ȭ �Ѵ�.
int ReInitSocketContext(LPSOCKETCONTEXT lpSockContext)
{
	int				dummy;
	DWORD			dwReceived;
	struct linger	li;

	EnterCriticalSection( &lpSockContext->csSTcp );

	
	lpSockContext->iSTRestCnt = 0;

#ifdef _LOGLEVEL1_
	ServerUtil.ConsoleOutput( "SocketContextReInit(%d)\n", lpSockContext->index );
#endif
	
	li.l_onoff = 1;
	li.l_linger = 0;
	if( setsockopt( lpSockContext->sockTcp, SOL_SOCKET, SO_LINGER, (char*)&li, sizeof(li) ) 
			== SOCKET_ERROR )
	{
#ifdef _LOGFILELEVEL1_
		ServerUtil.ReInitSocketContextErrLog( WSAGetLastError() );
#endif
	}
	
	closesocket( lpSockContext->sockTcp );
	
	lpSockContext->cpSTBegin = lpSockContext->cpSTEnd = lpSockContext->cSendTcpRingBuf;
	lpSockContext->cpRTMark = lpSockContext->cpRTBegin = lpSockContext->cpRTEnd = lpSockContext->cRecvTcpRingBuf;

	InitUserInfo( lpSockContext );
	
	lpSockContext->sockTcp = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
	if( lpSockContext->sockTcp == INVALID_SOCKET ) return 0;

	dummy = AcceptEx( ServerContext.sockListener, lpSockContext->sockTcp, lpSockContext->cpRTEnd,
					MAXRECVPACKSIZE, sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
					&dwReceived, (OVERLAPPED*)&lpSockContext->eovRecvTcp );
	if( dummy == FALSE && GetLastError() != ERROR_IO_PENDING ) return 0;
	
	int	npacksize = 0;
	setsockopt(lpSockContext->sockTcp, SOL_SOCKET, SO_SNDBUF, (const char*)&npacksize, sizeof(int));
	setsockopt(lpSockContext->sockTcp, SOL_SOCKET, SO_RCVBUF, (const char*)&npacksize, sizeof(int));

	LeaveCriticalSection( &lpSockContext->csSTcp );
	
	return 1;
}


// Ŭ���̾�Ʈ�� �������� �ʱ�ȭ �Ѵ�.
void InitUserInfo( LPSOCKETCONTEXT lpSockContext )
{
	memset( lpSockContext->cID, 0, MAXIDLENGTH );
	lpSockContext->idLen = 0;
	
	memset( lpSockContext->UserInfo.NicName, 0, MAXIDLENGTH );
	lpSockContext->UserInfo.nicLen = 0;
	
	memset( lpSockContext->cPWD, 0, MAXPWDLENGTH );
	lpSockContext->pwdLen = 0;

	memset (lpSockContext->AvatarInfo, 0, sizeof(short)*MAXAVATARLAYER );
	lpSockContext->AvatarType = 0;

	lpSockContext->bAgency		= FALSE;
	lpSockContext->cState		= GAME_CLOSE;
	lpSockContext->ibetMoney	 = 0;
	
	lpSockContext->bCanPrivateItem = FALSE;
	lpSockContext->bCanJumpItem	   = FALSE;	
	
	lpSockContext->bALLChat		= TRUE;
	lpSockContext->bAllInvite	= TRUE;

	lpSockContext->iUserDBIndex	= NOTLOGINUSER;
	lpSockContext->cPosition    = NOTLINKED;

	lpSockContext->iProcess  = DEFAULTPROCESS;
	lpSockContext->iChannel  = NOTLINKED;
	lpSockContext->iRoom	 = NOTLINKED;
	lpSockContext->bRequsetClose = FALSE;
}


// Ŭ���̾�Ʈ ���ؽ�Ʈ�� �ʱ�ȭ �Ѵ�.
int InitSocketContext(int maxUser)
{
	int			npacksize = 0;
	int			dummy;
	DWORD		dwReceved;

	LPSOCKETCONTEXT		sc = ServerContext.sc;

	ZeroMemory( sc, sizeof( SOCKETCONTEXT ) * maxUser );

	for( int iN=0; iN < maxUser; ++iN)
	{
		sc[ iN ].index = iN;
		sc[ iN ].iSTRestCnt = 0;
		sc[ iN ].eovSendTcp.mode = SENDEOVTCP;
		sc[ iN ].eovRecvTcp.mode = RECVEOVTCP;
		sc[ iN ].cpSTBegin = sc[ iN ].cpSTEnd = sc[ iN ].cSendTcpRingBuf;
		sc[ iN ].cpRTMark = sc[ iN ].cpRTBegin = sc[ iN ].cpRTEnd = sc[ iN ].cRecvTcpRingBuf;
		sc[ iN ].sockTcp = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0,WSA_FLAG_OVERLAPPED );

		if( sc[ iN ].sockTcp == INVALID_SOCKET ) return 0;

		dummy = AcceptEx( ServerContext.sockListener, sc[ iN ].sockTcp, sc[ iN ].cpRTEnd,
						MAXRECVPACKSIZE, sizeof( sockaddr_in ) + 16, sizeof( sockaddr_in ) +
						16, &dwReceved, ( OVERLAPPED* )&sc[ iN ].eovRecvTcp );

		setsockopt(sc[ iN ].sockTcp, SOL_SOCKET, SO_SNDBUF, (const char*)&npacksize, sizeof(int));
		setsockopt(sc[ iN ].sockTcp, SOL_SOCKET, SO_RCVBUF, (const char*)&npacksize, sizeof(int));

		InitializeCriticalSection( & sc[ iN ].csSTcp );
		
		InitUserInfo( &sc[ iN ] );
	}

	return 1;
}


int CreateSocketThread()
{
	int		dummy;
	HANDLE	hThread;

	hThread = (HANDLE)_beginthreadex( NULL, 0, AcceptTProc, NULL, 0, (unsigned int*)&dummy );
	if( hThread == NULL ) return 0;

	hThread = (HANDLE)_beginthreadex( NULL, 0, InWorkerTcpTProc, NULL, 0,
											(unsigned int*)&dummy );
	if( hThread== NULL ) return 0;
		
	return 1;
}



unsigned int __stdcall AcceptTProc(LPVOID pvParam)
{
	DWORD			dummy, dwTransfrred = 0;
	LPEOVERLAPPED	lpEov;
	LPSOCKETCONTEXT	lpSockContext;
	BOOL			bResult;
	sockaddr_in		*pLocal, *pRemote;
	int				localLen, remoteLen;
	short			type, bodysize;

	while( TRUE )
	{
		bResult = GetQueuedCompletionStatus( ServerContext.hIocpAcpt, &dwTransfrred, 
										(PULONG_PTR)&dummy, (LPOVERLAPPED*)&lpEov, INFINITE );
	
		if( ServerContext.bThreadStop == FALSE && lpEov != NULL && dwTransfrred < HEADERSIZE )
		{
			lpSockContext = ( LPSOCKETCONTEXT )lpEov;
			ReInitSocketContext( lpSockContext );
		}
		else if( lpEov != NULL && dwTransfrred >= HEADERSIZE )
		{
			lpSockContext = ( LPSOCKETCONTEXT )lpEov;

			lpSockContext->cState = GAME_OFF;

			GetAcceptExSockaddrs( lpSockContext->cpRTEnd, MAXRECVPACKSIZE, sizeof(sockaddr_in) + 16,
								sizeof( sockaddr_in ) + 16, ( sockaddr** )&pLocal, &localLen,
								( sockaddr** )&pRemote, &remoteLen );

			CopyMemory( &lpSockContext->remoteAddr, pRemote, sizeof( sockaddr_in ) );

			RecvTcpBufEnqueue( lpSockContext, dwTransfrred );

			CreateIoCompletionPort( (HANDLE)lpSockContext->sockTcp, ServerContext.hIocpWorkTcp,
									(DWORD)lpSockContext, 1 );
			
			PostTcpRecv( lpSockContext );

			// �α��� ��Ŷ���� Ȯ���ϰ� �ε��� ��Ŷ�� �ƴϸ� ������ ���� ��Ų��.
			CopyMemory( &type, lpSockContext->cRecvTcpRingBuf+2, sizeof(short) );
			if( type != REQUEST_LOGIN ) 
			{
				ReInitSocketContext( lpSockContext );
				continue;
			}

			// �α��� ��Ŷ�� ������ ũ�⸦ üũ�Ѵ�.
			CopyMemory( &bodysize, lpSockContext->cRecvTcpRingBuf, sizeof(short) );
			if( bodysize == 0 )
			{
				ReInitSocketContext( lpSockContext );
				continue;
			}

			GameBufEnqueueTcp( lpSockContext );
		}
		
	}

	return 1;
}


unsigned int __stdcall InWorkerTcpTProc(LPVOID pvParam)
{
	DWORD			dwTransferred;
	LPEOVERLAPPED	lpEov;
	LPSOCKETCONTEXT	lpSockContext;
	BOOL			bResult;
	
	while( TRUE )
	{
		bResult = GetQueuedCompletionStatus( ServerContext.hIocpWorkTcp, &dwTransferred,
					(PULONG_PTR)&lpSockContext, (LPOVERLAPPED*)&lpEov, INFINITE );
		if( lpEov != NULL )
		{
			// ������ ������.
			if( dwTransferred == 0 )
			{
				if( lpSockContext->cState != GAME_CLOSE ) 
				{
					UserKickOrAgency( lpSockContext );
				}
			}
			else
			{
				if( lpSockContext->cState != GAME_CLOSE && lpEov->mode == RECVEOVTCP )
				{
					// ��Ŷ�� ���� �ð��� �Է��Ѵ�.
					time( &lpSockContext->RecvTime );

					RecvTcpBufEnqueue( lpSockContext, dwTransferred );

					GameBufEnqueueTcp( lpSockContext );

					PostTcpRecv( lpSockContext );
				}
				//else if( lpEov->mode == SENDEOVTCP )
				//{
					//PostTcpSendRest( lpSockContext, dwTransferred );
				//}
			}
		}
	}

	return 1;
}


void RecvTcpBufEnqueue(LPSOCKETCONTEXT lpSockContext, int iPacketSize)
{
	int			iExtra;

	// ���� ��Ŷ�� �� ���ۺ��� ũ�ٸ� �̰� ������ �����ϰ� ó������ �ʴ´�.
	if( iPacketSize > RINGBUFSIZE ) return;

	iExtra = lpSockContext->cpRTEnd + iPacketSize - lpSockContext->cRecvTcpRingBuf - RINGBUFSIZE;
	
	if( iExtra >= 0 )
	{
		CopyMemory( lpSockContext->cRecvTcpRingBuf, lpSockContext->cRecvTcpRingBuf +
											RINGBUFSIZE, iExtra );
		lpSockContext->cpRTEnd = lpSockContext->cRecvTcpRingBuf + iExtra;
	}
	else
	{
		lpSockContext->cpRTEnd += iPacketSize;
	}

#ifdef _LOGLEVEL1_
	ServerUtil.ConsoleOutput( "RecvTcpbufenqueue : %d, %d bytes\n", lpSockContext->index, iPacketSize);
#endif

}


void GameBufEnqueueTcp(LPSOCKETCONTEXT lpSockContext)
{
	short			iBodySize;
	int				iRestSize, iExtra;

	iRestSize = lpSockContext->cpRTEnd - lpSockContext->cpRTMark;
	if( iRestSize < 0 ) iRestSize += RINGBUFSIZE;

	// ����� ũ�⺸�� ��Ŷ�� ������ �ǵ�����.
	if( iRestSize < HEADERSIZE ) return;
	
	while( TRUE )
	{
		iExtra = lpSockContext->cRecvTcpRingBuf + RINGBUFSIZE - lpSockContext->cpRTMark;

		// ��Ŷ �ٵ� �����ϴ� short���� ���� 1����Ʈ�� ���۳��� �����Ƿ� ���ʿ��� 1����Ʈ �����´�.
		if( iExtra < sizeof(short) )
		{
			*(lpSockContext->cRecvTcpRingBuf + RINGBUFSIZE) = *lpSockContext->cRecvTcpRingBuf;
		}
		
		// �ٵ��� ũ�⸦ ���´�.
		CopyMemory( &iBodySize, lpSockContext->cpRTMark, sizeof( short ) );

		// ������ ��Ŷ�� ���� ���޴ٸ� �� ��ƾ�� ��� ���� �� �޵��� �Ѵ�.
		if( iRestSize < iBodySize + HEADERSIZE ) return;
		
		lpSockContext->cpRTMark += iBodySize + HEADERSIZE;

		if( lpSockContext->cpRTMark >= lpSockContext->cRecvTcpRingBuf + RINGBUFSIZE )
			lpSockContext->cpRTMark -= RINGBUFSIZE;
		
		iRestSize -= iBodySize + HEADERSIZE;

		ServerContext.ps[ lpSockContext->iProcess ].GameBufEnqueue( lpSockContext );

		if( iRestSize < HEADERSIZE ) return;
	}
}


void PostTcpRecv(LPSOCKETCONTEXT lpSockContext)
{
	WSABUF			wsaBuf;
	DWORD			dwReceived, dwFlags;
	int				iResult;

	dwFlags		= 0;
	wsaBuf.buf	= lpSockContext->cpRTEnd;
	wsaBuf.len	= MAXRECVPACKSIZE;

	iResult = WSARecv( lpSockContext->sockTcp, &wsaBuf, 1, &dwReceived, &dwFlags,
				(OVERLAPPED*)&lpSockContext->eovRecvTcp, NULL );

	if( iResult == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING )
	{
#ifdef _LOGLEVEL1_
		ServerUtil.ConsoleOutput( "----- PostTcpRecv �Լ� ���� : %d, %d\n", lpSockContext->index,
									WSAGetLastError() );
#endif
#ifdef _LOGFILELEVEL1_
		ServerUtil.PostTcpRecvErrLog( lpSockContext->cID, WSAGetLastError() );
#endif
	}
}



void PostTcpSendRest(LPSOCKETCONTEXT lpSockContext, int iTransferred)
{
	WSABUF			wsaBuf;
	DWORD			dwSent;
	int				iResult, iRestSize, iExtra;

	// ������ ������ ��ġ�� �ٲپ� �ش�.
	iExtra = lpSockContext->cpSTBegin + iTransferred - lpSockContext->cSendTcpRingBuf 
													  - RINGBUFSIZE;
	if( iExtra >= 0 )
	{
		lpSockContext->cpSTBegin = lpSockContext->cSendTcpRingBuf + iExtra;
	}
	else
	{
		lpSockContext->cpSTBegin += iTransferred;
	}
	
	EnterCriticalSection( &lpSockContext->csSTcp );
	// Ŭ���̾�Ʈ���� ������ �ϴ� �ѷ��� ���۵� �縸ŭ ���δ�.
	lpSockContext->iSTRestCnt -= iTransferred;
	iRestSize = lpSockContext->iSTRestCnt;
	LeaveCriticalSection( &lpSockContext->csSTcp );
	
#ifdef _LOGLEVEL1_
	ServerUtil.ConsoleOutput( "SendTcpBufDequeue : (�ε���)%d, (������ ��)%d, (���� ������ġ)%X, (���� �� ��ġ)%X\n",
						lpSockContext->index, iTransferred, lpSockContext->cpSTBegin, lpSockContext->cpSTEnd );
#endif

	if( iRestSize > 0 )
	{
		//�߰��� �����ؾ� �� ���� ���� ����.
		
		// ������ �ѹ��� �ִ��� ���� ���� �� �� �ְ� �Ѵ�.
		if( iRestSize > MAXSENDPACKSIZE ) iRestSize = MAXSENDPACKSIZE;

		iExtra = lpSockContext->cSendTcpRingBuf + RINGBUFSIZE - lpSockContext->cpSTBegin;
		
		// ���۰� �� ���� ���Ҵٸ� �������� �ڷ� �����ؿ´�.
		if( iExtra < iRestSize )
		{
			CopyMemory( lpSockContext->cSendTcpRingBuf+RINGBUFSIZE, 
						lpSockContext->cSendTcpRingBuf, iRestSize - iExtra );
		}
		
		wsaBuf.buf = lpSockContext->cpSTBegin;
		wsaBuf.len = iRestSize;

		iResult = WSASend( lpSockContext->sockTcp, &wsaBuf, 1, &dwSent, 0,
						(OVERLAPPED*)&lpSockContext->eovRecvTcp, NULL );
		if( iResult == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING )
		{
#ifdef _LOGLEVEL1_
			ServerUtil.ConsoleOutput( "--- PostTcpSendRest error : %d, %d \n",
				lpSockContext->index, WSAGetLastError() );
#endif
#ifdef _LOGFILELEVEL1_
			ServerUtil.PostTcpSendRestErrLog( lpSockContext->cID, WSAGetLastError() );
#endif
		}

#ifdef _LOGLEVEL1_
		ServerUtil.ConsoleOutput( "RestTCP##SEND##() : (idx)%d, (iRestSize)%d, (iExtra)%d, (iTransferred)%d, (cpSTBegin)%X, (cpSTEnd)%X\n",
		lpSockContext->index, iRestSize, iExtra, iTransferred, lpSockContext->cpSTBegin, lpSockContext->cpSTEnd );
#endif

	}
}


void MoveSendRingBufEndPosition( LPSOCKETCONTEXT lpSockContext, int iPacketSize )
{
	int		iExtra;
	LPSOCKETCONTEXT& SockContext = lpSockContext;

	iExtra = SockContext->cpSTEnd + iPacketSize - SockContext->cSendTcpRingBuf -
														RINGBUFSIZE;

	if( iExtra >= 0 )
	{
		CopyMemory( SockContext->cSendTcpRingBuf, SockContext->cSendTcpRingBuf
													+ RINGBUFSIZE, iExtra );
		SockContext->cpSTEnd = SockContext->cSendTcpRingBuf + iExtra;
	}
	else
	{
		SockContext->cpSTEnd += iPacketSize;
	}
}


void PostTcpSend( int iSockNum, int iSockAddr[], char* cpPacket, int iPacketSize, BOOL bClose )
{
	LPSOCKETCONTEXT		lpSockContext;
	int					iSendNow;
	

	lpSockContext = ( LPSOCKETCONTEXT )( iSockAddr[ 0 ] );
	
	// ������ ������ �������� ��Ŷ�� ������ �ʵ��� �Ѵ�.
	if( lpSockContext->bAgency == TRUE || lpSockContext->cState == GAME_CLOSE || 
																		lpSockContext->idLen == 0 )
		return;
	
	
	//��Ŷ�� ���� �������ؽ�Ʈ�� �����ۿ� �����Ѵ�.
	CopyMemory( lpSockContext->cpSTEnd, cpPacket, iPacketSize );

	MoveSendRingBufEndPosition( lpSockContext, iPacketSize );
	
	EnterCriticalSection( &lpSockContext->csSTcp );
	lpSockContext->iSTRestCnt += iPacketSize;
	iSendNow = ( lpSockContext->iSTRestCnt == iPacketSize )? TRUE : FALSE;
	LeaveCriticalSection( &lpSockContext->csSTcp );

#ifdef _LOGLEVEL1_
	ServerUtil.ConsoleOutput( "SendTcpBufEnqueue : (idx)%d, (iPacketSize)%d, (cpSTBegin)%X, (cpSTEnd)%X\n",
				lpSockContext->index, iPacketSize, lpSockContext->cpSTBegin, lpSockContext->cpSTEnd );
#endif

	if( iSendNow )
	{
		NowTcpSend( lpSockContext, 1, iPacketSize );
		
		if( bClose == SOCKETCLOSE )
		{
			EnqueueClose( lpSockContext );
		
		}
	}
	else
	{

		if( lpSockContext->iSTRestCnt > RINGBUFSIZE )
		{
			// �� ��Ȳ�� �������� ���۸� �۰� �Ҵ� �ߴ��� �ƴϸ� ��ŷ�� �ǽ� �� �� �ִ�.
#ifdef _LOGLEVEL1_
			ServerUtil.ConsoleOutput( "---tcp sendbuf overflow!(%d) : %d ---\n",
				lpSockContext->index, lpSockContext->iSTRestCnt );
#endif
#ifdef _LOGFILELEVEL1_
			ServerUtil.PostTcpSendErrLog( lpSockContext->cID, WSAGetLastError() );
#endif
		}
	}
	
}


void NowTcpSend( LPSOCKETCONTEXT lpSockContext, int iN, int iPacketSize )
{
	WSABUF  wsaBuf;
	DWORD   dwSent;
	int		iResult;

	wsaBuf.buf = lpSockContext->cpSTBegin;
	wsaBuf.len = iPacketSize;
	iResult = WSASend( lpSockContext->sockTcp, &wsaBuf, 1, &dwSent, 0, 
							(OVERLAPPED*)&lpSockContext->eovSendTcp, NULL );
			
	if( iResult == SOCKET_ERROR && WSAGetLastError() != ERROR_IO_PENDING )
	{
#ifdef _LOGLEVEL1_
		ServerUtil.ConsoleOutput( "--- PostTcpSend error : %d, %d\n",
							lpSockContext->index, WSAGetLastError() );
#endif
#ifdef _LOGFILELEVEL1_
		ServerUtil.NowTcpSendErrLog( lpSockContext->cID, WSAGetLastError() );
#endif
	}

#ifdef _LOGLEVEL1_
	ServerUtil.ConsoleOutput( " PostTcpSend(%d) : (idx)%d, (iPacketSize)%d, (cpSTBegin)%X, (cpSTEnd)%X\n",
			iN, lpSockContext->index, iPacketSize, lpSockContext->cpSTBegin, lpSockContext->cpSTEnd );
#endif
	
	PostTcpSendRest( lpSockContext, dwSent );
}


void PostTcpSend( int iBegin, char* cpPacket, int iPacketSize )
{
	LPSOCKETCONTEXT		lpSockContext;
	int					iSendNow;
	int					iNext, dummy, j;

	iNext = iBegin;
	j = 0;	
	while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
	{
		j++;
		lpSockContext = &ServerContext.sc[ iNext ];

		// ������ ���°� AGENCY��� ��Ŷ�� ������ �ʴ´�.
		if( lpSockContext->bAgency == TRUE || lpSockContext->cState == GAME_CLOSE ||
																	lpSockContext->idLen == 0 )
		{
			iNext = ServerContext.pn[ iNext ].next;
			continue;
		}

		
		CopyMemory( lpSockContext->cpSTEnd, cpPacket, iPacketSize );
		
		MoveSendRingBufEndPosition( lpSockContext, iPacketSize );

		EnterCriticalSection( &lpSockContext->csSTcp );
		lpSockContext->iSTRestCnt += iPacketSize;
		iSendNow = ( lpSockContext->iSTRestCnt == iPacketSize ) ? TRUE : FALSE;
		LeaveCriticalSection( &lpSockContext->csSTcp );

#ifdef _LOGLEVEL1_
		ServerUtil.ConsoleOutput( "SendTcpBufEnqueue(%d) : (idx)%d, (iPacketSize)%d, (cpSTBegin)%X, (cpSTBegin)%X\n",
			iNext, lpSockContext->index, iPacketSize, lpSockContext->cpSTBegin, lpSockContext->cpSTEnd );
#endif

		if( iSendNow )
		{
			CopyMemory( &dummy, lpSockContext->cpSTBegin+sizeof(short), sizeof(short) );
			
			NowTcpSend( lpSockContext, j, iPacketSize );
		}
		else
		{

			if( lpSockContext->iSTRestCnt > RINGBUFSIZE )
			{
				// �� ��Ȳ�� �������� ���۸� �۰� �Ҵ� �ߴ��� �ƴϸ� ��ŷ�� �ǽ� �� �� �ִ�.
#ifdef _LOGLEVEL1_
				ServerUtil.ConsoleOutput( "---tcp sendbuf overflow!(%d) : %d ---\n",
					lpSockContext->index, lpSockContext->iSTRestCnt );
#endif
#ifdef _LOGFILELEVEL1_
				ServerUtil.PostTcpSendErrLog( lpSockContext->cID, WSAGetLastError() );
#endif
			}
		}

		iNext = ServerContext.pn[ iNext ].next;
	}
}



void RecvTcpBufDequeue(LPSOCKETCONTEXT lpSockContext, char **cpPacket, int *iPacketSize)
{
	short		iBodySize;
	int			iExtra;

	*cpPacket	= lpSockContext->cpRTBegin;
	
	iExtra		= lpSockContext->cRecvTcpRingBuf + RINGBUFSIZE - lpSockContext->cpRTBegin;
	if( iExtra < sizeof( short) )
	{
		*(lpSockContext->cRecvTcpRingBuf + RINGBUFSIZE) = *lpSockContext->cRecvTcpRingBuf;
	}

	CopyMemory(&iBodySize, lpSockContext->cpRTBegin, sizeof( short) );

	if( iExtra <= iBodySize + HEADERSIZE )
	{
		CopyMemory( lpSockContext->cRecvTcpRingBuf + RINGBUFSIZE, 
				lpSockContext->cRecvTcpRingBuf, iBodySize + HEADERSIZE -iExtra );
		
		lpSockContext->cpRTBegin = lpSockContext->cRecvTcpRingBuf + iBodySize + 
																HEADERSIZE - iExtra;
	}
	else
	{
		lpSockContext->cpRTBegin = lpSockContext->cpRTBegin + iBodySize + HEADERSIZE;
	}

	*iPacketSize = iBodySize + HEADERSIZE;

#ifdef _LOGLEVEL1_
	ServerUtil.ConsoleOutput( "RecvTcpDequeue : %d, %d byte\n", lpSockContext->index, *iPacketSize);
#endif

}



void EnqueueClose(LPSOCKETCONTEXT lpSockContext)
{
	if( lpSockContext->cPosition == WH_LOBBY || lpSockContext->cPosition == WH_ROOM )
		LogOutProcess( lpSockContext );

	OnRequestLogoutDB( lpSockContext, NULL );
}



BOOL UserKickOrAgency( LPSOCKETCONTEXT lpSockContext )
{
	// ���� ���� ��ġ�� �ƴϸ� �� ���� �ʱ�ȭ�� ���̴�.
	if( lpSockContext->cPosition == NOTLINKED ) return 0;

	// �α��� ���� �� ������ ��ġ�� �κ� ���̶�� ��Ȳ�� �°� ������ ó���Ѵ�.
	EnterCriticalSection( &ServerContext.csKickUserIndexList );


	// ä�� ����Ʈ�� �������� �α� �ƿ��̶��....
	if( lpSockContext->bRequsetClose == TRUE )
	{
		//EnqueueClose( lpSockContext );

		lpSockContext->bAgency = TRUE;
		ServerContext.dKickUserIndexList.push_back( lpSockContext->index );

		LeaveCriticalSection( &ServerContext.csKickUserIndexList );
		return TRUE;
	}

	// �濡 �� ���� �ʴٸ�
	if( lpSockContext->iRoom < 0 || lpSockContext->iRoom > ServerContext.iMaxRoom )
	{
#ifdef _LOGFILELEVEL3_
		ServerUtil.UserKickLog( lpSockContext->cID, lpSockContext->cState );
#endif
		//EnqueueClose( lpSockContext );
		lpSockContext->bAgency = TRUE;
		ServerContext.dKickUserIndexList.push_back( lpSockContext->index );

		LeaveCriticalSection( &ServerContext.csKickUserIndexList );
		return TRUE;
	}


	// �濡 �� �ִٸ�
	CRoom* pRoom = &ServerContext.rm[ lpSockContext->iRoom ];

	// ���� ���̸� ������ ������ ��� ǥ���ϰ�, ���� ���� �ƴϸ� ���������.
	if( ( pRoom->m_iState >= GAME_WAIT && pRoom->m_iState <= GAME_SCORECALCULATE ) ||
		( lpSockContext->cState > GAME_OFF && lpSockContext->cState <= GAME_ING ) )
	{
#ifdef _LOGFILELEVEL3_
		ServerUtil.UserAgencyLog( lpSockContext->cID, lpSockContext->cState );
#endif
		lpSockContext->bAgency = TRUE;

		LeaveCriticalSection( &ServerContext.csKickUserIndexList );
		return FALSE;
	}
	else
	{
#ifdef _LOGFILELEVEL3_
		ServerUtil.UserKickLog( lpSockContext->cID, lpSockContext->cState );
#endif
		//EnqueueClose( lpSockContext );
		lpSockContext->bAgency = TRUE;
		ServerContext.dKickUserIndexList.push_back( lpSockContext->index );

		LeaveCriticalSection( &ServerContext.csKickUserIndexList );
		return TRUE;
	}
}	