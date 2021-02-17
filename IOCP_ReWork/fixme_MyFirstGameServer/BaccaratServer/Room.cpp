// Room.cpp: implementation of the CRoom class.
//
//////////////////////////////////////////////////////////////////////


#include "protocol_baccarat.h"
#include "packetcoder.h"
#include "server.h"
#include "sock.h"
#include "channel.h"
#include "game.h"
#include "serverutil.h"
#include "Room.h"




//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CRoom::CRoom()
{

}

CRoom::~CRoom()
{

}

int InitRoomLayer()
{
	int		iN, iNum;
	int		iLow, iMiddle;
	char	nLevel;

	iLow = ( ServerContext.cChannelLowNum * ServerContext.iMaxRoomInChannel );
	iMiddle = ( ( ServerContext.cChannelLowNum + ServerContext.cCnannelMiddleNum ) * 
											ServerContext.iMaxRoomInChannel );

	iNum				= ServerContext.iMaxChannelInProcess * ServerContext.iMaxRoomInChannel;
	ServerContext.rm	= new CRoom[ iNum ];
	if( ServerContext.rm == NULL ) return 0;

	for( iN = 0; iN < iNum; ++iN )
	{
		if( iN < iLow )
		{
			nLevel = LEVEL_LOW;
		}
		else if( iN < iMiddle )
		{
			nLevel = LEVEL_MIDDLE;
		}
		else
		{
			nLevel = LEVEL_HIGH;
		}

		ServerContext.rm[ iN ].InitRoom( iN, nLevel );
	}

	ServerContext.rn = new OBJECTNODE[ iNum ];
	if( ServerContext.rn == NULL ) return 0;

	OnTransFunc[ REQUEST_CREATEROOM ].proc		= OnRequestCreateRoom;
	OnTransFunc[ REQUEST_JOINROOM ].proc		= OnRequestJoinRoom;
	OnTransFunc[ REQUEST_EXITROOM ].proc		= OnRequestExitRoom;
	OnTransFunc[ REQUEST_CHATINROOM ].proc		= OnRequestChatInRoom;
	OnTransFunc[ REQUEST_INVITEA ].proc			= OnRequestInvite;
	OnTransFunc[ REQUEST_INVITE_RESULTB].proc	= OnRequestInvite_Result;
	OnTransFunc[ REQUEST_CHANGE_ROOMTITLE ].proc= OnRequestChangeRoomTitle;
    OnTransFunc[ REQUEST_CHANGE_ROOMTYPE ].proc = OnRequestChangeRoomType;

		
#ifdef _LOGROOM_
	ServerUtil.ConsoleOutput( "InitRoom \n" );
#endif

	return 1;
}


void CRoom::InitRoom( int idx, char nlevel )
{
	int i;
	char j;

	m_iIndex		= idx;
	m_nLevel		= nlevel;

	m_iChannel		= static_cast<char>( idx / ServerContext.iMaxRoomInChannel );
	m_RoomNum		= static_cast<char>( m_iIndex - ( ServerContext.iMaxRoomInChannel * m_iChannel ) );

	for( i = 0; i < 2; ++i )
	{
		if( nlevel == LEVEL_LOW )
		{
			m_Basebettingmoney[0] = ServerContext.BaseBettingMoney.first;
			m_Basebettingmoney[1] = ServerContext.BaseBettingMoney.first * 5;
			m_Basebettingmoney[2] = ServerContext.BaseBettingMoney.first * 10;
			m_Basebettingmoney[3] = ServerContext.BaseBettingMoney.first * 50;
			m_Basebettingmoney[4] = ServerContext.BaseBettingMoney.first * 100;
		}
		else if( nlevel == LEVEL_MIDDLE )
		{
			m_Basebettingmoney[0] = ServerContext.BaseBettingMoney.middle;
			m_Basebettingmoney[1] = ServerContext.BaseBettingMoney.middle * 5;
			m_Basebettingmoney[2] = ServerContext.BaseBettingMoney.middle * 10;
			m_Basebettingmoney[3] = ServerContext.BaseBettingMoney.middle * 50;
			m_Basebettingmoney[4] = ServerContext.BaseBettingMoney.middle * 100;
		}
		else
		{
			m_Basebettingmoney[0] = ServerContext.BaseBettingMoney.high;
			m_Basebettingmoney[1] = ServerContext.BaseBettingMoney.high * 5;
			m_Basebettingmoney[2] = ServerContext.BaseBettingMoney.high * 10;
			m_Basebettingmoney[3] = ServerContext.BaseBettingMoney.high * 50;
			m_Basebettingmoney[4] = ServerContext.BaseBettingMoney.high * 100;
		}
	}
	
	// ī�带 �ʱ�ȭ �Ѵ�.
	for( j = 0 ; j < MAXIMUMCARDNUM; ++j )
	{
		m_privatecard[j] = j;
	}
	


	m_iUserBegin	= NOTLINKED;
	m_iUserEnd		= NOTLINKED;
	m_iUserNum		= 0;
	m_iState		= GAME_OFF;	
	m_tRoomTime		= 0; 
}


void CRoom::SetUserLink(int idx)
{
	if( m_iUserNum == 0 )
	{
		ServerContext.pn[ idx ].prev = NOTLINKED;
		m_iUserBegin = m_iUserEnd    = idx;
	}
	else
	{
		ServerContext.pn[ m_iUserEnd ].next	 = idx;
		ServerContext.pn[ idx ].prev		 = m_iUserEnd;
		m_iUserEnd = idx;
	}

	ServerContext.pn[ idx ].next = NOTLINKED;
	++m_iUserNum;
}


void CRoom::KillUserLink(int idx)
{
	int			prev, next;

	prev = ServerContext.pn[ idx ].prev;
	next = ServerContext.pn[ idx ].next;

	if( m_iUserNum == 1 )
	{
		m_iUserBegin = NOTLINKED;
		m_iUserEnd   = NOTLINKED;
	}
	else if( idx == m_iUserBegin )
	{
		ServerContext.pn[ next ].prev = NOTLINKED;
		m_iUserBegin = next;
	}
	else if( idx == m_iUserEnd )
	{
		ServerContext.pn[ prev ].next = NOTLINKED;
		m_iUserEnd = prev;
	}
	else
	{
		ServerContext.pn[ prev ].next = next;
		ServerContext.pn[ next ].prev = prev;
	}

	ServerContext.pn[ idx ].next = NOTLINKED;
	ServerContext.pn[ idx ].prev = NOTLINKED;
	--m_iUserNum;
}




int OnRequestCreateRoom(LPSOCKETCONTEXT lpSockContext, char *cpPacket)
{
	CChannel		*pChannel = &ServerContext.ch[ lpSockContext->iChannel ];
	CRoom			*pRoom = NULL;
	CPacketCoder	packetcoder;
	ROOMINFO		RoomInfo;
	char			cRoomNum;
	char			cPacket[ 16 ] = { 0, };
	int				iSize, iNext;

	iSize = iNext = 0;

	// ���� ������ �߶� ������.
	if( lpSockContext->GameState.money <= 0 ) 
	{
		EnqueueClose( lpSockContext);
		return 0;
	}

	// ������ ��ġ�� �κ� �ƴ϶�� �� ����� �߸��� ���̴�.
	if( lpSockContext->cPosition != WH_LOBBY ) return 0;

	packetcoder.SetBuf( cpPacket );
	packetcoder.GetChar( &RoomInfo.cType );
	packetcoder.GetChar( &RoomInfo.cTitleLen );		
	if( RoomInfo.cTitleLen <= 0 || RoomInfo.cTitleLen > MAXROOMTITLE ) return 0;
	
	packetcoder.GetText( RoomInfo.cTitle, RoomInfo.cTitleLen );

	if( RoomInfo.cType == PRIVATETYPE )
	{
		// �� ������ ���� �������� ������ �̰� �߸��� ������ ��û�̹Ƿ� ����.
		if( lpSockContext->bCanPrivateItem == FALSE ) return 0;

		packetcoder.GetChar( &RoomInfo.cPWDLen );   
		if( RoomInfo.cPWDLen <=0 || RoomInfo.cPWDLen >= MAXROOMPWD ) return 0;
		
		packetcoder.GetText( RoomInfo.cPWD, RoomInfo.cPWDLen );
	}
		
#ifdef _LOGROOM_
	ServerUtil.ConsoleOutput( "OnRequestCreateRoom(%d) : %d\n", 
							lpSockContext->index, RoomInfo.cType );
#endif

	
	if( pChannel->m_iUnusedRoomNum <= 0 )
	{
		packetcoder.SetBuf( cPacket );
		packetcoder.PutChar( CREATEROOM_FAIL_MAXROOM );
		iSize = packetcoder.SetHeader( ANSWER_CREATEROOM );
		PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );
		return 1;
	}
	else
	{
		// ���� ������ �����ϰ� ��û�� �������� ����������� �뺸�Ѵ�.
		pRoom = &ServerContext.rm[ pChannel->m_iUnusedRoomBegin ];
		AnswerCreateRoom( lpSockContext, pChannel, pRoom, RoomInfo );
	}

	
	if( pRoom )
	{
		cRoomNum = static_cast<char>(pRoom->m_iIndex - pChannel->m_iBaseRoomIndex);

		// �� ����, ���� ���� ���� �뺸
		NotifyCreateRoom( lpSockContext, pRoom, cRoomNum, pChannel->m_iUserBegin, pChannel->m_iUsedRoomBegin );
	}

	return 1;
}


void AnswerCreateRoom( LPSOCKETCONTEXT lpSockContext, CChannel* pChannel, CRoom* pRoom, ROOMINFO& pRoomInfo )
{
	CPacketCoder	packetcoder;
	char			cPacket[ 32 ] = { 0, };
	int				iSize;

	pRoom->m_iType = pRoomInfo.cType;
	pRoom->m_iState = GAME_OFF;
	pRoom->m_iIndexprivatecard = NOTLINKED;
	pRoom->m_cTitleLen = pRoomInfo.cTitleLen;
	CopyMemory( pRoom->m_cTitle, pRoomInfo.cTitle, pRoomInfo.cTitleLen );

	pRoom->m_cOwnerLen = lpSockContext->idLen;
	CopyMemory( pRoom->m_cOwner, lpSockContext->cID, lpSockContext->idLen );
	pRoom->m_iOwnerIndex = lpSockContext->index;
		
	if( pRoomInfo.cType == PRIVATETYPE )
	{
		pRoom->m_cPWDLen = pRoomInfo.cPWDLen;
		CopyMemory( pRoom->m_cPWD, pRoomInfo.cPWD, pRoomInfo.cPWDLen );
	}

	pChannel->SetRoomLink( pChannel->m_iUnusedRoomBegin );
	pChannel->KillUserLink( lpSockContext->index );
	pRoom->SetUserLink( lpSockContext->index );
	
	pRoom->InitRoomOrder();
	lpSockContext->iOrderInRoom = pRoom->GetOrderInRoom();

	lpSockContext->iRoom = pRoom->m_iIndex;
	lpSockContext->cPosition = WH_ROOM;
	
	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( CREATEROOM_SUCCESS );
	packetcoder.PutChar( pRoom->m_RoomNum );
	packetcoder.PutChar( pRoom->m_nLevel );
	packetcoder.PutInt( pRoom->m_Basebettingmoney[0] );
	iSize = packetcoder.SetHeader( ANSWER_CREATEROOM );
	PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );

#ifdef _LOGFILELEVEL3_
	ServerUtil.EnterRoomLog( pRoom->m_iChannel, pRoom->m_RoomNum, pRoom->GetCurUserNum(), TRUE, lpSockContext->cID );
#endif
}


void NotifyCreateRoom( LPSOCKETCONTEXT lpSockContext, CRoom* pRoom, char cRoomNum, int iBeginUser, int iUsedRoomBegin )
{
	CPacketCoder	packetcoder;
	CRoom*			tmpRoom = NULL;
	int				iSize, iNext;
	char			cPacket[ 64 ] = { 0, };

	// �� ���� �뺸
	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( 1 );
	packetcoder.PutChar( cRoomNum );
	packetcoder.PutChar( pRoom->m_cOwnerLen );
	packetcoder.PutText( pRoom->m_cOwner, pRoom->m_cOwnerLen );
	packetcoder.PutChar( pRoom->m_cTitleLen );
	packetcoder.PutText( pRoom->m_cTitle, pRoom->m_cTitleLen );
	packetcoder.PutChar( pRoom->m_iUserNum );
	packetcoder.PutChar( pRoom->m_iType );
	packetcoder.PutChar( pRoom->m_iState );
	iSize = packetcoder.SetHeader( ANSWER_NOTIFY_ROOMLIST );

	// ���� ���� ���� �뺸
	packetcoder.SetBuf( cPacket + iSize );
	packetcoder.PutInt( lpSockContext->iUserDBIndex );
	packetcoder.PutChar( WH_ROOM );
	packetcoder.PutChar( cRoomNum );
	iSize += packetcoder.SetHeader( ANSWER_NOTIFY_USERSETINFO );
	if( iBeginUser != NOTLINKED ) 
		PostTcpSend( iBeginUser, cPacket, iSize );

	iNext = iUsedRoomBegin;
	while( iNext > NOTLINKED && iNext < ServerContext.iMaxRoom )
	{
		tmpRoom = &ServerContext.rm[ iNext ];
		PostTcpSend( tmpRoom->m_iUserBegin, cPacket, iSize );
		iNext = ServerContext.rn[ iNext ].next;
	}
}



int OnRequestJoinRoom(LPSOCKETCONTEXT lpSockContext, char *cpPacket)
{
	CChannel*		pChannel = &ServerContext.ch[ lpSockContext->iChannel ];
	CRoom			*pRoom;
	CPacketCoder	packetcoder;
	char			cRoomNum, cPWDLen; 
	char			cPWD[32] = { 0, };
	char			cPacket[64] = { 0, };
	int				iSize;
	BOOL			bJoined = FALSE; 


	// ���� ������ �߶� ������.
	if( lpSockContext->GameState.money <= 0 ) 
	{
		EnqueueClose( lpSockContext);
		return 0;
	}

	if( lpSockContext->cPosition != WH_LOBBY ) return 0;
		
	packetcoder.SetBuf( cpPacket );
	packetcoder.GetChar( &cRoomNum );
	packetcoder.GetChar( &cPWDLen );		if( cPWDLen < 0 ) return 0;		
	if( cPWDLen > 1 ) packetcoder.GetText( cPWD, cPWDLen );
	
	if( cRoomNum < 0 || cRoomNum >= ServerContext.iMaxRoomInChannel ) return 0;
	
	pRoom = &ServerContext.rm[ cRoomNum + pChannel->m_iBaseRoomIndex ];

	// ���� ���°� ��⳪ ���� ���̶�� ���� �� �� ����.
	if( pRoom->m_iState != GAME_OFF && pRoom->m_iState != GAME_READY ) return 0;
	
	// �� ������ ���� �� ���� ���� �� ��� �뺸
	bJoined = AnswerJoinRoom( lpSockContext, pRoom, cRoomNum, cPWD, cPWDLen );
	
	if( bJoined )
	{
		pChannel->KillUserLink( lpSockContext->index );
		pRoom->SetUserLink( lpSockContext->index );

		// �� �� ���� ���� ���� �뺸
		NotifyJoinRoomSetInfo( lpSockContext, pRoom, cRoomNum, pChannel->m_iUserBegin, pChannel->m_iUsedRoomBegin );

		// ������ �뿡 ������ �뺸
		packetcoder.SetBuf( cPacket );
		packetcoder.PutInt( lpSockContext->iUserDBIndex );
		packetcoder.PutChar( lpSockContext->iOrderInRoom );
		// ��
		int iHigh = (int)((lpSockContext->GameState.money >> 32) & (__int64)0x00000000FFFFFFFF);
		int iLow = (int)((lpSockContext->GameState.money) & (__int64)0x00000000FFFFFFFF);
		packetcoder.PutInt( iHigh );
		packetcoder.PutInt( iLow );
		//packetcoder.PutText( (char*)lpSockContext->AvatarInfo, sizeof(short)*MAXAVATARLAYER );
		//packetcoder.PutChar( lpSockContext->AvatarType );

		iSize = packetcoder.SetHeader( ANSWER_NOTIFY_USERJOINROOM );
		PostTcpSend( pRoom->m_iUserBegin, cPacket, iSize );

		CheckChangeStateRoom( pRoom );		// ���� ���¸� ���� �Ѵ�.
	}

#ifdef _LOGROOM_
	ServerUtil.ConsoleOutput( "OnRequestJoinRoom(%d)\n", lpSockContext->index, cRoomNum );
#endif

	return 1;
}



BOOL AnswerJoinRoom( LPSOCKETCONTEXT lpSockContext, CRoom* pRoom, char cRoomNum, char* cPWD, char cPWDLen )
{
	CPacketCoder	packetcoder;
	LPSOCKETCONTEXT lpSC;
	char			cPacket[256] = { 0, };
	BOOL			bJoined = FALSE;
	int				iSize, iHigh, iLow;
	int				iNext = pRoom->GetUserBegin();
	
	packetcoder.SetBuf( cPacket );

	
	if( pRoom->m_iUserNum < ServerContext.iMaxUserInRoom )
	{
		if( pRoom->m_iType == PRIVATETYPE && strncmp( cPWD, pRoom->m_cPWD, cPWDLen ) != 0 )
		{
			packetcoder.PutChar( ROOM_FAIL_PWD );
		}
		else if( pRoom->m_iUserNum <= 0 )
		{
			packetcoder.PutChar( ROOM_FAIL_EXIST);
		}
		else
		{
			pRoom->CheckUserLinkInRoom();

			lpSockContext->iOrderInRoom = pRoom->GetOrderInRoom();

			packetcoder.PutChar( ROOM_SUCCESS_JOINROOM );
			packetcoder.PutChar( cRoomNum );
			packetcoder.PutChar( pRoom->m_nLevel );
			packetcoder.PutInt( pRoom->m_Basebettingmoney[0] );
			packetcoder.PutChar( pRoom->GetCurUserNum() + 1 );
			
			while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
			{
				lpSC = &ServerContext.sc[ iNext ];

				packetcoder.PutInt( lpSC->iUserDBIndex );
				packetcoder.PutChar( lpSC->iOrderInRoom );

				// ��
				iHigh = (int)((lpSC->GameState.money >> 32) & (__int64)0x00000000FFFFFFFF);
				iLow = (int)((lpSC->GameState.money) & (__int64)0x00000000FFFFFFFF);
				packetcoder.PutInt( iHigh );
				packetcoder.PutInt( iLow );
				//packetcoder.PutText( (char*)lpSC->AvatarInfo, sizeof(short)*MAXAVATARLAYER );
				//packetcoder.PutChar( lpSC->AvatarType );

				iNext = ServerContext.pn[ iNext ].next;
			}
			
			packetcoder.PutInt( lpSockContext->iUserDBIndex );
			packetcoder.PutChar( lpSockContext->iOrderInRoom );
			iHigh = (int)((lpSockContext->GameState.money >> 32) & (__int64)0x00000000FFFFFFFF);
			iLow = (int)((lpSockContext->GameState.money) & (__int64)0x00000000FFFFFFFF);
			packetcoder.PutInt( iHigh );
			packetcoder.PutInt( iLow );

			lpSockContext->iRoom		= pRoom->m_iIndex;
			lpSockContext->cPosition	= WH_ROOM;
			
			bJoined = TRUE;

#ifdef _LOGFILELEVEL3_
			ServerUtil.EnterRoomLog( pRoom->m_iChannel, pRoom->m_RoomNum, pRoom->GetCurUserNum(), FALSE, lpSockContext->cID );
#endif
		}
	}
	else
	{
		packetcoder.PutChar( ROOM_FAIL_MAXUSER );
	}

	iSize = packetcoder.SetHeader( ANSWER_JOINROOM );
	PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );

	return bJoined;
}



void NotifyJoinRoomSetInfo( LPSOCKETCONTEXT lpSockContext, CRoom* pRoom, char cRoomNum, int iUserBegin, int iUsedRoomBegin )
{
	CPacketCoder	packetcoder;
	CRoom*			pTmp = NULL;
	char			cPacket[ 32 ] = { 0, };
	int				iSize, iNext;


	//�� ���� ���� �뺸
	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( cRoomNum );
	packetcoder.PutChar( pRoom->m_iUserNum );
	packetcoder.PutInt( 0 );
	iSize = packetcoder.SetHeader( ANSWER_NOTIFY_ROOMSETINFO );

	// ������ ���� ����
	packetcoder.SetBuf( cPacket + iSize );
	packetcoder.PutInt( lpSockContext->iUserDBIndex );
	packetcoder.PutChar( WH_ROOM );
	packetcoder.PutChar( cRoomNum );
	iSize += packetcoder.SetHeader( ANSWER_NOTIFY_USERSETINFO );
	PostTcpSend( iUserBegin, cPacket, iSize );

	
	// �濡 �ִ� ������Ե� �˷��ش�.
	iNext = iUsedRoomBegin;
	while( iNext > NOTLINKED && iNext < ServerContext.iMaxRoom )
	{
		pTmp = &ServerContext.rm[ iNext ];
		PostTcpSend( pTmp->m_iUserBegin, cPacket, iSize );
		iNext = ServerContext.rn[ iNext ].next;
	}
}



void CheckChangeStateRoom( CRoom* pRoom )
{
	if( pRoom->m_iState == GAME_OFF && pRoom->m_iUserNum >= MINPLAYER )
	{
		pRoom->m_iState = GAME_READY;
		time( &pRoom->m_tRoomTime );
	}
	else if( pRoom->m_iState == GAME_READY && pRoom->m_iUserNum < MINPLAYER)
	{
		pRoom->m_iState = GAME_OFF;
		pRoom->m_iIndexprivatecard = NOTLINKED;
	}
}



int OnRequestExitRoom(LPSOCKETCONTEXT lpSockContext, char* cpPacket)
{
	CChannel		*pChannel = &ServerContext.ch[ lpSockContext->iChannel ];
	CPacketCoder	packetcoder;
	char			result, cPacket[ 64 ] = { 0, };
	int				iSize;
	BOOL			bRemain = FALSE;

	if( lpSockContext->cPosition != WH_ROOM || lpSockContext->iRoom == NOTLINKED ) return 0;

	CRoom			*pRoom = &ServerContext.rm[ lpSockContext->iRoom ];

	// ���� �����̸� ���� �� �� �� ����.
	if( lpSockContext->cState > GAME_OFF || 
		( pRoom->m_iState >= GAME_WAIT && pRoom->m_iState <= GAME_SCORECALCULATE ) ) 
	{
		lpSockContext->bReservation = !lpSockContext->bReservation;

		// ��Ŷ ���ڵ�
		packetcoder.SetBuf( cPacket );
		packetcoder.PutInt( lpSockContext->iUserDBIndex );

		// ������ ������ ������ ���� ��� ������� �˷��ش�.
		if( lpSockContext->bReservation == TRUE )
			packetcoder.PutChar( 1 );
		else
			packetcoder.PutChar( 0 );

		iSize = packetcoder.SetHeader( ANSWER_NOTIFY_RESERVATE_EXITROOM );
		PostTcpSend( pRoom->GetUserBegin(), cPacket, iSize );
		
		return 2;
	}

#ifdef _LOGFILELEVEL3_
	time( &ServerUtil.m_LogTime );
	tm* m_LogCalTime = localtime( &ServerUtil.m_LogTime );
	ServerUtil.LogPrint( "< %d-%d-%dh-%dm-%ds : OnRequestExitRoom - Channel(%d), Room(%d), State(%d) >\n",
		m_LogCalTime->tm_mon+1, m_LogCalTime->tm_mday, m_LogCalTime->tm_hour, 
		m_LogCalTime->tm_min, m_LogCalTime->tm_sec, pRoom->m_iChannel, pRoom->m_RoomNum, lpSockContext->cState );
#endif

	// ��Ŷ ���ڵ�
	packetcoder.SetBuf( cpPacket );
	packetcoder.GetChar( &result );


	// �뿡�� ������ ����� ä�ο� ��ũ ��Ų��.
	pRoom->KillUserLink( lpSockContext->index );
	pChannel->SetUserLink( lpSockContext->index );

	lpSockContext->cState	= GAME_OFF;
	lpSockContext->cPosition = WH_LOBBY;
	lpSockContext->iRoom = NOTLINKED;

	// ��Ŷ ���ڵ�
	packetcoder.SetBuf( cPacket );

	if( result == EXITROOM_NORMAL ) 
		packetcoder.PutChar( EXITROOM_NORMAL );
	else
		packetcoder.PutChar( EXITROOM_NOTMONEY );

	iSize = packetcoder.SetHeader( ANSWER_EXITROOM );
	PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );


	// ���� ���¸� ���� �Ѵ�.
	CheckChangeStateRoom( pRoom );

	if( pRoom->m_iUserNum <= 0 )
	{
		// ���� ���� ���� �˸�
		iSize = ExitNotifyRoomDelete( pChannel, pRoom, *cPacket );
	}
	else
	{
		// ���ο� ���� ������ �����ش�.
		iSize = ExitNotiftRoomSetInfo( lpSockContext, pChannel, pRoom, *cPacket );
		bRemain = TRUE;
	}

	// ���� ���� �뺸
	ExitRoomNotifyUserSetInfo( lpSockContext, *cPacket, iSize,  pChannel->m_iUserBegin, pChannel->m_iUsedRoomBegin );

	if( bRemain )
	{
		// �濡 ���� �ִ� ������� Ÿ ������ ������ �˸��� ������ �����Ѵ�.
		RemainUserInRoom( pRoom, lpSockContext );
	}
	return 1;
}


int ExitNotifyRoomDelete( CChannel* pChannel, CRoom* pRoom, char& cPacket )
{
	CPacketCoder	packetcoder;
	int				iSize;

	pRoom->m_iState = GAME_OFF;
	pChannel->KillRoomLink( pRoom->m_iIndex );

	packetcoder.SetBuf( &cPacket );
	packetcoder.PutChar( static_cast<char>(pRoom->m_iIndex - pChannel->m_iBaseRoomIndex) );
	iSize = packetcoder.SetHeader( ANSWER_NOTIFY_ROOMDELETE );

	return iSize;
}


int ExitNotiftRoomSetInfo( LPSOCKETCONTEXT lpSockContext, CChannel* pChannel, CRoom* pRoom, char& cPacket )
{
	CPacketCoder	packetcoder;
	LPSOCKETCONTEXT	lpSC;
	int				iSize;

	packetcoder.SetBuf( &cPacket );
	packetcoder.PutChar( static_cast<char>(pRoom->m_iIndex - pChannel->m_iBaseRoomIndex) );
	
	if( strncmp( pRoom->m_cOwner, lpSockContext->cID, pRoom->m_cOwnerLen ) == 0 )
	{
		lpSC = &ServerContext.sc[ pRoom->m_iUserBegin ];
		CopyMemory( pRoom->m_cOwner, lpSC->cID, lpSC->idLen );
		pRoom->m_cOwnerLen = lpSC->idLen;
		pRoom->m_iOwnerIndex = lpSC->index;

		packetcoder.PutChar( pRoom->m_iUserNum );
		packetcoder.PutInt( lpSC->iUserDBIndex );
	}
	else
	{
		packetcoder.PutChar( pRoom->m_iUserNum );
		packetcoder.PutInt( 0 );
	}
	
	iSize = packetcoder.SetHeader( ANSWER_NOTIFY_ROOMSETINFO );

	return iSize;
}


void ExitRoomNotifyUserSetInfo( LPSOCKETCONTEXT lpSockContext, char& cPacket, int iSize, int iUserBegin, int iRoomBegin )
{
	CPacketCoder	packetcoder;
	CRoom*			pTmp;
	int				iNext;
		
	// ���� ���� �뺸
	packetcoder.SetBuf( &cPacket+iSize );
	packetcoder.PutInt( lpSockContext->iUserDBIndex );
	packetcoder.PutChar( WH_LOBBY );
	iSize += packetcoder.SetHeader( ANSWER_NOTIFY_USERSETINFO );
	PostTcpSend( iUserBegin, &cPacket, iSize );

	iNext = iRoomBegin;
	while( iNext > NOTLINKED && iNext < ServerContext.iMaxRoom )
	{
		pTmp = &ServerContext.rm[ iNext ];
		PostTcpSend( pTmp->m_iUserBegin, &cPacket, iSize );
		iNext = ServerContext.rn[ iNext ].next;
	}
}



void RemainUserInRoom( CRoom* pRoom, LPSOCKETCONTEXT lpSockContext )
{
	CPacketCoder	packetcoder;
	char			cPacket[32] = { 0, };
	int				iSize;

	// ������ �뿡�� ������ �뺸
	packetcoder.SetBuf( cPacket );
	packetcoder.PutInt( lpSockContext->iUserDBIndex );
	packetcoder.PutChar( lpSockContext->iOrderInRoom );
	iSize = packetcoder.SetHeader( ANSWER_NOTIFY_USEREXITROOM );
	PostTcpSend( pRoom->m_iUserBegin, cPacket, iSize );
		
	// ���� ������ �ݳ��Ѵ�.
	pRoom->DeleteOrderInRoom( lpSockContext->iOrderInRoom );
}


int OnRequestChatInRoom(LPSOCKETCONTEXT lpSockContext, char *cpPacket)
{
	CRoom		*pRoom = &ServerContext.rm[ lpSockContext->iRoom ];
	short		dummy;

	dummy = ANSWER_CHATINROOM;
	CopyMemory( cpPacket + sizeof(short), &dummy, sizeof(short) );
	CopyMemory( &dummy, cpPacket, sizeof(short) );

	if( dummy > MAXCHATPACKETLENGTH ) return 0;

	PostTcpSend( pRoom->m_iUserBegin, cpPacket, dummy + HEADERSIZE );

#ifdef _LOGROOM_
	ServerUtil.ConsoleOutput( "OnRequestChatInRoom(%d) : %d, %s\n",
		lpSockContext->index, lpSockContext->iRoom, cpPacket+5 );
#endif

	return 1;
}


int OnRequestInvite( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder	packetcoder;
	CRoom			*pRoom;
	LPSOCKETCONTEXT	lpSC;
	int				iSize, iUserDBIndex, iUserIndex, iRoomNum;
	char			cChannelNum, cRoomNum;
	char			cIdLen, cPacket[ 64 ], cPWDLen, cPWD[32];

	iUserIndex = NOTLINKED;
	iSize = cChannelNum = cRoomNum = cIdLen = cPWDLen = 0;
	
	packetcoder.SetBuf( cpPacket );
	packetcoder.GetInt( &iUserDBIndex );
	
	cChannelNum = lpSockContext->iChannel;
	iRoomNum	= lpSockContext->iRoom;
	cRoomNum = static_cast<char>( iRoomNum - ( cChannelNum * ServerContext.iMaxRoomInChannel ) );
	
	if( cRoomNum == NOTLINKED ) return 0;
	if( lpSockContext->index != ServerContext.rm[ iRoomNum ].GetOwnerIndex() ) return 0;

	iUserIndex = ServerContext.ch[ cChannelNum ].GetDBIndexToSockIndex( iUserDBIndex );
	if( iUserIndex == NOTLINKED )
	{
		packetcoder.SetBuf( cPacket );
		packetcoder.PutChar( INVITE_FAIL_NOTUSER );
		iSize = packetcoder.SetHeader( ANSWER_INVITE_RESULTA );
		PostTcpSend(1, (int*)&lpSockContext, cPacket, iSize );
		return 1;
	}
	
		
	lpSC = &ServerContext.sc[ iUserIndex ];
	
	if( lpSC->bAllInvite == FALSE )
	{
		packetcoder.SetBuf( cPacket );
		packetcoder.PutChar( INVITE_FAIL_ALLREJECT );
		iSize = packetcoder.SetHeader( ANSWER_INVITE_RESULTA );
		PostTcpSend(1, (int*)&lpSockContext, cPacket, iSize );
		return 1;
	}


	pRoom = &ServerContext.rm[ iRoomNum ];
	if( pRoom->m_iType == PRIVATETYPE )
	{
		cPWDLen = pRoom->m_cPWDLen;
		CopyMemory( cPWD, pRoom->m_cPWD, cPWDLen );
	}


	
	packetcoder.SetBuf( cPacket );
	packetcoder.PutInt( lpSockContext->iUserDBIndex );
	packetcoder.PutChar( cChannelNum );
	packetcoder.PutChar( cRoomNum );

	if( cPWDLen > 0 )
	{
		packetcoder.PutChar( cPWDLen);
		packetcoder.PutText( cPWD, cPWDLen );
	}else
	{
		packetcoder.PutChar( 0 );
	}

	iSize = packetcoder.SetHeader( ANSWER_INVITEB );
	PostTcpSend(1, (int*)&lpSC, cPacket, iSize );
	
	return 1;
}


int OnRequestInvite_Result( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder		packetcoder;
	LPSOCKETCONTEXT		lpSC;
	int					iSize, iIndex, iDBIndex, iRoomNum;
	char				cResult, cChannelNum, cRoomNum, cPacket[ 64 ];

	iSize = iIndex = iDBIndex = iRoomNum = cChannelNum = cRoomNum = cResult = 0;

	packetcoder.SetBuf( cpPacket );
	packetcoder.GetChar( &cResult );
	packetcoder.GetInt( &iDBIndex );
	packetcoder.GetChar( &cChannelNum );
	packetcoder.GetChar( &cRoomNum );

	if( cRoomNum < 0 || cRoomNum >= ServerContext.iMaxRoomInChannel ) return 0;
	if( cChannelNum < 0 || cChannelNum >= ServerContext.iMaxChannel ) return 0;
	
	iRoomNum = cRoomNum + ( cChannelNum * ServerContext.iMaxRoomInChannel );
	
	iIndex = ServerContext.rm[ iRoomNum ].GetDBIndexToSockIndex( iDBIndex );
	if( iIndex == NOTLINKED ) return 0;
	
	lpSC = &ServerContext.sc[ iIndex ];

	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( cResult );
	iSize = packetcoder.SetHeader( ANSWER_INVITE_RESULTA );
	PostTcpSend( 1, (int*)&lpSC, cPacket, iSize );

#ifdef _LOGROOM_
	ServerUtil.ConsoleOutput( "OnRequestInvite_Result3(%d) : Size(%d)\n", lpSockContext->index, iSize);
#endif

	return 1;
}



int OnRequestChangeRoomTitle( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder packetcoder;
	char		 cTitlelen, dummy, cPacket[32] = {0,};
	short        iSize, iType = ANSWER_NOTIFY_CHANGEROOMTITLE;
	int			 iRoomNext;
	int			 RoomNum = lpSockContext->iRoom;

	if( RoomNum <= NOTLINKED || RoomNum >= ServerContext.iMaxRoom ) return 0;
	
	CRoom*		pRoom	 = &ServerContext.rm[ RoomNum ];
	CChannel*	pChannel = &ServerContext.ch[ pRoom->m_iChannel ];

	if( pRoom->m_iOwnerIndex != lpSockContext->index ) return 0;

	packetcoder.SetBuf( cpPacket );
	packetcoder.GetChar( &dummy );
	packetcoder.GetChar( &cTitlelen ); 

	if( cTitlelen <= 0 || cTitlelen > MAXROOMTITLE ) return 0;

	pRoom->m_cTitleLen = cTitlelen;
	packetcoder.GetText( pRoom->m_cTitle, pRoom->m_cTitleLen );


	CopyMemory( &iSize, cpPacket, sizeof(short) );
	CopyMemory( cpPacket+HEADERSIZE, &pRoom->m_RoomNum, sizeof(char) );
	CopyMemory( cpPacket + sizeof(short), &iType, sizeof(short) );
	PostTcpSend( pChannel->GetBeginUser(), cpPacket, iSize+HEADERSIZE );

	// �κ��� �濡 �ִ� ����鿡�� �뺸.
	CRoom*		pTmpRoom = NULL;
	iRoomNext = pChannel->GetBeginRoom();
	while( iRoomNext > NOTLINKED && iRoomNext < ServerContext.iMaxRoom )
	{
		pTmpRoom = & ServerContext.rm[ iRoomNext ];
		
		PostTcpSend( pTmpRoom->GetUserBegin(), cpPacket, iSize+HEADERSIZE );

		iRoomNext = ServerContext.rn[ iRoomNext ].next;
	}

	return 1;
}


int OnRequestChangeRoomType( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder packetcoder;
	char		 cType, cPWDlen, cPacket[32] = {0,};
	int			 iSize, iRoomNext;
	int			 RoomNum = lpSockContext->iRoom;
	

	if( RoomNum <= NOTLINKED || RoomNum >= ServerContext.iMaxRoom ) return 0;
	
	CRoom*		pRoom	 = &ServerContext.rm[ RoomNum ];
	CChannel*	pChannel = &ServerContext.ch[ pRoom->m_iChannel ];

	if( pRoom->m_iOwnerIndex != lpSockContext->index ) return 0;

	packetcoder.SetBuf( cpPacket );
	packetcoder.GetChar( &cType );
	
	// ���� Ÿ���� ������ �� ������ �ƴϸ� ��ŷ.
	if( cType < PUBLICTYPE || cType > PRIVATETYPE ) return 0;

	// ���� �н����� ������ ���ٸ�.
	if( cType == PUBLICTYPE )
	{
		if( cType == PRIVATETYPE && lpSockContext->bCanPrivateItem == FALSE ) return 0;
		pRoom->m_iType = cType;

		packetcoder.SetBuf( cPacket );
		packetcoder.PutChar( pRoom->m_RoomNum );
		packetcoder.PutChar( pRoom->m_iType );
	}
	else 
	{
		if( lpSockContext->bCanPrivateItem == FALSE ) return 0;
		pRoom->m_iType = cType;
		
		packetcoder.GetChar( &cPWDlen );

		if( cPWDlen <= 0 || cPWDlen >= MAXROOMPWD ) return 0;
		pRoom->m_cPWDLen = cPWDlen;

		packetcoder.GetText( pRoom->m_cPWD, pRoom->m_cPWDLen );

		packetcoder.SetBuf( cPacket );
		packetcoder.PutChar( pRoom->m_RoomNum );
		packetcoder.PutChar( pRoom->m_iType );
	}
	iSize = packetcoder.SetHeader( ANSWER_NOTIFY_CHANGEROOMTYPE );

	// �κ��� ����鿡�� �뺸
	PostTcpSend( pChannel->GetBeginUser(), cPacket, iSize );  

	// �κ��� �濡 �ִ� ����鿡�� �뺸.
	CRoom*		pTmpRoom = NULL;
	iRoomNext = pChannel->GetBeginRoom();
	while( iRoomNext > NOTLINKED && iRoomNext < ServerContext.iMaxRoom )
	{
		pTmpRoom = & ServerContext.rm[ iRoomNext ];
		
		PostTcpSend( pTmpRoom->GetUserBegin(), cPacket, iSize );

		iRoomNext = ServerContext.rn[ iRoomNext ].next;
	}
	
	return 1;
}

