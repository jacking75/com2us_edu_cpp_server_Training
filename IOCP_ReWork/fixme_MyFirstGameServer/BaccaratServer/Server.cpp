// Server.cpp: implementation of the CServer class.
//
//////////////////////////////////////////////////////////////////////


#include <wchar.h>
#include <process.h>
#include "serverprocess.h"
#include "channel.h"
#include "room.h"
#include "database.h"
#include "sock.h"
#include "game.h"
#include "serverutil.h"
#include "Server.h"


SERVERCONTEXT	ServerContext;
ONTRANSFUNC		OnTransFunc[ MAXTRANSFUNC ];
//class CRoom;
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CServer::CServer()
{
	
}

CServer::~CServer()
{

}

int CServer::Initialize()
{
	if (!InitServerInfo()) {
		return 0;
	}

	// DB �ʱ�ȭ�� �ϸ鼭 ���� ���α׷��� �ʿ��� ��� ������ �����´�.
	if (!InitDataBaseLayer()) {
		return 0;
	}

	if (!InitSocketIO()) {
		return 0;
	}

	if (!InitRoomLayer()) {
		return 0;
	}

	if (!InitChannelLayer()) {
		return 0;
	}

	if (!InitProcessLayer()) {
		return 0;
	}

	ServerContext.db->StroageCurUserNumInServer();

#ifdef _LOGFILELEVEL1_
	ServerUtil.ServerStartLog();
#endif
	
	return 1;
}



int CServer::InitServerInfo()
{
	WCHAR			wcModuleName[ MAX_PATH ];	// ������ ������ �� �ִ� ini ������ ��ġ
	char			cModuleName[ MAX_PATH ];
	int				length = 0;
	
	// ini ������ �ִ� ��ġ�� �����Ѵ�.
	GetModuleFileNameW( GetModuleHandle( NULL ), wcModuleName, MAX_PATH );
	*( wcsrchr( wcModuleName, '\\') + 1 ) = 0;
	wcscat( wcModuleName, L"BaccaratServerConf.ini" );

	GetModuleFileName( GetModuleHandle( NULL ), cModuleName, MAX_PATH );
	*( strrchr( cModuleName, '\\') + 1 ) = 0;
	strcat( cModuleName, "BaccaratServerConf.ini" );

	
	// ����, ���� ���̵� ���� �� ������ NULL�� �ʱ�ȭ �Ѵ�.
	wmemset( ServerContext.wcServerID, 0, MAXSERVERID );
	wmemset( ServerContext.wcGameID, 0, MAXGAMEID );
	wmemset( ServerContext.wcDBServerIP, 0, MAXDBSERVERIP );
	wmemset( ServerContext.wcDataBase, 0, MAXDATABASE );

	// ���� ���̵� INI ���Ͽ��� �ҷ��´�.
	length = GetPrivateProfileStringW( L"SETTING", L"SERVERID", L"baccarat_001",
					               ServerContext.wcServerID, MAXSERVERID, wcModuleName );
	if( length == 0 ) return 0;

	// ���� ���� ���̵� INI ���Ͽ��� �ҷ��´�.
	length = GetPrivateProfileStringW( L"SETTING", L"GAMEID", L"baccarat",
									ServerContext.wcGameID, MAXGAMEID, wcModuleName );
	if( length == 0 ) return 0;

	// ���� DB ������ IP�� INI ���Ͽ��� �ҷ��´�.
	length = GetPrivateProfileStringW( L"SETTING", L"DB_GAMESERVER_IP", L"211.106.195.38",
									ServerContext.wcDBServerIP, MAXDBSERVERIP, wcModuleName );
	if( length == 0 ) return 0;

	// ���ӿ��� ����ϴ� ����Ÿ ���̽��� �̸��� INI ���Ͽ��� �ҷ��´�.
	length = GetPrivateProfileStringW( L"SETTING", L"DB_DATABASE_NAME", L"haogame",
							ServerContext.wcDataBase, MAXDATABASE, wcModuleName );
	if( length == 0 ) return 0;

	ServerContext.bThreadStop = FALSE;


	// ��ī�� ���ӿ��� ����ϴ� ������ ���� ���� �ݾ� �����Ѵ�.
	ServerContext.BaseBettingMoney.first = GetPrivateProfileInt( "BACCARAT-BASEBETTINGMONEY", "FIRST", 0, cModuleName );
	ServerContext.BaseBettingMoney.middle = GetPrivateProfileInt( "BACCARAT-BASEBETTINGMONEY", "MIDDLE", 0, cModuleName );
	ServerContext.BaseBettingMoney.high = GetPrivateProfileInt( "BACCARAT-BASEBETTINGMONEY", "HIGH", 0, cModuleName );

	// ���ӿ��� ����ϴ� �Ѱ� �ð��� �����Ѵ�.
	ServerContext.iCheckLifeTime = GetPrivateProfileInt( "GAMETIME", "CHECKLIFTTIME", 90, cModuleName );
	ServerContext.iGameReadyTime = GetPrivateProfileInt( "GAMETIME", "GAMEREADYTIME", 10, cModuleName );
	ServerContext.iGameOnTime    = GetPrivateProfileInt( "GAMETIME", "GAMEONTIME", 10, cModuleName );
	ServerContext.iThinkTime     = GetPrivateProfileInt( "GAMETIME", "THINKTIME", 10, cModuleName );

	InitializeCriticalSection( &ServerContext.csKickUserIndexList );

	// ���ӿ��� ����ϴ� ��й� ���� �����۰� ���� �������� �ε��� ��ȣ�� ���´�.
	//ServerContext.ItemIndex.iPrivateRoom = GetPrivateProfileInt( "ITEM", "PRIVATEROOM", 3, 
	//																	cModuleName );
	//ServerContext.ItemIndex.iJump = GetPrivateProfileInt( "ITEM", "LEVELJUMP", 4, cModuleName );


	/* ���� �� ��� 
	// ini ������ �� ������ ������� ������ ���� �����´�.
	ServerContext.iPortNum = GetPrivateProfileInt( "SETTING", "PORT", 10004, cModuleName );
	ServerContext.iMaxUserNum = GetPrivateProfileInt( "SETTING", "MAXUSER", 32, cModuleName );
	ServerContext.iMaxProcess = GetPrivateProfileInt( "SETTING", "MAXPROCESS", 1, cModuleName );
	ServerContext.iMaxChannelInProcess = GetPrivateProfileInt( "SETTING", "MAXCHANNELINPROCESS",
																16, cModuleName );
	ServerContext.iMaxRoomInChannel = GetPrivateProfileInt( "SETTING", "MAXROOMINCHANNEL", 
																16, cModuleName );
	ServerContext.iMaxUserInRoom = GetPrivateProfileInt( "SETTING", "MAXUSERINROOM", 5, cModuleName );
	ServerContext.iInWorkerTNum = GetPrivateProfileInt( "SETTING", "WORKERTHREAD", 0, cModuleName );
	ServerContext.iInDataBaseTNum = GetPrivateProfileInt( "SETTING", "DATABASETHREAD", 0, cModuleName );

	ServerContext.iMaxChannel = ServerContext.iMaxProcess * ServerContext.iMaxChannelInProcess;
	ServerContext.iMaxRoom = ServerContext.iMaxChannel * ServerContext.iMaxRoomInChannel;
	ServerContext.iMaxUserInChannel = ServerContext.iMaxUserInRoom * ServerContext.iMaxRoomInChannel;
	ServerContext.iCurUserNum = 0;

	// ini ���Ͽ� ��ũ �������� ũ�Ⱑ ���� �Ǿ� ���� ���� ��� ��ũ�������� ũ�⸦
	// ������ �ӽ��� CPU ������ ����� �������� ���ڸ� �����Ѵ�.
	if( ServerContext.iInWorkerTNum == 0 )
	{
		GetSystemInfo( &si );

		// �ִ� 16���� ���� ���ϵ��� �Ѵ�.
		ServerContext.iInWorkerTNum = min( si.dwNumberOfProcessors * 2, 16 );
	}
	*/

	return 1;
}



void CServer::FinalCleanup()
{
	// ������ �۵��� ���� ��Ų��.
	ServerContext.bThreadStop = TRUE;
	
	CloseServerSocket();
	DeleteCS();

		
	SAFE_DELETE_ARRAY( ServerContext.rm )
	SAFE_DELETE_ARRAY( ServerContext.rn )
	SAFE_DELETE_ARRAY( ServerContext.ch )
	SAFE_DELETE_ARRAY( ServerContext.gameproc )

		
	SAFE_DELETE( ServerContext.db )
	SAFE_DELETE_ARRAY( ServerContext.sc )
	SAFE_DELETE_ARRAY( ServerContext.pn )
	SAFE_DELETE( ServerContext.ps )

#ifdef _LOGFILELEVEL1_
	ServerUtil.ServerEndLog();
#endif
	
}


void CServer::CloseServerSocket()
{
	int iN, iMax;
	
	iMax = ServerContext.iMaxUserNum;

	closesocket( ServerContext.sockListener );

	for( iN=0; iN < iMax; ++iN )
	{
		closesocket( ServerContext.sc[ iN ].sockTcp );
	}

	WSACleanup();
}

void CServer::DeleteCS()
{
	int iN, iMax;

	iMax = ServerContext.iMaxUserNum;
	for( iN = 0; iN < iMax; ++iN )
	{
		DeleteCriticalSection( &ServerContext.sc[ iN ].csSTcp );
	}

	DeleteCriticalSection( &ServerContext.csKickUserIndexList );
}
