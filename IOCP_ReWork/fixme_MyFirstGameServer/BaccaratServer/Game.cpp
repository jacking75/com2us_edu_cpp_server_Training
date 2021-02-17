// Game.cpp: implementation of the CGame class.
//
//////////////////////////////////////////////////////////////////////

#include "server.h"
#include "sock.h"
#include "protocol_baccarat.h"
#include "packetcoder.h"
#include "database.h"
#include "serverprocess.h"
#include "channel.h"
#include "room.h"
#include "serverutil.h"
#include "Game.h"


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGame::CGame()
{
	InitializeGameProc();
	AheadPacketed();
}

CGame::~CGame()
{

}


void InitializeGameProc()
{
	OnTransFunc[ REQUEST_BEGINGAME ].proc		  = OnRequestBeginGame;
	
}


void CGame::AheadPacketed()
{
	short size;
	short ReservateExitRoom = ANSWER_RESERVATE_EXITROOM;
	short NotMoney = ANSWER_EXITROOM;
	char  result;

	// ������ ������ ����� ������� ��Ŷ ����.
	size = HEADERSIZE;
	CopyMemory( ReservateExitRoomPacket, &size, sizeof(short) );
	CopyMemory( ReservateExitRoomPacket + sizeof(short), &ReservateExitRoom, sizeof(short) );

	// ���� ������ ���� ���� �� �α� �ƿ� �϶�� ��Ŷ ����.
	size = 1;
	result = EXITROOM_NOTMONEY;
	CopyMemory( NotMoneyPacket, &size, sizeof(short) );
	CopyMemory( NotMoneyPacket + sizeof(short), &NotMoney, sizeof(short) );
	CopyMemory( NotMoneyPacket + HEADERSIZE, &result, sizeof(char) );
}


void CGame::MixCard(CRoom *pRoom, int MixCount)
{
	int num1, num2;
	char ImsiCard;						// ī�带 �������� �ӽú���
	
	srand((unsigned)time(NULL));		// ���������� �ʱ�ȭ

	
	for(int k = 0 ; k < MixCount ; k++)
	{
		num1 = rand() % TOTALMAXCARDNUM;
		num2 = rand() % TOTALMAXCARDNUM;

		ImsiCard					= pRoom->m_privatecard[num1];
		pRoom->m_privatecard[num1]	= pRoom->m_privatecard[num2];
		pRoom->m_privatecard[num2]	= ImsiCard;
	}
}




void CGame::BeginGameInRoom( CRoom* pRoom )
{
	CPacketCoder	packetcoder;
	char			cPacket[32] = { 0, };
	int				i, iSize, iNext;
	int				nRoomNum = pRoom->m_RoomNum;

	pRoom->m_iState					= GAME_WAIT;

	// �濡 �ִ� ������ ���¸� ��� ���·� �ٲ۴�.
	i = 0;
	iNext = pRoom->GetUserBegin();
	while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
	{
		// ���� ������� ���� �����Ѵ�.
		if( iNext <= NOTLINKED || iNext >= ServerContext.iMaxUserNum )
			break;

		// ���߿� ���� ������ ������ ������ �������´�.
		if( ServerContext.sc[ iNext ].iRoom != nRoomNum )
			break;

		ServerContext.sc[ iNext ].cState = GAME_WAIT;
		ServerContext.sc[ iNext ].bReservation = FALSE;
		ServerContext.sc[ iNext ].nSelectArea = NOTLINKED;
	
		iNext = ServerContext.pn[ iNext ].next;

		++i;
	}

	// ������ �ʱ�ȭ ���� ���� ������ �ִٸ�
	if( i != pRoom->GetCurUserNum() )
	{
		BackGameInRoomState( pRoom );
		return;
	}

    NotifyRoomState( pRoom->m_iChannel, pRoom->m_RoomNum, GAME_ING );

    // ��Ŀ, �÷��̾�, Ÿ���� ī�带 �ʱ�ȭ �Ѵ�.
	pRoom->BankerInfo.nCurCardNum = 0;
	pRoom->PlayerInfo.nCurCardNum = 0;
	pRoom->TieInfo.nCurCardNum    = 0;

	// �� ī���� ������ �ʱ�ȭ �Ѵ�.
	memset( pRoom->BankerInfo.Cards, NOTUSECARD, BACCARAT_HOLDCARDNUM );
	memset( pRoom->PlayerInfo.Cards, NOTUSECARD, BACCARAT_HOLDCARDNUM );
	memset( pRoom->TieInfo.Cards, NOTUSECARD, BACCARAT_HOLDCARDNUM );
		

	// ��Ŀ, �÷��̾��� ������ ������ �ʱ�ȭ �Ѵ�.
	pRoom->BankerInfo.Score = 0;
	pRoom->PlayerInfo.Score = 0;

	// ī�带 ���� �ʱ�ȭ �Ѵ�.
	MixCard( pRoom, TOTALMAXCARDNUM );
	pRoom->m_iIndexprivatecard = NOTLINKED;
	

	// ������ ���۵��� �뺸 �Ѵ�.
	packetcoder.SetBuf( cPacket );
	iSize = packetcoder.SetHeader( ANSWER_GAMESTART );
	PostTcpSend( pRoom->GetUserBegin(), cPacket, iSize );

#ifdef _LOGFILELEVEL3_
	ServerUtil.BeginGameInRoomLog( pRoom->m_iChannel, pRoom->m_RoomNum, pRoom->GetCurUserNum() );
#endif
	
	pRoom->m_nStartCompleteUserNum	= 0;	// ���� �غ� ���� ��� ���� �ʱ�ȭ.
	
	time( &pRoom->m_tRoomTime );
	
}



int	OnRequestBeginGame( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder	packetcoder;
	LPSOCKETCONTEXT lpSC;
	int				roomNum = lpSockContext->iRoom;
	int				iSize, iNext;
	char			BetLevel, Pos, cPacket[64] = { 0,};	

	if( roomNum < 0 || roomNum >= ServerContext.iMaxRoom ) return 0;

	CRoom*		pRoom = &ServerContext.rm[ roomNum ];
	
	if( pRoom->m_iState != GAME_WAIT || lpSockContext->cState != GAME_WAIT ) return 0;
	
	packetcoder.SetBuf( cpPacket );
	packetcoder.GetChar( &BetLevel );
	packetcoder.GetChar( &Pos );

	// ���� ��ġ�� ��ȿ�� ���� �ߴ��� üũ �Ѵ�.
	if( Pos < 0 || Pos >= SELECTAREANUM ) return 0;

	// ���ñݾ��� �� ���� �ּ� �ִ� �ݾ��� �Ѿ�� ���̶�� �����Ѵ�. 
	if( BetLevel < MINBETTINGMULTYPLE || BetLevel >= MAXBETTINGMULTYPLE ) return 0;
	
	// ���� �ݾ��� ����Ѵ�.
	lpSockContext->ibetMoney = pRoom->m_Basebettingmoney[ BetLevel ];
	

	// ������� ���� �ߴ°��� üũ�Ͽ� �����Ѵ�.
	lpSockContext->nSelectArea = Pos;

	
	// ������ �����Ѱ��� �˷��� ������ ���� �����Ѵ�.
	pRoom->m_nStartCompleteUserNum++;

		
	// ������ ���¸� ���� ������ �ٲ۴�.
	lpSockContext->cState = GAME_ING;

	
	// ���� ���� ����ŭ ������ ������ ī�� 2���� ������.
	if( pRoom->m_nStartCompleteUserNum == pRoom->GetCurUserNum() )
	{
		// ��� ��������  ������ ���� ���� ������ �ݾ��� �˷��ش�.
		packetcoder.SetBuf( cPacket );
		packetcoder.PutChar( pRoom->GetCurUserNum() );

		iNext = pRoom->GetUserBegin();
		while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
		{	
			lpSC = &ServerContext.sc[ iNext ];

			packetcoder.PutInt( lpSC->iUserDBIndex );
			packetcoder.PutInt( lpSC->ibetMoney );
			packetcoder.PutChar( lpSC->nSelectArea );

			iNext = ServerContext.pn[ iNext ].next;
		}
		iSize = packetcoder.SetHeader( ANSWER_BEGINGAME );
		PostTcpSend( pRoom->GetUserBegin(), cPacket, iSize );
		
		pRoom->m_nStartCompleteUserNum = 0;
		pRoom->m_iState = GAME_ING;

#ifdef _LOGFILELEVEL3_
		ServerUtil.GameLog( pRoom->m_iChannel, pRoom->m_RoomNum, strRequestBeginGame );
#endif
		
		// ī�带 ������.
		ServerContext.gameproc->SendStartCards( pRoom );
	}
	
	return 1;
	
}



void CGame::AgencyRequestBeginGame(CRoom *pRoom)
{
	LPSOCKETCONTEXT lpSC;
	CPacketCoder	packetcoder;
	char			cPacket[64] = {0,};
	int				iSize;
	int				iNext = pRoom->GetUserBegin();


	pRoom->m_nStartCompleteUserNum = 0;
	pRoom->m_iState = GAME_ING;

	// ������ ���� ������ ��� �������� �뺸���ش�.
	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( pRoom->GetCurUserNum() );
		
	while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
	{
		lpSC = &ServerContext.sc[ iNext ];
		
		// ������ �˷����� ���� ������ ������ ���Ƿ� ��� �����Ѵ�.
		if( lpSC->cState == GAME_WAIT )
		{
			lpSC->cState	  = GAME_ING;
			lpSC->nSelectArea = _PLAYER;
			lpSC->ibetMoney   = pRoom->m_Basebettingmoney[0];
		}

		packetcoder.PutInt( lpSC->iUserDBIndex );
		packetcoder.PutInt( lpSC->ibetMoney );
		packetcoder.PutChar( lpSC->nSelectArea );

		iNext = ServerContext.pn[ iNext ].next;
		
	}
	iSize = packetcoder.SetHeader( ANSWER_BEGINGAME );
	PostTcpSend( pRoom->GetUserBegin(), cPacket, iSize );

#ifdef _LOGFILELEVEL3_
	ServerUtil.GameLog( pRoom->m_iChannel, pRoom->m_RoomNum, strAgencyRequestBeginGame );
#endif

	ServerContext.gameproc->SendStartCards( pRoom );
	
}


void CGame::SendStartCards( CRoom* pRoom )
{
	CPacketCoder	packetcoder;
	char			BankerScore, PlayerScore, cPacket[ 64 ] = { 0, };
	int				iSize;
	
	// ��Ŀ�� �÷��̾�� ī�带 2��� �Ҵ��Ѵ�.
	pRoom->BankerInfo.Cards[0] = GetPrivateCard( pRoom );
	pRoom->PlayerInfo.Cards[0] = GetPrivateCard( pRoom );

	pRoom->BankerInfo.Cards[1] = GetPrivateCard( pRoom );
	pRoom->PlayerInfo.Cards[1] = GetPrivateCard( pRoom );
	
	pRoom->BankerInfo.nCurCardNum = 2;
	pRoom->PlayerInfo.nCurCardNum = 2;

	// ��Ŀ�� ���� ī���� ������ ����Ѵ�.
	pRoom->BankerInfo.Score += CardToNumber[ pRoom->BankerInfo.Cards[0] ];
	pRoom->BankerInfo.Score += CardToNumber[ pRoom->BankerInfo.Cards[1] ];
	if( pRoom->BankerInfo.Score >= 10 ) pRoom->BankerInfo.Score %= 10;
	

	// �÷��̾ ���� ī���� ������ ����Ѵ�.
	pRoom->PlayerInfo.Score += CardToNumber[ pRoom->PlayerInfo.Cards[0] ];
	pRoom->PlayerInfo.Score += CardToNumber[ pRoom->PlayerInfo.Cards[1] ];
	if( pRoom->PlayerInfo.Score >= 10 ) pRoom->PlayerInfo.Score %= 10;
	

	// ��Ŀ�� �÷��̾��� ī��� ������ �˷��ش�. 
	packetcoder.SetBuf( cPacket );
	packetcoder.PutText( pRoom->BankerInfo.Cards, 2 );
	packetcoder.PutChar( pRoom->BankerInfo.Score );
	packetcoder.PutText( pRoom->PlayerInfo.Cards, 2 );
	packetcoder.PutChar( pRoom->PlayerInfo.Score );
	iSize = packetcoder.SetHeader( ANSWER_STARTCARDS );
	PostTcpSend( pRoom->GetUserBegin(), cPacket, iSize );

#ifdef _LOGFILELEVEL3_
	ServerUtil.GameLog( pRoom->m_iChannel, pRoom->m_RoomNum, strSendStartCards );
#endif

	BankerScore = pRoom->BankerInfo.Score;
	PlayerScore = pRoom->PlayerInfo.Score;

	// ����° ī�带 ���� �ƴϸ� ������ �ٷ� ������ �����Ͽ� �����ش�.
	if( BankerScore >= EIGHTCARD || PlayerScore >= EIGHTCARD )
	{
		NotifyGameResult( pRoom );
	}
	else	// �÷��̾ ��Ŀ�� ����° ī�带 ���� �� �ִٸ�
	{
		char	P_threecard = NOTUSECARD;	// �÷��̾��� ����° ī��
		char	B_threecard = NOTUSECARD;	// ��Ŀ�� ����° ī��

		packetcoder.SetBuf( cPacket );

		// �÷��̾�� ����° ī�带 ���� �� �ֳ�
		if( ServerContext.gameproc->CanPlayerReceiveThreedCard( PlayerScore ) == TRUE )
		{
			P_threecard = ServerContext.gameproc->GetPrivateCard( pRoom );
			pRoom->PlayerInfo.Cards[2] = P_threecard;
			pRoom->PlayerInfo.Score += CardToNumber[ P_threecard ];
			if( pRoom->PlayerInfo.Score >= 10 ) pRoom->PlayerInfo.Score %= 10;
		}
        packetcoder.PutChar( P_threecard );
	

		// ��Ŀ�� ����° ī�带 ���� �� �ֳ� ?
		if( ServerContext.gameproc->CanBankerReceiveThreeCard( P_threecard, BankerScore ) == TRUE )
		{
			B_threecard = ServerContext.gameproc->GetPrivateCard( pRoom );
			pRoom->BankerInfo.Cards[2] = B_threecard;
			pRoom->BankerInfo.Score += CardToNumber[ B_threecard ];
			if( pRoom->BankerInfo.Score >= 10 ) pRoom->BankerInfo.Score %= 10;
		}
        packetcoder.PutChar( B_threecard );
		

		// ����° ī�带 ������.
		iSize = packetcoder.SetHeader( ANSWER_SENDTHREECARD );
		PostTcpSend( pRoom->GetUserBegin(), cPacket, iSize );

		// ����� �뺸 �Ѵ�.
		NotifyGameResult( pRoom );
	}

}


BOOL CGame::CanPlayerReceiveThreedCard( char PlayerScore )
{
	if( PlayerScore <= 5 ) 
		return TRUE;
	else
		return FALSE;
}



BOOL CGame::CanBankerReceiveThreeCard( char Playerthreecard, char BankerScore )
{
	if( BankerScore >= 0 && BankerScore <= 2 )
	{
		return TRUE;
	}
	else
	{
		if( Playerthreecard == NOTUSECARD ) return FALSE;

		switch ( BankerScore )
		{
		case 3:
			if( Playerthreecard != 8 ) return TRUE;
			break;
		case 4:
			if( Playerthreecard >= 2 && Playerthreecard <= 7 ) return TRUE;
			break;
		case 5:
			if( Playerthreecard >= 4 && Playerthreecard <= 7 ) return TRUE;
			break;
		case 6:
			if( Playerthreecard == 6 || Playerthreecard == 7 ) return TRUE;
			break;
		}
	}

	return FALSE;
}


void CGame::NotifyGameResult(CRoom *pRoom)
{
	CPacketCoder	packetcoder;
	LPSOCKETCONTEXT lpSC;
	char			cPacket[64] = {0,};
	char			BankerScore = pRoom->BankerInfo.Score;
	char			PlayerScore = pRoom->PlayerInfo.Score;
	char			area;
	char			nUserNum = pRoom->GetCurUserNum();
	int				iSize;
	int				iNext = pRoom->GetUserBegin();


	pRoom->m_iState	= GAME_SCORECALCULATE;
	time( &pRoom->m_tRoomTime );


	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( NOTLINKED );
	packetcoder.PutChar( BankerScore );
	packetcoder.PutChar( PlayerScore );
	packetcoder.PutChar( nUserNum );


	if( BankerScore == PlayerScore )	// ��Ŀ�� �÷��̾��� ������ ����. �� Ÿ�̰� �̱�.
	{	
		area = _TIE;
		CopyMemory( cPacket + HEADERSIZE, &area, sizeof(char) );


		while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
		{
			lpSC = &ServerContext.sc[ iNext ];

			if( lpSC->nSelectArea == _TIE )		// ������ Ÿ�̸� �����ߴٸ�
			{
                lpSC->ibetMoney *= TIEWIN_MULTIPLE;
				ServerContext.db->SaveUserData( lpSC, FALSE );

				packetcoder.PutInt( lpSC->iUserDBIndex );
				packetcoder.PutInt( lpSC->ibetMoney );
			}
			else	// ������ Ÿ�̸� �������� �ʾҴٸ�
			{
				lpSC->ibetMoney = -( lpSC->ibetMoney ); 

				ServerContext.db->SaveUserData( lpSC, FALSE );
				packetcoder.PutInt( lpSC->iUserDBIndex );
				packetcoder.PutInt( lpSC->ibetMoney );
			}

			iNext = ServerContext.pn[ iNext ].next;
		}
	}
	else if( BankerScore > PlayerScore )	// ��Ŀ�� �÷��̾ �̱� ���.
	{
		area = _BANKER;
		CopyMemory( cPacket + HEADERSIZE, &area, sizeof(char) );

		while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
		{
			lpSC = &ServerContext.sc[ iNext ];

			if( lpSC->nSelectArea == _BANKER )	// ������ ��Ŀ�� �����ߴ�.
			{
				lpSC->ibetMoney -= static_cast<int>( ( lpSC->ibetMoney * COMMISSION ) );
				ServerContext.db->SaveUserData( lpSC, FALSE );
				
				packetcoder.PutInt( lpSC->iUserDBIndex );
				packetcoder.PutInt( lpSC->ibetMoney );
			}
			else								// ������ ��Ŀ�� �������� �ʾҴ�.		
			{
				lpSC->ibetMoney = -( lpSC->ibetMoney );
				ServerContext.db->SaveUserData( lpSC, FALSE );
				packetcoder.PutInt( lpSC->iUserDBIndex );
				packetcoder.PutInt( lpSC->ibetMoney );
			}
			iNext = ServerContext.pn[ iNext ].next;
		}
	}
	else								// �÷��̾ ��Ŀ�� �̱� ���.
	{
		area = _PLAYER;
		CopyMemory( cPacket + HEADERSIZE, &area, sizeof(char) );

		while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
		{
			lpSC = &ServerContext.sc[ iNext ];

			if( lpSC->nSelectArea == _PLAYER )		//  ������ �÷��̾����� �����ߴ�.
			{
				ServerContext.db->SaveUserData( lpSC, FALSE );
				
				packetcoder.PutInt( lpSC->iUserDBIndex );
				packetcoder.PutInt( lpSC->ibetMoney );
			}
			else									// ������ �÷��̾����� �������� �ʾҴ�.
			{
				lpSC->ibetMoney = -( lpSC->ibetMoney ); 
				ServerContext.db->SaveUserData( lpSC, FALSE );
				packetcoder.PutInt( lpSC->iUserDBIndex );
				packetcoder.PutInt( lpSC->ibetMoney );
			}
			iNext = ServerContext.pn[ iNext ].next;
		}
	}

	iSize = packetcoder.SetHeader( ANSWER_RESULT );
	PostTcpSend( pRoom->GetUserBegin(), cPacket, iSize );

#ifdef _LOGFILELEVEL3_
	ServerUtil.GameResultLog( pRoom->m_iChannel, pRoom->m_RoomNum );
#endif

	// �� ���� ���� ������ ���� ��Ű��, ������ ���� ������ ���� ������ �Ѵ�.
	ResetUserState( pRoom );

	// �ٽ� ������ �� �� �ִ� ���·� �����.
	SendGameOff( pRoom );
}



void CGame::ResetUserState(CRoom *pRoom)
{
	LPSOCKETCONTEXT lpSC;
	CPacketCoder	packetcoder;
	int				iNext = pRoom->GetUserBegin();
	
	pRoom->m_iState = GAME_OFF;

	while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
	{
		lpSC = &ServerContext.sc[ iNext ];
		iNext = ServerContext.pn[ iNext ].next;

		if( lpSC->bAgency == FALSE && lpSC->cState != GAME_NOTMONEY )
			lpSC->cState = GAME_OFF;

		// �� ���� �����ڴ�  �i�� ����.
		if( lpSC->bAgency == TRUE )	
		{
			EnqueueClose( lpSC );
		}
		else // ���� ���� ����� ������� �������� �뺸�Ѵ�.
		if( lpSC->cState == GAME_NOTMONEY )
		{
			lpSC->cState = GAME_OFF;
			OnRequestExitRoom( lpSC, NotMoneyPacket );
		}
		else // ������ �����̶��
		if( lpSC->bReservation == TRUE )		
		{
			lpSC->bReservation = FALSE;
			PostTcpSend( 1, (int*)&lpSC, ReservateExitRoomPacket, HEADERSIZE );
		}
	}

#ifdef _LOGFILELEVEL3_
	ServerUtil.GameLog( pRoom->m_iChannel, pRoom->m_RoomNum, strResetUserState );
#endif

}



void CGame::SendGameOff( CRoom* pRoom )
{
	NotifyRoomState( pRoom->m_iChannel, pRoom->m_RoomNum, GAME_OFF );


	if( pRoom->GetCurUserNum() >= MINPLAYER )
	{
		pRoom->m_iState = GAME_READY;
		time( &pRoom->m_tRoomTime );
	}

#ifdef _LOGFILELEVEL3_
	ServerUtil.GameLog( pRoom->m_iChannel, pRoom->m_RoomNum, strSendGameOff );
#endif
}



void CGame::NotifyRoomState( int iChannelIndex, char RoomNum, char state )
{
	
	CPacketCoder	packetcoder;
	CChannel*	pChannel = &ServerContext.ch[ iChannelIndex ];
	CRoom*		pRoom	= NULL;
	char		cPacket[16] = { 0, };
	int			iSize, iNext;


	// �κ� �ִ� �������� ���� ���¸� �˷��ش�.
	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( RoomNum );
	packetcoder.PutChar( state );
	iSize = packetcoder.SetHeader( ANSWER_NOTIFYROOMSTATE );
	PostTcpSend( pChannel->GetBeginUser(), cPacket, iSize );

	// ä���� �뿡 �ִ� �������� �˷��ش�.
	iNext = pChannel->GetBeginRoom();
	while( iNext > NOTLINKED && iNext < ServerContext.iMaxRoom )
	{
		pRoom = &ServerContext.rm[ iNext ];

		if( pRoom->m_RoomNum != RoomNum )
            PostTcpSend( pRoom->GetUserBegin(), cPacket, iSize );

		iNext = ServerContext.rn[ iNext ].next;
	}

#ifdef _LOGFILELEVEL3_
	ServerUtil.NotifyRoomStateLog( pRoom->m_iChannel, pRoom->m_RoomNum, pRoom->m_iState );
#endif
}


char CGame::GetPrivateCard( CRoom* pRoom )
{
	char card;
	 
	++pRoom->m_iIndexprivatecard;
	
	card = pRoom->m_privatecard[ pRoom->m_iIndexprivatecard ];

	return card;
}

void CGame::BackGameInRoomState( CRoom* pRoom )
{
	LPSOCKETCONTEXT lpSC; 
	int iNext = pRoom->GetUserBegin();

	pRoom->m_iState = GAME_OFF;

	while( iNext >= 0 && iNext < ServerContext.iMaxUserNum )
	{
		lpSC = &ServerContext.sc[ iNext ];
		if( lpSC->idLen == 0 ) break;

		iNext = ServerContext.pn[ iNext ].next;

		if( lpSC->cState != GAME_OFF ) lpSC->cState = GAME_OFF;
	}

#ifdef _LOGFILELEVEL3_
	ServerUtil.GameLog( pRoom->m_iChannel, pRoom->m_RoomNum, strBackGameInRoomState );
#endif

	CheckChangeStateRoom( pRoom );
}