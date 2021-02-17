// Room.h: interface for the CRoom class.
//
//////////////////////////////////////////////////////////////////////

#ifndef	_ROOM_H_
#define _ROOM_H_


#include "ServerUtil.h"

const char strCheckUserLinkInRoom[] = "CheckUserLinkInRoom() - Not UserInRoom";


class CRoom  
{

private:
	
	int		m_iIndex;
	char	m_nLevel;
	char	m_iType;
	char	m_cTitleLen, m_cTitle[32], m_cOwnerLen, m_cOwner[16], m_cPWDLen, m_cPWD[32];

	int		m_iOwnerIndex; 
	
	int		m_iUserBegin, m_iUserEnd;
	char	m_iUserNum;

	char	bUsedOrder[MAXPLAYER];			// ���� �濡 �ִ� ����� ����� ���� ��ȣ�� ����.

public:
	char	m_iChannel;						// �� ���� ���� ä�� ��ȣ.
	char	m_RoomNum;						// ä�� �ӿ����� �� ��ȣ.
	char	m_iState;
	time_t	m_tRoomTime;					// �濡 ��ϵ� �ð�.
	int		m_Basebettingmoney[5];				// ���� �ݾ�.

	SELECTAREAINFO BankerInfo;					// ��Ŀ�� ������ ���� ��, �����ε���, ī��, ����
	SELECTAREAINFO PlayerInfo;					// �÷��̾ ������ ���� ��, �����ε���, ī��, ����
	SELECTAREAINFO TieInfo;						// Ÿ�̸� ������ ���� ��, �����ε���, ī��, ����

	char	m_nStartCompleteUserNum;			//  ���� �غ� ���� �÷��̾��� ��.
	
	char	m_privatecard[ TOTALMAXCARDNUM ];	//  ���ӿ��� �ڷ� ���� �ִ� ī��(���� ī��).
	int		m_iIndexprivatecard;				//  ������ �� ���� ī�� �ε���
	BOOL	m_bNoExitprivatecard;				//  TRUE : ������ �� ���� ī�尡 ����.
	
public:
	
	//! ���� �������� ��ũ�� ����� �Ǿ� �ִ��� Ȯ�� �� ������ ������ �����Ѵ�.
	void CheckUserLinkInRoom()
	{
		LPSOCKETCONTEXT lpSC;
		int iNext = m_iUserBegin;

		while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
		{
			lpSC = &ServerContext.sc[ iNext ];
			iNext = ServerContext.pn[ iNext ].next;

			if( lpSC->iRoom != m_iIndex || lpSC->idLen <= 0 )
			{
#ifdef _LOGFILELEVEL3_
				ServerUtil.RoomLog( m_iChannel, m_RoomNum, strCheckUserLinkInRoom );
#endif
				KillUserLink( lpSC->index );
			}
		}
	}
	//! ���� �� ������ �ʱ��� �Ѵ�.
	void InitRoomOrder()
	{
		for( int i = 0; i < MAXPLAYER; ++i )
		{
			bUsedOrder[ i ] = NOTLINKED;
		}
	}

	//! ���ο� ������ �濡 ������ ���� ��ȣ�� �ο��Ѵ�.
	char GetOrderInRoom()
	{
		char i;

		for( i = 0; i < MAXPLAYER; ++i )
		{
			if( bUsedOrder[ i ] == NOTLINKED )
			{
				bUsedOrder[ i ] = 1;
				return i;
			}	
		}

		return i;
	}

	//! ���� ������ �ݳ��Ѵ�.
	void DeleteOrderInRoom( char i )
	{
		bUsedOrder[ i ] = NOTLINKED;
	}

	//! DBIndex�� �̿��Ͽ� Ŭ���̾�Ʈ ������ �ε����� ���´�.
	int GetDBIndexToSockIndex(int iDBIndex)
	{
		LPSOCKETCONTEXT lpSC;
		int				iNext;

		iNext = m_iUserBegin;
		while( iNext > NOTLINKED && iNext < ServerContext.iMaxUserNum )
		{
			lpSC = &ServerContext.sc[ iNext ];
			if( lpSC->iUserDBIndex == iDBIndex )
			{
				return lpSC->index;
			}
			iNext = ServerContext.pn[ iNext ].next;
		}

		return NOTLINKED;
	}


	int GetUserBegin()
	{
		return m_iUserBegin;
	}


	int GetRoomIndex()
	{
		return m_iIndex;
	}


	char GetRoomType()
	{
		return m_iType;
	}


	char GetCurUserNum()
	{
		return m_iUserNum;
	}


	int GetOwnerIndex()
	{
		return m_iOwnerIndex;
	}


	CRoom();
	virtual ~CRoom();

	void InitRoom( int idx, char nlevel );
	void KillUserLink( int idx );
	void SetUserLink( int idx );
	
	
	friend int OnRequestCreateRoom( LPSOCKETCONTEXT lpSockContext, char* cpPacket );
	friend void AnswerCreateRoom( LPSOCKETCONTEXT lpSockContext, CChannel* pChannel, CRoom* pRoom, ROOMINFO& pRoomInfo );
	friend void NotifyCreateRoom( LPSOCKETCONTEXT lpSockContext, CRoom* pRoom, char cRoomNum, 
										int iBeginUser, int iUsedRoomBegin );
	
	friend int OnRequestExitRoom( LPSOCKETCONTEXT lpSockContext, char* cpPacket );
	friend int ExitNotifyRoomDelete( CChannel* pChannel, CRoom* pRoom, char& cPacket );
	friend int ExitNotiftRoomSetInfo( LPSOCKETCONTEXT lpSockContext, CChannel* pChannel, CRoom* pRoom, char& cPacket );
	friend void ExitRoomNotifyUserSetInfo( LPSOCKETCONTEXT lpSockContext, char& cPacket, int iSize, 
											int iUserBegin, int iRoomBegin );
	friend void RemainUserInRoom( CRoom* pRoom, LPSOCKETCONTEXT lpSockContext );
	

	friend int OnRequestJoinRoom( LPSOCKETCONTEXT lpSockContext, char* cpPacket );
	friend BOOL AnswerJoinRoom( LPSOCKETCONTEXT lpSockContext, CRoom* pRoom, char cRoomNum, char* cPWD, char cPWDLen );
	friend void NotifyJoinRoomSetInfo( LPSOCKETCONTEXT lpSockContext, CRoom* pRoom, char cRoomNum, 
										int iUserBegin, int iUsedRoomBegin );
	
	
	friend int OnRequestChangeRoomTitle( LPSOCKETCONTEXT lpSockContext, char* cpPacket );
	friend int OnRequestChangeRoomType( LPSOCKETCONTEXT lpSockContext, char* cpPacket );

	
	friend int OnRequestChatInRoom( LPSOCKETCONTEXT lpSockContext, char* cpPacket );
	

	friend int OnRequestInvite( LPSOCKETCONTEXT lpSockContext, char* cpPacket );
	friend int OnRequestInvite_Result( LPSOCKETCONTEXT lpSockContext, char* cpPacket );


	friend int OnNotifyRoomList( LPSOCKETCONTEXT lpSockContext, char* cpPacket );
	friend int OnNotifyUserList( LPSOCKETCONTEXT lpSockContext, char* cpPacket );


	friend void CheckChangeStateRoom( CRoom* pRoom );

	// CRoom class�� ������� �����ϱ� ���� ����� �Լ�. ���Ǵ� �ٸ� ������ �ϰ� �ִ�.
	friend int OnRequestRoomPWD_Result( LPSOCKETCONTEXT lpSockContext, char* cpPacket );
	friend int LogOutProcess( LPSOCKETCONTEXT lpSockContext );

	
	void UserInRoom_Regulation(void);
};

int InitRoomLayer();

#endif // !defined(AFX_ROOM_H__3BF27723_7098_4778_A07E_96D3AAF17DB8__INCLUDED_)
