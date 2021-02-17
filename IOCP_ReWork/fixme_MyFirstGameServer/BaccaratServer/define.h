#ifndef	_DEFINE_H_
#define _DEFINE_H_

#define _WIN32_WINNT 0x0500
#include <windows.h>
#include <time.h>
#include <deque>

using namespace std;


#define _LOGFILELEVEL1_
#define _LOGFILELEVEL2_
#define _LOGFILELEVEL3_
#define _LOGFILELEVEL4_

//#define _LOGCONSOLE_	// ���� �÷��׸� ���� �ҷ��� �� �� �÷��װ� ���� �Ǿ� �ȴ�.
//#define _LOGLEVEL1_
//#define _LOGPROCESS_
//#define _LOGDATABASE_
//#define _LOGCHANNEL_
//#define _LOGROOM_
//#define _LOGGAME_

#define	SAFE_DELETE(p) { if(p) { delete p; (p) = NULL; } }
#define	SAFE_DELETE_ARRAY(p) { if(p) { delete[] p; (p) = NULL; } }


#define	MAXSENDPACKSIZE	512			// ������ ��Ŷ�� �ִ� ũ��
#define MAXRECVPACKSIZE	512			// �޴� ��Ŷ�� �ִ� ũ��
#define RINGBUFSIZE		16384		// �������� �ִ� ũ��
#define	MAXTRANSFUNC	256			// ��Ŷ ó�� �Լ��� �迭 ũ��
#define	HEADERSIZE		4			// ��Ŷ�� ��� ũ��


#define ROOMPERPACKET	10
#define USERPERPACKET	12


#define LIFETIMECOUNT	90			// ������ LIFETIMECOUNT�� ���� ���� ��Ŷ�� ������ ������ ������ ����� ����.
									// �ڿ� 30���� �ٲٱ�.
#define	DEFAULTPROCESS	0
#define DEFAULTCHANNEL	0
#define NOTLINKED		-1

#define NOTLOGINUSER    -1			// ������ ���� ��û�� �� ���������� ��Ÿ����.


#define	NOTINITIALIZED	0
#define	INITIALIZED		1

#define SOCKETCLOSE		0
#define SOCKETCONTINUE	1


#define	MAXSERVERID		20			// ���� ���̵��� �ִ� ����
#define	MAXGAMEID		20			// ���� ���� ���̵��� �ִ� ����
#define MAXDBSERVERIP	30			// ���� DB ������ IP �ּ� ����	
#define MAXDATABASE		30			// ���� DB ������ ����Ÿ ���̽� �̸�
#define	MAXIDLENGTH		13			// ID�� �ִ� ����
#define MAXPWDLENGTH	13			// �н������� �ִ� ����
#define MAXQUERYLENGTH	512			// SQL ������ �ִ� ����
#define MAXCHATPACKETLENGTH	110		// ä�ÿ��� �ִ� ��带 ������ ��Ŷ ũ��
#define MAXROOMTITLE	20			// �� ������ �ִ� ����
#define	MAXROOMPWD		15			// ���� �н������� �ִ� ����



#define USERWIN			1			// ������ �̰���
#define USERLOSE		2			// ������ �й���
#define USERDISCONNECT	3			// ������ ������ �� ���������� ����

#define	MINIMUMUSERLEVEL	0		// ���� ���� �� ���� �Ʒ�
#define	MAXIMUMUSERLEVEL	11		// ���� ���� �� ���� ��

#define SQLPASSWD			12
#define SQLNUM				4
#define	SQLGAME_ID			12
#define	SQLRESULT_WIN		4
#define	SQLRESULT_LOSE		4
#define	SQLDISCONNECT		4
#define	SQLSCORE			8
#define	SQLITEMCODE			12


#define MAXAVATARLAYER		16		// 16���̾�
#define LAYER_LENGTH		2		// �ƹ�Ÿ ���̾� �ϳ��� ũ��(2����Ʈ)


#define SELECTAREANUM		3		// ���� ���� ���� ��.
#define _BANKER				0
#define _PLAYER				1
#define _TIE				2

#define MAXBETTINGMULTYPLE	5		// �ְ� ���� �ܰ�
#define MINBETTINGMULTYPLE	1		// �ּ�(�⺻) ���� �ܰ�

#define	LEVEL_LOW			0		// ��� - �ϼ�
#define LEVEL_MIDDLE		1		// ��� - �߼�
#define LEVEL_HIGH			2		// ��� - ���

#define MAXMONEY			9999999999		// �÷��̾ ���� �� �ִ� �ְ� �ݾ�

#define PLAYERWIN_MULTIPLE	2		// �÷��̾����� �̱� ��쿡 �޴� �ݾ��� ���
#define BANKERWIN_MULTIPLE  2		// ��Ŀ���� �̱� ��쿡 �޴� �ݾ��� ���
#define TIEWIN_MULTIPLE		8		// Ÿ������ �̱� ��쿡 �޴� �ݾ��� ���
#define COMMISSION			0.05	// ��Ŀ�� �ɾ� �̱� ����� ���� ������.


#define MAXIMUMCARDNUM			52	// ī�� ���
#define	TOTALMAXCARDNUM			52	// �� ī�� ���� 
#define NOTUSECARD				-1	// ������� �ʴ� ī��
#define BACCARAT_HOLDCARDNUM	3	// �Ƕ� ���ӿ��� ������ ���� �� �ִ� �� ī�� ��.
#define	EIGHTCARD				8	
#define NINECARD				9 
#define JACKCARD				11  

// ��ī�󿡼� ����ϴ� ī�� �̹��� �迭�� ���� ���� ��
const char CardToNumber[ MAXIMUMCARDNUM ] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
											  1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
											  1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
											  1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0,
											};


#define MAXPLAYER				5		// ���� ���� �÷��̾��� �ε����� ������ �迭�� ũ���̴�.
#define	MINPLAYER				2

//define	GAMEREADYTIME			15		// ��⿡�� ���� ���� �� �������� �ð�(�� ����)
//#define	GAMEONTIME				15		// Ŭ���̾�Ʈ���� ���� ���� �غ� �� �� �ִ� �ð�. 
//#define	THINKTIME				10		// ���� �Ͽ��� �ִ� ���� �ð�.

enum
{
	RECVEOVTCP = 0,
	SENDEOVTCP,
};


// ������ ����� ��� ��
typedef struct
{
	__int64	LevelFrom;
	__int64 LevelTo;
}USERLEVELBOUNDARY;


// �����ۺ� �νĹ�ȣ 
typedef struct
{
	int	iPrivateRoom;
	int	iJump;
}ITEMINDEX;



// ��ī�󿡼������� ������ ���� ���� �ݾ�
typedef struct
{
	int	first;
	int	middle;
	int	high;
}BASEBETTINGMONEY;


// ���� ī���п����� ī�� ��ġ�� ī�� ��
typedef struct
{
	char Pos[8];
	char value[8];
}CARDPOS_VALUE;



// ���� ����
typedef struct
{
	char cType;
	char cTitleLen;
	char cTitle[ MAXROOMTITLE ];
	char cPWDLen;
	char cPWD[ MAXROOMPWD ];
}ROOMINFO;



// ������ ����
typedef struct
{
	char	nicLen;					// �г����� ũ��;
	char	NicName[MAXIDLENGTH];	// ���ӿ��� ����ϴ� �̸�.
	char	sex;					// ����

}USERINFO;


// ������ ���� ���� ����
typedef struct
{
	int			win;			//	�� �̱� Ƚ��
	int			lose;			//  �� ���� Ƚ��
	int 		disconnect;		//  ���� ���� Ƚ��
	__int64		money;			//	������ ( �ִ� ������ ũ�⿡ ���� �ڷ����� ���� �� �� ����)
	char		level;			//	���� 

}GAMESTATEMENT;


// ��ī�󿡼� Banker, Player, Tie ������ ���� ���� 
typedef struct
{
	char nCurCardNum;
	char Cards[ BACCARAT_HOLDCARDNUM ];
	char Score;
}SELECTAREAINFO;


typedef struct
{
	OVERLAPPED		ovl;
	int				mode;
}EOVERLAPPED, *LPEOVERLAPPED;


//������ ���� ���ؽ�Ʈ
typedef struct
{
	EOVERLAPPED			eovRecvTcp,
						eovSendTcp;

	char				cRecvTcpRingBuf[ RINGBUFSIZE + MAXRECVPACKSIZE ],	// �ޱ� ������
						*cpRTBegin,											// ������ ���� ��ġ
						*cpRTEnd,											// ������ �� ��ġ
						*cpRTMark,
						cSendTcpRingBuf[ RINGBUFSIZE + MAXSENDPACKSIZE ],	// ������ ������
						*cpSTBegin,							// ������ ���� ��ġ
						*cpSTEnd;							// ������ �� ��ġ
	int					iSTRestCnt;							// ������ �Ǵ� ũ��
	SOCKET				sockTcp;							// Ŭ���̾�Ʈ ����
	CRITICAL_SECTION	csSTcp;								// �Ӱ迵��
	sockaddr_in			remoteAddr;							// Ŭ���̾�Ʈ �ּ�
	
	time_t				RecvTime;							// Ŭ���̾�Ʈ�� ��Ŷ�� ���� �ð�
	
	int					index;								// ���� ���� �ε���
	
	char				idLen, cID[ MAXIDLENGTH ];		// ���� ID�� ����, ���� ID
													 
	char				pwdLen, cPWD[ MAXPWDLENGTH ];
				
	int					iUserDBIndex;						// DB������ ���� �ε���
	USERINFO			UserInfo;							// ������ �� ����
	GAMESTATEMENT		GameState;							// ������ ���� ����
	int					ibetMoney;							// ������ �ݾ�

	short				AvatarInfo[ MAXAVATARLAYER ];		// �ƹ�Ÿ ���̾��� ������ �����Ѵ�.
	char				AvatarType;							// �ƹ�Ÿ Ÿ��.

	BOOL				bCanPrivateItem,					// ����� ���� ���� �� �ִ� ������ ���� ���� 
						bCanJumpItem;						// ���� ���� ���� ������ ���� ����			
	
	BOOL				bALLChat;							// ��� ������ ��ȭ �ź�
	BOOL				bAllInvite;                         // ��� ������ �ʴ� �ź�
	char				cPosition;							// ������ ��ġ�� ��(������)
	char				cLogoutState;						// ������ ���� ��û ����.
	char				iProcess;							// ������ ��ġ�ϴ� ���μ��� ��ȣ
	char				iChannel;							// ������ ��ġ�ϴ� ä�� ��ȣ
	char				iOrderInRoom;						// �濡���� ������ ����
	int 				iRoom;								// ������ ��ġ�ϴ� �� ��ȣ

	BOOL				bAgency;							// �� ���� ���� ����.
	char				cState;								// ������ ���¸� ����.
	BOOL				bRequsetClose;						// Ŭ���̾�Ʈ���� ���Ḧ ��û.
	BOOL				bReservation;						// ������ ���� ����.
	char				nSelectArea;						// ������ ������ ����.
										
}SOCKETCONTEXT, *LPSOCKETCONTEXT;


typedef struct
{
	LPSOCKETCONTEXT			lpSockContext;
	char					*cpPacket;
}DATABASEDATA, *LPDATABASEDATA;


typedef struct
{
	int		prev, next;
}OBJECTNODE, *LPOBJECTNODE;


typedef struct
{
	int( *proc )( LPSOCKETCONTEXT lpSockContext, char* cpPacket );
}ONTRANSFUNC;

extern ONTRANSFUNC	OnTransFunc[MAXTRANSFUNC];



#endif