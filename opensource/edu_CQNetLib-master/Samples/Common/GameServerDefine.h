/*--------------------------------------------------------------------------------------------------*/
/*																									*/
/*		File			:	GameServerDefine.h														*/                     
/*		Date			:	2006�� 5�� 30��															*/
/*		Author			:	Kanggoon																*/
/*		Modifications	:																			*/
/*		Notes			:	���� ���� ���� ����														*/
/*							_RQ	:	Client -> Server�� ��û											*/
/*							_AQ	:	Server -> Client�� Rq�� ���� ����								*/
/*							_CN :   Client -> Server�� �˸�(�����̾���)								*/
/*							_SN :	Server -> Client�� �˸�(�����̾���)								*/
/*		Protocol		:	Clinet	<--->	LoginServer		:	1		~	20  					*/
/*							Client	<--->	GameServer		:	21		~	80						*/
/*							����							:	81		~	99						*/
/*																									*/
/*--------------------------------------------------------------------------------------------------*/
#pragma once
/*--------------------------------------------------------------------------------------*/
//define ����
/*--------------------------------------------------------------------------------------*/
//���ڿ� ����  + 1�� �ν�Ʈ���� ���ؼ�
#define MAX_USERID			12 + 1
#define MAX_USERPW			12 + 1
#define MAX_USERNAME		12 + 1
#define MAX_NICKNAME		16 + 1	//�г���
#define MAX_DATE			24 + 1  //��¥
#define MAX_ITEMMESSAGE		20 + 1	//������ ���� �޾����� �޽���
#define MAX_GAMEROOMNAME	24 + 1  //���̸�
#define MAX_LOBBYROOMNAME	24 + 1  //���̸�
#define MAX_ROOMPW			12 + 1  //�� ��й�ȣ
#define MAX_CHATMSG			100 + 1 //ä�� �޽���
#define MAX_INTRO			100 + 1 //�λ縻
#define MAX_EMAIL			100 + 1 //EMail
#define MAX_PRESENTMSG		30 + 1  //���� �޽���

//CHECK TIME
#define CHECKTIME_LOGIN		10
#define CHECKTIME_KEEPALIVE 60

//����
#define SERVER_TICK			500

//�ִ� ����	
#define MAX_CHARACTER_PARTS		15	//ĳ���Ͱ� ������ �� �ִ� �ִ� ������
#define MAX_CHARACTER_INSTRUMENTSLOT	6		//ĳ���� �Ǳ� ����
#define MAX_PROCESSFUNC		100		//��Ŷ ó���� ���� �Լ� ������
#define MAX_CHARACTERITEM	500		//ĳ���Ͱ� ������ �� �ִ� �ִ� ĳ���� ������ ��
#define MAX_ROOMUSER		8		//�ѹ濡 ������ ���� �ִ� �ִ��

//�ּ� ����
#define MIN_ROOMUSER		2

/*--------------------------------------------------------------------------------------*/
//enum ����
/*--------------------------------------------------------------------------------------*/
enum eErrorCode
{
	//������ ���� �ڵ�
	ERR_NONE					= 0x00000000,			//�Ϲ����� ��� ����
	ERR_NONE_LOGIN				= 0x00000001,			//�α� ���� ����		
	ERR_NONE_CREATENICKNAME		= 0x00000002,			//�г��� ���� ����
	ERR_NONE_CREATECHARACTER	= 0x00000003,			//ĳ���� ���� ����
	ERR_NONE_CONNECTGAMESERVER	= 0x00000004,			//���Ӽ��� ���� ���
	ERR_NONE_CREATEACCOUNT		= 0x00000005,			//�������� ����

	ERR_EXIST					= 0x00001000,			//������ �ִ��� üũ�ϴ� ����
	//�α� ���� ���� �ڵ�
	ERR_LOGIN_SAMEID			= 0x00001001,			//���� ���̵� �����մϴ�.
	ERR_LOGIN_SERVER			= 0x00001002,			//�α� ���� ����
	ERR_LOGIN_ID				= 0x00001003,			//ID�� �������� �ʽ��ϴ�.
	ERR_LOGIN_PW				= 0x00001004,			//PW�� Ʋ���ϴ�.
	ERR_LOGIN_SAMENICKNAME     	= 0x00001005,			//���� �г����� ���� �մϴ�.
	ERR_LOGIN_NOEXISTCHACRACTER	= 0x00001006,			//ĳ���Ͱ� ����. ó�� ���� ����� ��
	ERR_LOGIN_NOEXISTNICKNAME	= 0x00001007,			//�г����� ����
	

	

	//�κ�� ���� ���� �ڵ�
	ERR_LOBBYROOM_FULLROOM		= 0x00001100,			//�κ���� ���̻� �����Ҽ� ����.
	ERR_LOBBYROOM_NOTEXIST		= 0x00001101,			//�κ���� ã���� �����ϴ�.
	ERR_LOBBYROOM_FULLUSER		= 0x00001102,			//����ڰ� �������� �濡 ���� �����ϴ�.
	ERR_LOBBYROOM_NOTREADY		= 0x00001103,			//�غ���� ���� ����ڰ� �ֽ��ϴ�
	ERR_LOBBYROOM_ALREADYSTART  = 0x00001104,			//�̹� �÷������� ���̴�.
	ERR_SQL_FAILED				= 0x00009001,			//��� ����
	
	//������ ���� ���� �ڵ�
	ERR_ITEM_FULLRUBYBAG		= 0x00001200,			//��� �������� �������� ���̻� ��� ������ �� ����.
	ERR_ITEM_FULLSKILLSLOT		= 0x00001201,			//��ų ������ �������� ���̻� ��ų �������� ������ �� ����.
};

enum eGameOver
{
	GAMEOVER_ALLDEAD	= 0 ,
	GAMEOVER_TIMEOUT	= 1 ,
		
};

//BATTLE,CONCERT
enum eGameRoomMode
{
	GAMEROOMMODE_BATTLE,
	GAMEROOMMODE_CONCERT,
	
};

/*--------------------------------------------------------------------------------------*/
//struct ����
/*--------------------------------------------------------------------------------------*/
//��Ŷ ���
struct KPACKET_BASE
{
	unsigned short usLength;
	unsigned short usType;
};

struct KUDPPACKET_BASE
{
	unsigned short usLength;
	unsigned short usType;
	DWORD		   dwSequenceNo;		//UDP��Ŷ�� ���� ����
	DWORD		   dwPKey;				//��Ŷ�� ���� ������ PKey
	
	KUDPPACKET_BASE()
	{
		dwSequenceNo = 0;
		dwPKey = 0;
	}
};

//������ ������ �ִ� ĳ���� ������
struct CHARACTER_ITEM
{
	__int64		n64ItemKey;
	int			nItemCode;
	int			nRentalEndDate;
	
	CHARACTER_ITEM()
	{
		ZeroMemory( this , sizeof( CHARACTER_ITEM ) );
	}
};

