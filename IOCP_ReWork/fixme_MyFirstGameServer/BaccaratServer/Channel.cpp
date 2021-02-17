// Channel.cpp: implementation of the CChannel class.
//
//////////////////////////////////////////////////////////////////////
#define _WIN32_WINNT 0x0500

#include "protocol_baccarat.h"
#include "server.h"
#include "database.h"
#include "serverprocess.h"
#include "packetcoder.h"
#include "serverutil.h"
#include "sock.h"
#include "Channel.h"
#include <wchar.h>


CChannel::CChannel()
{

}

CChannel::~CChannel()
{

}

int InitChannelLayer()
{
	int		iN, iNum;

	iNum = ServerContext.iMaxProcess * ServerContext.iMaxChannelInProcess;
	ServerContext.ch = new CChannel[ iNum ];
	if( ServerContext.ch == NULL ) return 0;

	for( iN = 0; iN < iNum; ++iN )
	{
		ServerContext.ch[ iN ].InitChannel( iN );
	}

	
	OnTransFunc[ REQUEST_JOINCHANNEL ].proc			 = OnRequestJoinChannel;
	OnTransFunc[ REQUEST_CHANGE_CHANNEL ].proc		 = OnRequestChangeChannel;
	OnTransFunc[ REQUEST_CHATINLOBBY ].proc			 = OnRequestChatInLobby;
	OnTransFunc[ REQUEST_WHISPER ].proc				 = OnRequestWhisper;
	OnTransFunc[ REQUEST_ALLINVITE_REJECT ].proc     = OnRequestAllInviteReject;
	OnTransFunc[ REQUEST_ALLCHAT_REJECT ].proc		 = OnRequestAllChatReject;
	OnTransFunc[ REQUEST_ROOMPASSWORDA ].proc		 = OnRequestRoomPWD;
	OnTransFunc[ REQUEST_ROOMPASSWORD_RESULTB ].proc = OnRequestRoomPWD_Result;
	OnTransFunc[ REQUEST_1ON1CHAT_DEMANDA ].proc     = OnRequest1ON1Chat_Demand;
	OnTransFunc[ REQUEST_1ON1CHAT_RESULTB ].proc     = OnRequest1ON1Chat_Result;
	OnTransFunc[ REQUEST_1ON1CHAT ].proc			 = OnRequest1ON1Chat;
	OnTransFunc[ REQUEST_1ON1CHAT_CLOSE ].proc		 = OnRequest1ON1Chat_Close;
	OnTransFunc[ REQUEST_RELOADUSERITEM ].proc		 = OnRequestReLoadItem;

	return 1;
}


void CChannel::InitChannel(int idx)
{
	int		iN, dummy;

	m_iIndex = idx;
	m_iBaseRoomIndex = idx * ServerContext.iMaxRoomInChannel;

	// ä���� ����� �����Ѵ�.
	if( idx < ServerContext.cChannelLowNum ) 
	{
		m_nLevel = LEVEL_LOW;
	}
	else if( idx < ( ServerContext.cCnannelMiddleNum + ServerContext.cChannelLowNum ) )
	{
		m_nLevel = LEVEL_MIDDLE;
	}
	else
	{
		m_nLevel = LEVEL_HIGH;
	}


	m_iUserBegin	= NOTLINKED;
	m_iUserEnd		= NOTLINKED;
	m_iUserNum		= 0;

	m_iUsedRoomBegin = NOTLINKED;
	m_iUsedRoomEnd   = NOTLINKED;
	m_iUsedRoomNum	 = 0;

	m_iUnusedRoomBegin = NOTLINKED;
	m_iUnusedRoomEnd   = NOTLINKED;
	m_iUnusedRoomNum   = 0;

	// ó������ ��� ��밡���� ������ �ʱ�ȭ �Ѵ�.
	for( iN = 0; iN < ServerContext.iMaxRoomInChannel; ++iN )
	{
		dummy = m_iBaseRoomIndex + iN;
		if( m_iUnusedRoomNum == 0 )
		{
			ServerContext.rn[ dummy ].prev = NOTLINKED;
			m_iUnusedRoomBegin = m_iUnusedRoomEnd = dummy;
		}
		else
		{
			ServerContext.rn[ m_iUnusedRoomEnd ].next = dummy;
			ServerContext.rn[ dummy ].prev = m_iUnusedRoomEnd;
			m_iUnusedRoomEnd = dummy;
		}

		ServerContext.rn[ dummy ].next = NOTLINKED;
		++m_iUnusedRoomNum;
	}

}


void CChannel::SetUserLink(int idx)
{
	if( m_iUserNum == 0 )
	{
		ServerContext.pn[ idx ].prev = NOTLINKED;
		m_iUserBegin = m_iUserEnd = idx;
	}
	else
	{
		ServerContext.pn[ m_iUserEnd ].next = idx;
		ServerContext.pn[ idx ].prev = m_iUserEnd;
		m_iUserEnd = idx;
	}

	ServerContext.pn[ idx ].next = NOTLINKED;
	++m_iUserNum;
}

void CChannel::KillUserLink(int idx)
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

void CChannel::SetRoomLink(int idx)
{
	// ������� ���� �� ����Ʈ�� ���� ����
	DeleteUnusedRoomList( idx );

	ServerContext.rn[ idx ].next = NOTLINKED;
	ServerContext.rn[ idx ].prev = NOTLINKED;
	--m_iUnusedRoomNum;

	// ����� �� ����Ʈ�� �߰�
	AddUsedRommList( idx );

	ServerContext.rn[ idx ].next = NOTLINKED;
	++m_iUsedRoomNum;
}

void CChannel::DeleteUnusedRoomList(int idx)
{
	int			prev, next;

	prev = ServerContext.rn[ idx ].prev;
	next = ServerContext.rn[ idx ].next;

	if( m_iUnusedRoomNum == 1 )
	{
		m_iUnusedRoomBegin = NOTLINKED;
		m_iUnusedRoomEnd   = NOTLINKED;
	}
	else
	{
		ServerContext.rn[ next ].prev = NOTLINKED;
		m_iUnusedRoomBegin = next;
	}
}

void CChannel::AddUsedRommList(int idx)
{
	if( m_iUsedRoomNum == 0 )
	{
		ServerContext.rn[ idx ].prev = NOTLINKED;
		m_iUsedRoomBegin = m_iUsedRoomEnd = idx;
	}
	else
	{
		ServerContext.rn[ m_iUsedRoomEnd ].next = idx;
		ServerContext.rn[ idx ].prev = m_iUsedRoomEnd;
		m_iUsedRoomEnd = idx;
	}
}

void CChannel::KillRoomLink(int idx)
{
	// ����� �� ����Ʈ�� ���� ����	
	DeleteRoomLink( idx );
	
	ServerContext.rn[ idx ].next = NOTLINKED;
	ServerContext.rn[ idx ].prev = NOTLINKED;
	--m_iUsedRoomNum;

	// ������� ���� �� ����Ʈ�� �߰�
	if( m_iUnusedRoomNum == 0 )
	{
		ServerContext.rn[ idx ].prev = NOTLINKED;
		m_iUnusedRoomBegin = m_iUnusedRoomEnd = idx;
	}
	else
	{
		ServerContext.rn[ m_iUnusedRoomEnd ].next = idx;
		ServerContext.rn[ idx ].prev = m_iUnusedRoomEnd;
		m_iUnusedRoomEnd = idx;
	}

	ServerContext.rn[ idx ].next = NOTLINKED;
	++m_iUnusedRoomNum;
}

void CChannel::DeleteRoomLink(int idx)
{
	int		prev, next;

	prev = ServerContext.rn[ idx ].prev;
	next = ServerContext.rn[ idx ].next;

	if( m_iUsedRoomNum == 1 )
	{
		m_iUsedRoomBegin = NOTLINKED;
		m_iUsedRoomEnd   = NOTLINKED;
	}
	else if( idx == m_iUsedRoomBegin )
	{
		ServerContext.rn[ next ].prev = NOTLINKED;
		m_iUsedRoomBegin = next;
	}
	else if( idx == m_iUsedRoomEnd )
	{
		ServerContext.rn[ prev ].next = NOTLINKED;
		m_iUsedRoomEnd = prev;
	}
	else
	{
		ServerContext.rn[ prev ].next = next;
		ServerContext.rn[ next ].prev = prev;
	}
}



int OnRequestJoinChannel( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	char	cPacket[ 16 ] = { 0, };
	char	cChannelNum;
	int		iSize;
	BOOL	bLevel = TRUE;
	CPacketCoder	packetcoder;
	CChannel*		pChannel;

	// ������ ä�� ���� ��ġ�� �ƴ϶�� �߸� �� ��ɾ� �̴�.  �����Ѵ�.
	if( lpSockContext->cPosition != WH_CHANNELSELECT ) return 0;

	packetcoder.SetBuf( cpPacket );
	packetcoder.GetChar( &cChannelNum );

	if( cChannelNum < 0 || cChannelNum >= ServerContext.iMaxChannel ) return 0;
	pChannel = &ServerContext.ch[cChannelNum ];

	// ������ ������ �� ä���� ������ ��� �� �� �ִ��� Ȯ���Ѵ�.
	if( lpSockContext->bCanJumpItem == FALSE && ( lpSockContext->GameState.level / 4 < pChannel->m_nLevel ) )
		bLevel = FALSE;
	

	packetcoder.SetBuf( cPacket );
	
	if ( bLevel == FALSE )
	{
		packetcoder.PutChar( JOINCHANNEL_FAIL_NOTLEVEL );
		iSize = packetcoder.SetHeader( ANSWER_JOINCHANNEL );
		PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );
	}
	else if( ServerContext.ch[ cChannelNum ].GetCurUserNum() < ServerContext.iMaxUserInChannel )
	{
		lpSockContext->iChannel = cChannelNum;

		packetcoder.PutChar( JOINCHANNEL_SUCCESS );
		packetcoder.PutChar( cChannelNum );
		packetcoder.PutChar( static_cast<char>( ServerContext.iMaxRoomInChannel ) );
		iSize = packetcoder.SetHeader( ANSWER_JOINCHANNEL );
		PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );
		
		OnRequestJoinChannelInt( lpSockContext, cpPacket );
	}
	else
	{
		packetcoder.PutChar( JOINCHANNEL_FAIL_FULL );
		iSize = packetcoder.SetHeader( ANSWER_JOINCHANNEL );
		PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );
	}
	
#ifdef _LOGFILELEVEL3_
	ServerUtil.JoinChannelLog( lpSockContext->iChannel, pChannel->m_iUserNum, lpSockContext->cID );
#endif

	return 1;
}


int OnRequestJoinChannelInt( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder		packetcoder;
	CChannel			*pChannel = &ServerContext.ch[ lpSockContext->iChannel ];
	CRoom				*pRoom;
	char				cPacket[ 64 ] = { 0, };
	int					iSize, iNext;
	

#ifdef _LOGCHANNEL_
	ServerUtil.ConsoleOutput( "OnRequestJoinChannelInt(%d) : %d\n", lpSockContext->index,
										lpSockContext->iChannel );
#endif

	// �� ����Ʈ ����
	OnNotifyRoomList( lpSockContext, NULL );

	// ���� ����Ʈ ����
	OnNotifyUserList( lpSockContext, NULL );

	// ������ ��ũ�� ����
	pChannel->SetUserLink( lpSockContext->index );
	lpSockContext->cPosition = WH_LOBBY;

#ifdef _LOGCHANNEL_
	ServerUtil.ConsoleOutput( "Channel UserNum : %d\n", pChannel->m_iUserNum );
#endif

	// ä���� Ÿ �������� ������ ������ ����
	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( 1 );
	packetcoder.PutChar( lpSockContext->idLen );    
	packetcoder.PutText( lpSockContext->cID, lpSockContext->idLen );
	packetcoder.PutChar( WH_LOBBY );
	packetcoder.PutChar( lpSockContext->UserInfo.nicLen);
	packetcoder.PutText( lpSockContext->UserInfo.NicName, lpSockContext->UserInfo.nicLen );
	packetcoder.PutChar( lpSockContext->UserInfo.sex );
	packetcoder.PutInt( lpSockContext->iUserDBIndex );
	packetcoder.PutChar( lpSockContext->GameState.level );
	packetcoder.PutChar( NOTLINKED );
	packetcoder.PutChar( NOTLINKED );

	iSize = packetcoder.SetHeader( ANSWER_NOTIFY_USERLIST );
	PostTcpSend( pChannel->m_iUserBegin, cPacket, iSize );

	iNext = pChannel->m_iUsedRoomBegin;
	while( iNext > NOTLINKED && iNext < ServerContext.iMaxRoom )
	{
		pRoom = &ServerContext.rm[ iNext ];
		PostTcpSend( pRoom->GetUserBegin(), cPacket, iSize );
		iNext = ServerContext.rn[ iNext ].next;
	}

	return 1;
}


int OnRequestChangeChannel( LPSOCKETCONTEXT lpSockContext, char* cpPacket)
{
	CPacketCoder		packetcoder;
	CChannel			*pChannel = &ServerContext.ch[ lpSockContext->iChannel ];
	int					iChannelNum = lpSockContext->iChannel;
	CRoom				*pRoom;
	char				cPacket[ 32 ] = { 0, }; 
	int					iNext, iSize;  

	// ������ ��ġ�� �κ� �ƴ϶�� �߸��� ��ɾ� �̴�.
	if( lpSockContext->cPosition != WH_LOBBY ) return 0;

	// ���� ��ũ ����
	pChannel->KillUserLink( lpSockContext->index );
	lpSockContext->cPosition = WH_CHANNELSELECT;
	lpSockContext->iChannel = NOTLINKED;
	
	// ���� ������ �뺸
	packetcoder.SetBuf( cPacket );
	packetcoder.PutInt( lpSockContext->iUserDBIndex );
	iSize = packetcoder.SetHeader( ANSWER_NOTIFY_USERDELETE );
	PostTcpSend( pChannel->m_iUserBegin, cPacket, iSize );

	// �뿡 �ִ� �������Ե� �뺸�Ѵ�.
	iNext = pChannel->m_iUsedRoomBegin;
	while( iNext > NOTLINKED && iNext < ServerContext.iMaxRoom )
	{
		pRoom = &ServerContext.rm[ iNext ];
		PostTcpSend( pRoom->GetUserBegin(), cPacket, iSize );
		iNext = ServerContext.rn[ iNext ].next;
	}

	// ä���� ������ �������� ä���� ���Դٰ� �����ش�.
	packetcoder.SetBuf( cPacket );
	iSize = packetcoder.SetHeader( ANSWER_CHANGE_CHANNEL );
	PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );

#ifdef _LOGCHANNEL_
	ServerUtil.ConsoleOutput( "OnRequestChangeChannel(%d) : %d\n", 
				lpSockContext->index, lpSockContext->cPosition );
#endif
#ifdef _LOGFILELEVEL3_
	ServerUtil.ExitChannelLog( iChannelNum, lpSockContext->cID );
#endif

	return 1;
}


int OnRequestChatInLobby( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CChannel		*pChannel = &ServerContext.ch[ lpSockContext->iChannel ];
	short			dummy;

	if( lpSockContext->cPosition != WH_LOBBY ) return 0;

	dummy = ANSWER_CHATINLOBBY;
	CopyMemory( cpPacket + sizeof(short), &dummy, sizeof(short) );
	CopyMemory( &dummy, cpPacket, sizeof(short) );

	if( dummy > MAXCHATPACKETLENGTH ) return 0;

	PostTcpSend( pChannel->m_iUserBegin, cpPacket, dummy + HEADERSIZE );

#ifdef _LOGCHANNEL_
	ServerUtil.ConsoleOutput( "OnRequestChatInLobby(%d) : %s\n", lpSockContext->index, cpPacket+5 );
#endif

	return 1;
}


int OnRequestWhisper( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder	packetcoder;
	LPSOCKETCONTEXT lpSC;
	int				iSize, iToDBIndex, iToIndex;
	char			cToIdLen; 
	char			cPacket[ 10 ] = { 0, };
	short			dummy = ANSWER_WHISPER;

	iSize = cToIdLen = 0;
	iToDBIndex = iToIndex = NOTLINKED;

	packetcoder.SetBuf( cpPacket );
	packetcoder.GetInt( &iToDBIndex );

	iToIndex = ServerContext.ch[ lpSockContext->iChannel ].GetDBIndexToSockIndex( iToDBIndex );
	if( iToIndex == NOTLINKED )
	{
		packetcoder.SetBuf( cPacket );
		iSize = packetcoder.SetHeader( ANSWER_WHISPER_FAIL );
		PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );
		return 0;
	}

	lpSC = &ServerContext.sc[ iToIndex ];
	if( lpSC == NULL ) return 0;

	CopyMemory( cpPacket + sizeof(short), &dummy, sizeof(short) );
	CopyMemory( &dummy, cpPacket, sizeof(short) );

	if( dummy > MAXCHATPACKETLENGTH ) return 0;

	PostTcpSend( 1, (int*)&lpSockContext, cpPacket, dummy + HEADERSIZE );
	PostTcpSend( 1, (int*)&lpSC, cpPacket, dummy + HEADERSIZE );
	return 1;
}


int OnNotifyRoomList( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CChannel		*pChannel = &ServerContext.ch[ lpSockContext->iChannel ];
	CRoom			*pRoom;
	int				iNext, iSize;
	CPacketCoder	packetcoder;
	char			cPacket[ MAXSENDPACKSIZE ] = { 0, };
	char			cRoomNum;

#ifdef _LOGCHANNEL_
	ServerUtil.ConsoleOutput( "OnNotifyRoomList(%d)\n", lpSockContext->index );
#endif

	cRoomNum = 0;
	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( 0 );

	iNext = pChannel->m_iUsedRoomBegin;
	while( iNext > NOTLINKED && iNext < ServerContext.iMaxRoom )
	{
		pRoom = &ServerContext.rm[ iNext ];
		iNext = ServerContext.rn[ iNext ].next;

		if( pRoom->GetCurUserNum() <= 0 )
		{
			pChannel->KillRoomLink( pRoom->GetRoomIndex() );
			continue;
		}

		packetcoder.PutChar( pRoom->m_RoomNum );
		packetcoder.PutChar( pRoom->m_cOwnerLen );
		packetcoder.PutText( pRoom->m_cOwner, pRoom->m_cOwnerLen );
		packetcoder.PutChar( pRoom->m_cTitleLen );
		packetcoder.PutText( pRoom->m_cTitle, pRoom->m_cTitleLen );
		packetcoder.PutChar( pRoom->m_iUserNum );
		packetcoder.PutChar( pRoom->m_iType );
		packetcoder.PutChar( pRoom->m_iState );
		
		
		++cRoomNum;
		if( cRoomNum >= ROOMPERPACKET )
		{
			cPacket[ 4 ] = cRoomNum;
			iSize = packetcoder.SetHeader( ANSWER_NOTIFY_ROOMLIST );
			PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );

			cRoomNum = 0;
			packetcoder.SetBuf( cPacket );
			packetcoder.PutChar( 0 );
		}
		else if( iNext == NOTLINKED )
		{
			cPacket[ 4 ] = cRoomNum;
			iSize = packetcoder.SetHeader( ANSWER_NOTIFY_ROOMLIST );
			PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );
		}
	}
	
	return 1;
}


int OnNotifyUserList( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CChannel		*pChannel = &ServerContext.ch[ lpSockContext->iChannel ];
	CRoom			*pRoom;
	CPacketCoder	packetcoder;
	LPSOCKETCONTEXT lpSC;
	int				iNext, iRoomNext, iSize;
	char			cPacket[ MAXSENDPACKSIZE ] = { 0, };
	char			cUserNum, cRoomNum;

#ifdef _LOGCHANNEL_
	ServerUtil.ConsoleOutput( "OnNotifyUserList(%d)\n", lpSockContext->index );
#endif

	cUserNum = 0;

	// �κ� �ִ� ����� ���� ����
	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( 0 );

	iNext = pChannel->m_iUserBegin;
	while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
	{
		lpSC = &ServerContext.sc[ iNext ];
		iNext = ServerContext.pn[ iNext ].next;

		if( lpSC->iUserDBIndex <= 0 || lpSC->idLen <= 0 )
		{
			pChannel->KillUserLink( lpSC->index );
			continue;
		}

		packetcoder.PutChar( lpSC->idLen );
		packetcoder.PutText( lpSC->cID, lpSC->idLen );
		packetcoder.PutChar( WH_LOBBY );
		packetcoder.PutChar( lpSC->UserInfo.nicLen);
		packetcoder.PutText( lpSC->UserInfo.NicName, lpSC->UserInfo.nicLen );
		packetcoder.PutChar( lpSC->UserInfo.sex );
		packetcoder.PutInt( lpSC->iUserDBIndex );
		packetcoder.PutChar( lpSC->GameState.level );
		packetcoder.PutChar( NOTLINKED );
		packetcoder.PutChar( NOTLINKED );

		++cUserNum;
		if( cUserNum >= USERPERPACKET )
		{
			cPacket[ 4 ] = cUserNum;
			iSize = packetcoder.SetHeader( ANSWER_NOTIFY_USERLIST );
			PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );

			cUserNum = 0;
			packetcoder.SetBuf( cPacket );
			packetcoder.PutChar( 0 );
		}
	}

	// �� �뿡 �ִ� ����� ���� ����
	iRoomNext = pChannel->m_iUsedRoomBegin;
	while( iRoomNext > NOTLINKED && iRoomNext < ServerContext.iMaxRoom )
	{
		pRoom = &ServerContext.rm[ iRoomNext ];
		iRoomNext = ServerContext.rn[ iRoomNext ].next;
		cRoomNum = NOTLINKED;

		// �뿡 ����ڰ� ���� ���.
		iNext = pRoom->m_iUserBegin;
		while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
		{
			lpSC = &ServerContext.sc[ iNext ];
			iNext = ServerContext.pn[ iNext ].next ;

			if( lpSC->iUserDBIndex <= 0 || lpSC->idLen <= 0 )
			{
				pRoom->KillUserLink( lpSC->index );
				continue;
			}
			
			if( cRoomNum == NOTLINKED )
				cRoomNum = static_cast<char>( lpSC->iRoom - ( lpSC->iChannel * ServerContext.iMaxRoomInChannel ) );
			
			packetcoder.PutChar( lpSC->idLen );
			packetcoder.PutText( lpSC->cID, lpSC->idLen );
			packetcoder.PutChar( WH_ROOM );
			packetcoder.PutChar( lpSC->UserInfo.nicLen);
			packetcoder.PutText( lpSC->UserInfo.NicName, lpSC->UserInfo.nicLen );
			packetcoder.PutChar( lpSC->UserInfo.sex );
			packetcoder.PutInt( lpSC->iUserDBIndex );
			packetcoder.PutChar( lpSC->GameState.level );
			packetcoder.PutChar( cRoomNum );
			packetcoder.PutChar( lpSC->iOrderInRoom );
			
			++cUserNum;
			if( cUserNum >= USERPERPACKET )
			{
				cPacket[ 4 ] = cUserNum;
				iSize = packetcoder.SetHeader( ANSWER_NOTIFY_USERLIST );
				PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );

				cUserNum = 0;
				packetcoder.SetBuf( cPacket );
				packetcoder.PutChar( 0 );
			}
		}
	}

	// �������� �ִٸ� ����
	if( cUserNum > 0 )
	{
		cPacket[ 4 ] = cUserNum;
		iSize = packetcoder.SetHeader( ANSWER_NOTIFY_USERLIST );
		PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );
	}

	return 1;
}



int OnRequestAllInviteReject( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	short			dummy;
	char			result;


	CopyMemory( &result, cpPacket + HEADERSIZE, sizeof(char) );
	if( result == 1 )
		lpSockContext->bAllInvite = TRUE;
	else
		lpSockContext->bAllInvite = FALSE;

	dummy = ANSWER_ALLINVITE_REJECT;
	CopyMemory( cpPacket + sizeof(short), &dummy, sizeof(short) );
	CopyMemory( &dummy, cpPacket, sizeof(short) );
	PostTcpSend( 1, (int*)&lpSockContext, cpPacket, dummy + HEADERSIZE );

#ifdef _LOGCHANNEL_
	ServerUtil.ConsoleOutput( "OnRequestAllInviteReject(%d) : %d\n", 
						lpSockContext->index, lpSockContext->bAllInvite );
#endif
	
	return 1;
}


int OnRequestAllChatReject( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	short			dummy;
	char			result;


	CopyMemory( &result, cpPacket + HEADERSIZE, sizeof(char) );
	if( result == 1 )
		lpSockContext->bALLChat = TRUE;
	else
		lpSockContext->bALLChat = FALSE;

	dummy = ANSWER_ALLCHAT_REJECT;
	CopyMemory( cpPacket + sizeof(short), &dummy, sizeof(short) );
	CopyMemory( &dummy, cpPacket, sizeof(short) );
	PostTcpSend( 1, (int*)&lpSockContext, cpPacket, dummy + HEADERSIZE );

#ifdef _LOGCHANNEL_
	ServerUtil.ConsoleOutput( "OnRequestAllChatReject(%d) : %d\n", 
						lpSockContext->index, lpSockContext->bAllInvite );
#endif

	return 1;
}


int OnRequestRoomPWD( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	char			cRoomNum, cChannelNum;
	char			cPacket[ 32 ] = { 0, };
	int				iSize, iRoomNum;
	LPSOCKETCONTEXT	lpSC;
	CPacketCoder	packetcoder;
	char			cResult = ROOMPASSWORD_SUCCESS;

	packetcoder.SetBuf( cpPacket );
	packetcoder.GetChar( &cRoomNum );

	if( cRoomNum < 0 || cRoomNum >=  ServerContext.iMaxRoomInChannel ) return 0;
	
	cChannelNum = lpSockContext->iChannel;
	iRoomNum = (cChannelNum * ServerContext.iMaxRoomInChannel) + cRoomNum;

	if( ServerContext.rm[ iRoomNum ].GetRoomType() == PUBLICTYPE ) return 0;
	
	lpSC = &ServerContext.sc[ ServerContext.rm[ iRoomNum ].GetOwnerIndex() ];
	if( lpSC == NULL ) return 0;

	if( lpSC->iUserDBIndex == NOTLOGINUSER )
		cResult = ROOMPASSWORD_FAIL_NOROOM;

	else if( ServerContext.rm[ iRoomNum ].GetCurUserNum() == 0 ) 
		cResult = ROOMPASSWORD_FAIL_NOROOM;
	
	else if( ServerContext.rm[ iRoomNum ].GetCurUserNum() >= ServerContext.iMaxUserInRoom )
		cResult = ROOMPASSWORD_FAIL_FULLUSER;
	
	else if( lpSC->bAllInvite == FALSE )
		cResult = ROOMPASSWORD_FAIL_NOINJECT;


	packetcoder.SetBuf( cPacket );
	if( cResult > ROOMPASSWORD_SUCCESS )
	{
		packetcoder.PutChar( cResult );
		iSize = packetcoder.SetHeader( ANSWER_ROOMPASSWORD_RESULTA );
		PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );
		return 1;
	}

	packetcoder.PutInt( lpSockContext->iUserDBIndex );
	iSize = packetcoder.SetHeader( ANSWER_ROOMPASSWORDB );
	PostTcpSend( 1, (int*)&lpSC, cPacket, iSize );

#ifdef _LOGCHANNEL_
	ServerUtil.ConsoleOutput( " OnRequestRoomPWD(%d) : result(%d), DBIndex(%d)\n",
					lpSockContext->index, cResult, lpSockContext->iUserDBIndex );
#endif

	return 1;
}


int OnRequestRoomPWD_Result( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	int				iDBIndex, iSize, iIndex;
	int				iRoomNum;
	char			cChannelNum = lpSockContext->iChannel;
	char			cPacket[ 32 ] = { 0, };
	char			cRoomNum, cResult, cPWDLen, cPWD[ 32 ] = { 0, };
	LPSOCKETCONTEXT lpSC;
	CPacketCoder	packetcoder;

	packetcoder.SetBuf( cpPacket );
	packetcoder.GetChar( &cResult );
	packetcoder.GetInt( &iDBIndex );
	

#ifdef _LOGCHANNEL_
	ServerUtil.ConsoleOutput( " OnRequestRoomPWD_Result(%d) : result(%d), DBIndex(%d)\n",
					lpSockContext->index, cResult, iDBIndex );
#endif
	
	iIndex = ServerContext.ch[ lpSockContext->iChannel ].GetDBIndexToSockIndex( iDBIndex );
	if( iIndex == NOTLINKED ) return 0;

	iRoomNum = lpSockContext->iRoom;
	if( ( ServerContext.rm[ iRoomNum ].GetCurUserNum() <= 0 ) ||
												lpSockContext->iRoom == NOTLINKED ) 
	{
		cResult = ROOMPASSWORD_FAIL_NOROOM;
	}
	else if( ServerContext.rm[ iRoomNum ].GetCurUserNum() >= ServerContext.iMaxUserInRoom ) 
	{
		cResult = ROOMPASSWORD_FAIL_FULLUSER;
	}
	
	
	packetcoder.SetBuf( cPacket );

	cRoomNum = static_cast<char>( iRoomNum - ( cChannelNum * ServerContext.iMaxRoomInChannel ) );
	if( cResult == ROOMPASSWORD_SUCCESS )
	{
		// �³� �뺸
		packetcoder.PutChar( cResult );
		packetcoder.PutChar( cRoomNum );

		cPWDLen = ServerContext.rm[ iRoomNum ].m_cPWDLen;
		CopyMemory( cPWD, ServerContext.rm[ iRoomNum ].m_cPWD, cPWDLen );

		packetcoder.PutChar( cPWDLen );
		packetcoder.PutText( cPWD, cPWDLen );
	}
	else
	{
		// ���� �뺸
		packetcoder.PutChar( cResult );
	}
	
	lpSC = &ServerContext.sc[ iIndex ];
	iSize = packetcoder.SetHeader( ANSWER_ROOMPASSWORD_RESULTA );
	PostTcpSend( 1, (int*)&lpSC, cPacket, iSize );
	
#ifdef _LOGCHANNEL_
	ServerUtil.ConsoleOutput( " OnRequestRoomPWD_Result End - result(%d)", cResult );
#endif
	return 1;
}


//! 1:1 ��ȭ ��û 
/*
	��û�� - 1:1 ��ȭ�� ��û�� ���.
	���� - ��û�ڰ� 1:1 ��ȭ �ϱ⸦ ���ϴ� ���

	cToIdLen:������ ���̵����, cToId: ������ ���̵�, iToIndex:������ ���� �ε��� 
	cCommentLen:�ڸ�Ʈ�� ����, cComment:�ڸ�Ʈ
*/
int OnRequest1ON1Chat_Demand( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder	packetcoder;
	LPSOCKETCONTEXT	lpSC;
	int				iSize, iToIndex, iToDBIndex;
	char			cToIdLen, cToId[16] = { 0, }, cChannelNum, cCommentLen, cComment[64] = { 0, }; 
	char			cPacket[128] = { 0, };
	
	iSize = cToIdLen =cCommentLen = 0;
	iToIndex = cChannelNum = NOTLINKED;

	packetcoder.SetBuf( cpPacket );
	packetcoder.GetInt( &iToDBIndex );
	packetcoder.GetChar( &cCommentLen );	if( cCommentLen <= 0 ) return 0;    
	packetcoder.GetText( cComment, cCommentLen );


	packetcoder.SetBuf( cPacket );

	
	// ������ ���� �ε����� ��� ����, �� ���� �����ε������ ������ ������ ��û�ڿ��� �˷��ش�.
	iToIndex = ServerContext.ch[ lpSockContext->iChannel ].GetDBIndexToSockIndex( iToDBIndex );
	if( iToIndex == NOTLINKED )
	{
		packetcoder.PutChar( ONECHAT_FAIL_NOTUSER );
		packetcoder.PutChar( cToIdLen );
		packetcoder.PutText( cToId, cToIdLen );
		iSize = packetcoder.SetHeader( ANSWER_1ON1CHAT_RESULTA );
		PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );
		return 0;
	}
	

	// ������ 1:1 ��ȭ�� �ź��ϰ� ������, ��û�ڿ��� �� ����� �˷��ش�.
	lpSC = &ServerContext.sc[ iToIndex ];
	if( lpSC == NULL ) return 0;
	if( lpSC->bALLChat == FALSE )
	{
		packetcoder.PutChar( ONECHAT_FAIL_ALLREJECT );
		packetcoder.PutChar( cToIdLen );
		packetcoder.PutText( cToId, cToIdLen );
		iSize = packetcoder.SetHeader( ANSWER_1ON1CHAT_RESULTA );
		PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );
		return 0;
	}
	
	// ���濡�� ��û�ڰ� 1:1 ��ȭ�� ��û���� �˷��ش�.
	cChannelNum = lpSockContext->iChannel;
	packetcoder.PutInt( lpSockContext->iUserDBIndex );
	packetcoder.PutChar( cChannelNum );
	packetcoder.PutChar( cCommentLen );
	packetcoder.PutText( cComment, cCommentLen );
	iSize = packetcoder.SetHeader( ANSWER_1ON1CHAT_DEMANDB );
	PostTcpSend( 1, (int*)&lpSC, cPacket, iSize );
	
	return 1;
}


//! ���濡�Լ� 1:1 ��ȭ ��û ����� �޾� ��û�ڿ��� �� ����� �뺸 ���ش�.
/*
	iFromIndex:��û���� ���� �ε���, iFromDBIndex:��û���� DB �ε���,
	cChannelNum:��û�ڰ� ���� ä�� ��ȣ
*/
int OnRequest1ON1Chat_Result( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder	packetcoder;
	LPSOCKETCONTEXT	lpSC;
	int				iSize, iFromIndex, iFromDBIndex;
	char			cResult, cChannelNum; 
	char			cPacket[16] = { 0, };

	iSize = 0;
	iFromIndex = iFromDBIndex = cChannelNum = NOTLINKED;

	packetcoder.SetBuf( cpPacket );
	packetcoder.GetChar( &cResult );
	packetcoder.GetInt( &iFromDBIndex );
	packetcoder.GetChar( &cChannelNum );

	if( cChannelNum < 0 || cChannelNum >= ServerContext.iMaxChannel ) return 0;

	packetcoder.SetBuf( cPacket );

	// ��û�ڰ� ���� �κ� ���ٸ� ����� �������� �ʴ´�.
	iFromIndex = ServerContext.ch[ cChannelNum ].GetDBIndexToSockIndex( iFromDBIndex );
	if( iFromIndex == NOTLINKED ) return 0;
	
	lpSC = &ServerContext.sc[ iFromIndex ];
	if( lpSC == NULL ) return 0;
	
	packetcoder.PutChar( cResult );
	packetcoder.PutInt( lpSockContext->iUserDBIndex );
	iSize = packetcoder.SetHeader( ANSWER_1ON1CHAT_RESULTA );
	PostTcpSend( 1, (int*)&lpSC, cPacket, iSize );
	
	return 1;
}


//! �������� 1:1 ��ȭ�� �Ѵ�.
int OnRequest1ON1Chat( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder	packetcoder;
	LPSOCKETCONTEXT	lpSC;
	int				iSize, iToIndex, iToDBIndex;
	char			cMsgLen, cMsg[128] = { 0, }; 
	char			cPacket[256] = { 0, };

	iSize = cMsgLen = 0;
	iToIndex = iToDBIndex = NOTLINKED;

	packetcoder.SetBuf( cpPacket );
	packetcoder.GetInt( &iToDBIndex );
	packetcoder.GetChar( &cMsgLen );	

	if( cMsgLen <= 0 || cMsgLen > MAXCHATPACKETLENGTH ) return 0;

	packetcoder.GetText( cMsg, cMsgLen );

	packetcoder.SetBuf( cPacket );

	// ������ �κ� ���ٸ� ��û�ڿ��� ���濡�� �޼����� ���� �� ������ �뺸�Ѵ�.
	iToIndex = ServerContext.ch[ lpSockContext->iChannel ].GetDBIndexToSockIndex( iToDBIndex );
	if( iToIndex == NOTLINKED ) 
	{
		packetcoder.PutInt( iToDBIndex );
		iSize = packetcoder.SetHeader( ANSWER_1ON1CHAT_CLOSE );
		PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );

#ifdef _LOGCHANNEL_
		ServerUtil.ConsoleOutput( " OnRequest1ON1Chat(%d) -Failed, iToDBIndex(%d), iToIndex(%d)\n",
			lpSockContext->index, iToDBIndex, iToIndex );
#endif
		return 0;
	}

	lpSC = &ServerContext.sc[ iToIndex ];
	if( lpSC == NULL ) return 0;

	// ��û���� �޼����� ���濡�� ������.
	packetcoder.PutInt( lpSockContext->iUserDBIndex );
	packetcoder.PutChar( cMsgLen );
	packetcoder.PutText( cMsg, cMsgLen );
	iSize = packetcoder.SetHeader( ANSWER_1ON1CHAT );
	PostTcpSend( 1, (int*)&lpSC, cPacket, iSize );

#ifdef _LOGCHANNEL_
	ServerUtil.ConsoleOutput( " OnRequest1ON1Chat(%d) -Success, iToDBIndex(%d), iToIndex(%d)\n",
		lpSockContext->index, iToDBIndex, iToIndex );
#endif

	return 1;
}


//! 1:1 ��ȭ �ߴ��� ��û�Ѵ�.
int OnRequest1ON1Chat_Close( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder	packetcoder;
	LPSOCKETCONTEXT	lpSC;
	int				iSize, iToDBIndex, iToIndex;
	char			cChannelNum;
	char			cPacket[16] = { 0, };

	iSize = 0;
	iToDBIndex = iToIndex = NOTLINKED;
	cChannelNum = lpSockContext->iChannel;
	
	packetcoder.SetBuf( cpPacket );
	packetcoder.GetInt( &iToDBIndex );

	// ������ ���� �ε����� ������, �κ� ���ٸ� ���濡�� ������ �ʴ´�.
	iToIndex = ServerContext.ch[ cChannelNum ].GetDBIndexToSockIndex( iToDBIndex );
	if( iToIndex == NOTLINKED ) return 0;

	lpSC = &ServerContext.sc[ iToIndex ];
	if( lpSC == NULL ) return 0;

	// 1:1 ��ȭ�� �ߴ� ���� �뺸�Ѵ�.
	packetcoder.SetBuf( cPacket );
	packetcoder.PutInt( lpSockContext->iUserDBIndex );
	iSize = packetcoder.SetHeader( ANSWER_1ON1CHAT_CLOSE );
	PostTcpSend( 1, (int*)&lpSC, cPacket, iSize );

	return 1;
}



// ������ ������ ������ �������� �ٽ� �ε��Ͽ� �������� �����ش�.
int OnRequestReLoadItem( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder	packetcoder;
	int				iSize;
	char			cPacket[16] = { 0, };
		
	ServerContext.db->GetDBUserItem( lpSockContext->cID, lpSockContext->index );

	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( static_cast<char>(lpSockContext->bCanPrivateItem) );
	packetcoder.PutChar( static_cast<char>(lpSockContext->bCanJumpItem) );
	iSize = packetcoder.SetHeader( ANSWER_RELOADUSERITEM );
	PostTcpSend( 1, (int*)&lpSockContext, cPacket, iSize );

	
	return 1;
}


