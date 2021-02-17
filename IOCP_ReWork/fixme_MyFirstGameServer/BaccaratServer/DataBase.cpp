#define UNICODE
#define _UNICODE
#define DBINITCONSTANTS
#define INITGUID

#include <process.h>
#include <comdef.h>
#include <sqloledb.h>
#include <wchar.h>

#include "define.h"
#include "protocol_baccarat.h"
#include "packetcoder.h"
#include "server.h"
#include "sock.h"
#include "serverprocess.h"
#include "serverutil.h"
#include "DataBase.h"


CDataBase::CDataBase()
{
}

CDataBase::~CDataBase()
{
	int i;
	//CloseHandle( m_hSemaphore );
	//DeleteCriticalSection( &m_cs );

	m_pICommandText->Release();
	m_pIDBCreateCommand->Release();
	m_pIDBCreateSession->Release();

	for( i = 0; i < 4; ++i )
	{
		SysFreeString( m_InitProperties[i].vValue.bstrVal );
	}
	
	m_pIDBInitialize->Uninitialize();
	m_pIDBInitialize->Release();

	CoUninitialize();
}


int InitDataBaseLayer()
{
	ServerContext.db = new CDataBase;

	if( ServerContext.db == NULL ) return 0;

	ServerContext.db->InitDataBase();
		
	OnTransFunc[ REQUEST_LOGIN ].proc = OnRequestLoginDB;
	OnTransFunc[ REQUEST_LOGOUT_DB ].proc = OnRequestLogoutDB;

	return 1;
}


void CDataBase::InitDataBase()
{
	//HANDLE		hDatabaseT;
	//int			iN, dummy;

	m_iBegin = m_iEnd = m_iRestCount = 0;

	// OLEDB ���� �ʱ�ȭ
	m_pIDBInitialize		= NULL;
	m_pIDBProperties		= NULL;
	m_pIDBCreateSession		= NULL;
	m_pIDBCreateCommand		= NULL;
	m_pICommandText			= NULL;
	m_pIRowset				= NULL;
	m_pIColumnsInfo			= NULL;
	m_pDBColumnInfo			= NULL;
	m_pIAccessor			= NULL;

	m_pRows					= &m_hRows[0];

	InitializeAndEstablishConnection();
	
	// OLE DB�� Session�� Command�� �����Ѵ�.
	if( InitSessionAndCommand() == FALSE )
	{
		ServerUtil.DBErrorLog( InitSessionAndCommand_Err, 0 );
		return;
	}

	GetGameServerInfo();			// ������ ���õ� ������ DB���� �����´�.
	GetUserLevelBoundary();			// �� ������ �Ѱ� ���� DB���� �����´�.
	

	//InitializeCriticalSection( &m_cs );

	//m_hSemaphore = CreateSemaphore( NULL, 0, DBRINGBUFSIZE, NULL );

	/*for( iN = 0; iN < ServerContext.iInDataBaseTNum; ++iN )
	{
		hDatabaseT = (HANDLE)_beginthreadex( NULL, 0, DataBaseTProc, this, 0, 
																(unsigned int*)&dummy );
		CloseHandle( hDatabaseT );
	}*/
}


void CDataBase::InitializeAndEstablishConnection()
{
	HRESULT		hr;
	ULONG		i;
	WCHAR	szIP[20];
	WCHAR	szDB[20];
	
	wmemset( szIP, 0, 20 );
	wmemset( szDB, 0, 20 );

	wcscat( szIP, ServerContext.wcDBServerIP );
	wcscat( szDB, ServerContext.wcDataBase );
	
	CoInitialize( NULL );

	hr = CoCreateInstance( CLSID_SQLOLEDB, NULL, CLSCTX_INPROC_SERVER, IID_IDBInitialize, 
							(void**)&m_pIDBInitialize );
		
	if( FAILED(hr) ) 
		ServerUtil.DBErrorLog( IDBInitialize_Err, hr );
		
	for( i = 0; i < 4; ++i )
		VariantInit( &m_InitProperties[i].vValue );

	
	// ���� �̸�(�ڿ� �ణ �����ؾߵ�..SQL ������ ��ġ�� �ٲ� �� ����
	m_InitProperties[0].dwPropertyID		= DBPROP_INIT_DATASOURCE;
	m_InitProperties[0].vValue.vt			= VT_BSTR;
	m_InitProperties[0].vValue.bstrVal		= SysAllocString( szIP );
	m_InitProperties[0].dwOptions			= DBPROPOPTIONS_REQUIRED;
	m_InitProperties[0].colid				= DB_NULLID;

	// ����Ÿ ���̽�
	m_InitProperties[1].dwPropertyID		= DBPROP_INIT_CATALOG;
	m_InitProperties[1].vValue.vt			= VT_BSTR;
	m_InitProperties[1].vValue.bstrVal		= SysAllocString( szDB );
	m_InitProperties[1].dwOptions			= DBPROPOPTIONS_REQUIRED;
	m_InitProperties[1].colid				= DB_NULLID;

	//�����̸�
	m_InitProperties[2].dwPropertyID		= DBPROP_AUTH_USERID;
	m_InitProperties[2].vValue.vt			= VT_BSTR;
	m_InitProperties[2].vValue.bstrVal		= SysAllocString( L"gamedb_1_user" );
	m_InitProperties[2].dwOptions			= DBPROPOPTIONS_REQUIRED;
	m_InitProperties[2].colid				= DB_NULLID;

	// PassWord
	m_InitProperties[3].dwPropertyID		= DBPROP_AUTH_PASSWORD;
	m_InitProperties[3].vValue.vt			= VT_BSTR;
	m_InitProperties[3].vValue.bstrVal		= SysAllocString( L"gamedb_1_pw" );
	m_InitProperties[3].dwOptions			= DBPROPOPTIONS_REQUIRED;
	m_InitProperties[3].colid				= DB_NULLID;
	
	m_rgInitPropSet[0].guidPropertySet		= DBPROPSET_DBINIT;
	m_rgInitPropSet[0].cProperties			= 4;
	m_rgInitPropSet[0].rgProperties			= m_InitProperties;

	hr = m_pIDBInitialize->QueryInterface( IID_IDBProperties, (void**)&m_pIDBProperties );
	if( FAILED(hr) ) 
		ServerUtil.DBErrorLog( IDBProperties_Err, hr );

	hr = m_pIDBProperties->SetProperties( 1, m_rgInitPropSet );
	if( FAILED(hr) ) 
		ServerUtil.DBErrorLog( Set_Initializetion_Err, hr );

	m_pIDBProperties->Release();

	hr = m_pIDBInitialize->Initialize();
	if( FAILED( hr ) ) 
		ServerUtil.DBErrorLog(  ConnectionErr, hr );
	
}

int CDataBase::InitSessionAndCommand()
{
	HRESULT hr;

	hr = m_pIDBInitialize->QueryInterface( IID_IDBCreateSession, (void**)&m_pIDBCreateSession );
	if( FAILED(hr) )
	{
		ServerUtil.DBErrorLog( IDBCreateSession_Err, hr );
		return 0;
	}

	hr = m_pIDBCreateSession->CreateSession( NULL, IID_IDBCreateCommand, 
		(IUnknown**)&m_pIDBCreateCommand );
	if( FAILED(hr) )
	{
		ServerUtil.DBErrorLog( CreateSession_Err, hr );
		return 0;
	}

	hr = m_pIDBCreateCommand->CreateCommand( NULL, IID_ICommandText, 
		(IUnknown**)&m_pICommandText );
	if( FAILED(hr) )
	{
		ServerUtil.DBErrorLog( ICommand_Err, hr );
		return 0;
	}

	return 1;
}

void CDataBase::SetDBBInding( ULONG lNumCols, ULONG& ConsumerBufColOffSel, DBBINDING *pBinding)
{
	ULONG i;

	for( i = 0; i < lNumCols; ++i )
	{
		pBinding[i].iOrdinal	= i + 1;
		pBinding[i].obValue		= ConsumerBufColOffSel;
		pBinding[i].pTypeInfo	= NULL;
		pBinding[i].pObject		= NULL;
		pBinding[i].pBindExt	= NULL;
		pBinding[i].dwPart		= DBPART_VALUE;
		pBinding[i].dwMemOwner	= DBMEMOWNER_CLIENTOWNED;
		pBinding[i].eParamIO	= DBPARAMIO_NOTPARAM;
		pBinding[i].cbMaxLen	= m_pDBColumnInfo[i].ulColumnSize;
		pBinding[i].dwFlags		= 0;
		pBinding[i].wType		= m_pDBColumnInfo[i].wType;
		pBinding[i].bPrecision	= m_pDBColumnInfo[i].bPrecision;
		pBinding[i].bScale		= m_pDBColumnInfo[i].bScale;
		ConsumerBufColOffSel	= ConsumerBufColOffSel + m_pDBColumnInfo[i].ulColumnSize;
	}
}


void CDataBase:: GetGameServerInfo()
{
	HRESULT hr;
	DBROWCOUNT	lNumRows = 0;	
	WCHAR	wCmdString[512] = { 0, };

	wsprintf( wCmdString, L"select VERSION, PORT, MAXUSERNUM, MAXPROCESS, MAXCHANNEL, \
						MAXROOMINCHANNEL, MAXUSERINROOM, WORKERTHREADNUM, DBTHREADNUM, \
						CHANNEL_HIGH, CHANNEL_MIDDLE, CHANNEL_LOW, LEVEL_MIDDLE, \
						LEVEL_HIGH from GameServer where SERVER_ID = '%s' ", 
						ServerContext.wcServerID );
	
    hr = m_pICommandText->SetCommandText( DBGUID_DBSQL, wCmdString );
	if( FAILED(hr) )
	{
		ServerUtil.DBErrorLog( strErrGetGameServerInfo1, hr );
		ReConnectionAndInitDB();
		return;
	}
		
	hr = m_pICommandText->Execute( NULL, IID_IRowset, NULL, &lNumRows, (IUnknown**)&m_pIRowset );
	if( FAILED(hr) ) 
	{
		ServerUtil.DBErrorLog( strErrGetGameServerInfo2, hr );
		ReConnectionAndInitDB();
		return;
	}

	GameServerInfoResultSet();
	
	m_pIRowset->Release();
}

void CDataBase::GameServerInfoResultSet()
{
	HRESULT		hr;
	DBORDINAL	lNumCols = 0;
	DBCOUNTITEM	lNumRowsRetrieved = 0;
	ULONG		ConsumerBufColOffset, j;
	int			iMaxRoomInChannel;

	lNumCols = ConsumerBufColOffset = lNumRowsRetrieved =0;

	hr = m_pIRowset->QueryInterface( IID_IColumnsInfo, (void**)&m_pIColumnsInfo );
	if( FAILED(hr) )
	{
		ServerUtil.DBErrorLog( strErrGameServerInfoResultSet1, hr );
		ReConnectionAndInitDB();
		return;
	}
	
	m_pIColumnsInfo->GetColumnInfo( &lNumCols, &m_pDBColumnInfo,&m_pStringsBuffer );
	m_pIColumnsInfo->Release();

	m_pBinding = new DBBINDING[ lNumCols ];
	ServerContext.db->SetDBBInding( lNumCols, ConsumerBufColOffset, m_pBinding);

	hr = m_pIRowset->QueryInterface( IID_IAccessor, (void**)&m_pIAccessor );
	if( FAILED(hr) ) 
	{
		ServerUtil.DBErrorLog( strErrGameServerInfoResultSet2, hr );
		ReConnectionAndInitDB();
		return;
	}

	m_pIAccessor->CreateAccessor( DBACCESSOR_ROWDATA, lNumCols, m_pBinding, 0, 
												&m_hAccessor, NULL );

	m_pIRowset->GetNextRows( NULL, 0, 3, &lNumRowsRetrieved, &m_pRows );

	// ���� ó�� - �ڸ� ������ ����� ���� �ʾ� �ӽ������� ó������
	m_pBuffer = new BYTE[ ConsumerBufColOffset ];
	if( lNumRowsRetrieved > 0 )
	{
		memset( m_pBuffer, 0, ConsumerBufColOffset );
		m_pIRowset->GetData( m_hRows[0], m_hAccessor, m_pBuffer );
			
		ServerContext.iVersion			= m_pBuffer[ m_pBinding[0].obValue ];
		
		j = m_pBinding[1].obValue;
		CopyMemory( &ServerContext.iPortNum, &m_pBuffer[ j ], 4);
		
		j = m_pBinding[2].obValue;
		CopyMemory( &ServerContext.iMaxUserNum, &m_pBuffer[ j ], 4);
			
		ServerContext.iMaxProcess		= m_pBuffer[ m_pBinding[3].obValue ];
		ServerContext.iMaxChannel		= m_pBuffer[ m_pBinding[4].obValue ];
		ServerContext.iMaxRoomInChannel = m_pBuffer[ m_pBinding[5].obValue ];
		ServerContext.iMaxUserInRoom	= m_pBuffer[ m_pBinding[6].obValue ];
		ServerContext.iInWorkerTNum		= m_pBuffer[ m_pBinding[7].obValue ];
		ServerContext.iInDataBaseTNum	= m_pBuffer[ m_pBinding[8].obValue ];
		ServerContext.cChannelHighNum	= m_pBuffer[ m_pBinding[9].obValue ];
		ServerContext.cCnannelMiddleNum = m_pBuffer[ m_pBinding[10].obValue ];
		ServerContext.cChannelLowNum	= m_pBuffer[ m_pBinding[11].obValue ];
		ServerContext.cLevel_Middle		= m_pBuffer[ m_pBinding[12].obValue ];
		ServerContext.cLevel_High		= m_pBuffer[ m_pBinding[13].obValue ];
	}
	
	iMaxRoomInChannel = ServerContext.iMaxRoomInChannel;
	ServerContext.iMaxChannelInProcess	= ServerContext.iMaxChannel;
	ServerContext.iMaxRoom = ServerContext.iMaxChannel * iMaxRoomInChannel;
	ServerContext.iMaxUserInChannel = ServerContext.iMaxUserInRoom * iMaxRoomInChannel;
	ServerContext.iCurUserNum = 0;

	delete[] m_pBuffer;			m_pBuffer = NULL;
	m_pIAccessor->ReleaseAccessor( m_hAccessor, NULL );
	m_pIAccessor->Release();
	delete[] m_pBinding;		m_pBinding = NULL;	
}


void CDataBase::GetUserLevelBoundary()
{
	HRESULT	hr;
	DBROWCOUNT	lNumRows = 0;
	WCHAR	wCmdString[256] = { 0, };
	
	wsprintf( wCmdString, L"select LEVEL_FROM, LEVEL_TO from GameLevel where GAME_ID = '%s' ORDER BY LEVEL",
						ServerContext.wcGameID );

	
	hr = m_pICommandText->SetCommandText( DBGUID_DBSQL, wCmdString );
	if( FAILED(hr) )
	{
		ServerUtil.DBErrorLog( strErrGetUserLevelBoundary1, hr );
		ReConnectionAndInitDB();
		return;
	}
	
	hr = m_pICommandText->Execute( NULL, IID_IRowset, NULL, &lNumRows,
							(IUnknown**)&m_pIRowset );
	if( FAILED(hr) )
	{
		ServerUtil.DBErrorLog( strErrGetUserLevelBoundary2, hr );
		ReConnectionAndInitDB();
		return;
	}

	UserLevelBoundaryResultSet();

	m_pIRowset->Release();
}


void CDataBase::UserLevelBoundaryResultSet()
{
	HRESULT		hr;
	DBORDINAL	lNumCols = 0;
	DBCOUNTITEM lNumRowsRetrieved = 0;
	ULONG		ConsumerBufColOffsel, i, j, nRow;
	DBBINDING	*pBinding;
	BYTE		*pBuffer;
	
	lNumCols = ConsumerBufColOffsel = lNumRowsRetrieved = 0;

	hr = m_pIRowset->QueryInterface( IID_IColumnsInfo, (void**)&m_pIColumnsInfo );
	if( FAILED(hr) )
	{
		ServerUtil.DBErrorLog( strErrUserLevelBoundaryResultSet1, hr );
		ReConnectionAndInitDB();
		return;
	}

	m_pIColumnsInfo->GetColumnInfo( &lNumCols, &m_pDBColumnInfo, &m_pStringsBuffer );
	m_pIColumnsInfo->Release();

	pBinding = new DBBINDING[ lNumCols ];
	SetDBBInding( lNumCols, ConsumerBufColOffsel, pBinding );
	

	hr = m_pIRowset->QueryInterface( IID_IAccessor, (void**)&m_pIAccessor );
	if( FAILED(hr) )
	{
		ServerUtil.DBErrorLog( strErrUserLevelBoundaryResultSet2, hr );
		ReConnectionAndInitDB();
		return;
	}

	m_pIAccessor->CreateAccessor( DBACCESSOR_ROWDATA, lNumCols, pBinding, 0, &m_hAccessor, NULL );
	m_pIRowset->GetNextRows( NULL, 0, 5, &lNumRowsRetrieved, &m_pRows );
	pBuffer = new BYTE[ ConsumerBufColOffsel ];

	nRow = 0;
	while( lNumRowsRetrieved > 0 )
	{
		for( i = 0; i< lNumRowsRetrieved; ++i )
		{
			memset( pBuffer, 0, ConsumerBufColOffsel );
			m_pIRowset->GetData( m_hRows[i], m_hAccessor, pBuffer );

			j = pBinding[0].obValue;
			CopyMemory( &ServerContext.ULBoundary[nRow].LevelFrom , &pBuffer[j], 6 );
			
			j = pBinding[1].obValue;
			CopyMemory( &ServerContext.ULBoundary[nRow].LevelTo, &pBuffer[j], 6 );

			++nRow;
		}

		m_pIRowset->ReleaseRows( lNumRowsRetrieved, m_hRows, NULL, NULL, NULL );
		m_pIRowset->GetNextRows( NULL, 0, MAXROW, &lNumRowsRetrieved, &m_pRows );
	}

	delete[] pBuffer;			pBuffer = NULL;
	m_pIAccessor->ReleaseAccessor( m_hAccessor, NULL );
	m_pIAccessor->Release();
	delete[] pBinding;			pBinding = NULL;
}	


/*void CDataBase::DataBaseBufEnqueue(LPSOCKETCONTEXT lpSockContext, char *cpPacket)
{
	EnterCriticalSection( &m_cs );

	m_dbDataRingBuf[ m_iEnd ].lpSockContext = lpSockContext;
	m_dbDataRingBuf[ m_iEnd ].cpPacket = cpPacket;
	m_iEnd++;
	if( m_iEnd >= DBRINGBUFSIZE ) m_iEnd = 0;

	++m_iRestCount;

#ifdef _LOGDATABASE_
	if( m_iRestCount >= DBRINGBUFSIZE ) ServerUtil.ConsoleOutput( "---dbDataringBuf Overf;ow ---\n" );
#endif

	ReleaseSemaphore( m_hSemaphore, 1, NULL );

	LeaveCriticalSection( &m_cs );
}


void CDataBase::DataBaseBufDequeue(LPSOCKETCONTEXT* lpSockContext, char** cpPacket)
{
	EnterCriticalSection( &m_cs );

	if( m_iRestCount <= 0 )
	{
		*lpSockContext = DBBUFEMPTY;
		*cpPacket = DBBUFEMPTY;

		LeaveCriticalSection( &m_cs );

		return;
	}

	*lpSockContext = m_dbDataRingBuf[ m_iBegin ].lpSockContext;
	*cpPacket = m_dbDataRingBuf[ m_iBegin ].cpPacket;
	m_iBegin++;
	if( m_iBegin >= DBRINGBUFSIZE ) m_iBegin = 0;
	--m_iRestCount;

	LeaveCriticalSection( &m_cs );
}


unsigned int __stdcall DataBaseTProc( void* pParam )
{
	CDataBase			*pDBManage = (CDataBase*)pParam;
	LPSOCKETCONTEXT		lpSockContext;
	char				*cpPacket;
	short				sType;

	while( TRUE )
	{
		WaitForSingleObject( pDBManage->m_hSemaphore, INFINITE );

		pDBManage->DataBaseBufDequeue( &lpSockContext, &cpPacket );
		
		if( lpSockContext == DBBUFEMPTY ) continue;

		CopyMemory( &sType, cpPacket + sizeof(short), sizeof(short) );
		
		if( sType < 0 || sType >= FINAL_PACKETDEFINE ) continue;
		
		OnTransFunc[ sType ].proc( lpSockContext, cpPacket );
	}

	return 1;
}
*/

int CDataBase::CheckPWD(int iIndex)
{
	HRESULT			hr;
	LPSOCKETCONTEXT	lpSC = &ServerContext.sc[ iIndex ];
	//WCHAR			szID[ MAXIDLENGTH ] = { 0, };
	WCHAR			szCmdString[ 128 ] = { 0, };
	char			strErr[256] = {0,};
	DBROWCOUNT			lNumRows = 0;
	int				ErrCode;
		
	//swprintf( szID, L"%S", lpSC->cID );
	//swprintf( szCmdString, L"select PASSWD, SEX, NICK_CODE from GameUser where ID = '%S'", lpSC->cID );
	swprintf( szCmdString, L"{call CheckPWD('%S')}", lpSC->cID );

	
	hr = m_pICommandText->SetCommandText( DBGUID_DBSQL, szCmdString );
	if( FAILED(hr) )
	{
		sprintf( strErr, "SetCommandText Failed - CheckPWD(%ld), ID(%s)", hr, lpSC->cID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return LOGFAIL_DBERROR;
	}


	hr = m_pICommandText->Execute( NULL, IID_IRowset, NULL, &lNumRows,
									(IUnknown**)&m_pIRowset );
	if( FAILED(hr) )
	{
		sprintf( strErr, "Execute Failed - CheckPWD(%ld), ID(%s)", hr, lpSC->cID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return LOGFAIL_DBERROR;
	}

	ErrCode = CheckPWDResultSet( lpSC->cID, iIndex );

	return ErrCode;
}


int CDataBase::CheckPWDResultSet( const char* szID, int iIndex)
{
	HRESULT		hr;
	LPSOCKETCONTEXT lpSC = &ServerContext.sc[ iIndex ];
	DBORDINAL		lNumCols = 0;
	DBCOUNTITEM		lNumRowsRetrieved = 0;
	ULONG		ConsumerBufColOffSel, Len;
	int			ErrCode;
	DBBINDING*	pBinding;
	BYTE*		pBuffer;
	char		strErr[256] = {0,};
	char		cPWD[ MAXPWDLENGTH ] = {0,};

	lNumCols = ConsumerBufColOffSel = lNumRowsRetrieved = Len = 0;
	
	hr = m_pIRowset->QueryInterface( IID_IColumnsInfo, (void**)&m_pIColumnsInfo );
	if( FAILED(hr) )
	{
		sprintf( strErr, "IID_IColumnsInfo Interface Failed - CheckPWDResultSet(%ld), ID(%s)", hr, lpSC->cID);
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return LOGFAIL_DBERROR;
	}

	m_pIColumnsInfo->GetColumnInfo( &lNumCols, &m_pDBColumnInfo, &m_pStringsBuffer );
	m_pIColumnsInfo->Release();

	pBinding = new DBBINDING[ lNumCols ];
	SetDBBInding( lNumCols, ConsumerBufColOffSel, pBinding );
	
	
	hr = m_pIRowset->QueryInterface( IID_IAccessor, (void**)&m_pIAccessor );
	if( FAILED(hr) )
	{
		sprintf( strErr, "IAccessor Interface Failed - CheckPWDResultSet(%d), ID(%s)", hr, lpSC->cID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		
		return LOGFAIL_DBERROR;
	}

	m_pIAccessor->CreateAccessor( DBACCESSOR_ROWDATA, lNumCols, pBinding, 
													0, &m_hAccessor, NULL );
	m_pIRowset->GetNextRows( NULL,0, 1, &lNumRowsRetrieved, &m_pRows );

	pBuffer = new BYTE[ ConsumerBufColOffSel ];
	if( lNumRowsRetrieved > 0 )
	{
		memset( pBuffer, 0, ConsumerBufColOffSel );
		m_pIRowset->GetData( m_hRows[0], m_hAccessor, pBuffer );
		
		CopyMemory( cPWD, &pBuffer[ pBinding[0].obValue ], SQLPASSWD );
		lpSC->UserInfo.sex = pBuffer[ pBinding[1].obValue ];
		CopyMemory( lpSC->UserInfo.NicName, &pBuffer[ pBinding[2].obValue ], SQLGAME_ID );
	}
	else
	{
		delete[] pBuffer;			pBuffer = NULL;
		m_pIAccessor->ReleaseAccessor( m_hAccessor, NULL );
		m_pIAccessor->Release();
		delete[] pBinding;		pBinding = NULL;	
		m_pIRowset->Release();
		
		return LOGFAIL_NOID;
	}

	delete[] pBuffer;			pBuffer = NULL;
	m_pIAccessor->ReleaseAccessor( m_hAccessor, NULL );
	m_pIAccessor->Release();
	delete[] pBinding;		pBinding = NULL;	
	m_pIRowset->Release();
	
	
	// �н������� ���� ���̸� ���Ѵ�.
	int pwdLen = strlen( cPWD );
	if( lpSC->pwdLen != pwdLen ) return LOGFAIL_PASSWORD;

	// �н����带 ���Ѵ�.
	if( strncmp( lpSC->cPWD, cPWD, pwdLen ) != 0 )
	{
		return LOGFAIL_PASSWORD;
	}
	else
	{
		ErrCode = GetDBUserInfo( szID, iIndex );
		if( ErrCode != LOGIN_SUCCESS )
			return ErrCode;
	}


	return LOGIN_SUCCESS;
}


int CDataBase::GetDBUserInfo( const char* szID, int iIndex )
{
	HRESULT hr;
	WCHAR szCmdString[ 128 ] = { 0, };
	char	strErr[256]={0,};
	DBROWCOUNT	lNumRows = 0;
	int		iResult;
		

	//swprintf( szCmdString, L"select NUM, RESULT_WIN, RESULT_LOSE, DISCONNECT, \
	//					      SCORE, LEVEL_NUM from GameResult where ID = '%S' AND GAME_ID = '%s'",
	//						szID, ServerContext.wcGameID );
	swprintf( szCmdString, L"{call GetDBUserInfo('%S', '%s')}", szID, ServerContext.wcGameID );
	
	hr = m_pICommandText->SetCommandText( DBGUID_DBSQL, szCmdString );
	if( FAILED(hr) )
	{
		sprintf( strErr, "SetCommandText Failed - GetDBUserInfo(%ld), ID(%s)", hr, szID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return LOGFAIL_DBERROR;
	}


	hr = m_pICommandText->Execute( NULL, IID_IRowset, NULL, &lNumRows,
									(IUnknown**)&m_pIRowset );
	if( FAILED(hr) )
	{
		sprintf( strErr, "Execute Failed - GetDBUserInfo(%ld), ID(%s)", hr, szID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return LOGFAIL_DBERROR;
	}

	iResult = UserInfoResultSet( iIndex );
	m_pIRowset->Release();

	if( iResult == LOGIN_SUCCESS ) 
	{
		ServerContext.db->GetDBUserAvatarInfo( szID, iIndex );
		ServerContext.db->GetDBUserItem( szID, iIndex );
	}
	
	return iResult;
}


int CDataBase::UserInfoResultSet( int iIndex )
{
	HRESULT		hr;
	LPSOCKETCONTEXT lpSC = &ServerContext.sc[ iIndex ];
	DBORDINAL		lNumCols = 0;
	DBCOUNTITEM		lNumRowsRetrieved = 0;
	ULONG		ConsumerBufColOffSel, Len;
	DBBINDING*	pBinding;
	BYTE*		pBuffer;
	char		strErr[256]={0,};

	lNumCols = ConsumerBufColOffSel = lNumRowsRetrieved = Len = 0;

	hr = m_pIRowset->QueryInterface( IID_IColumnsInfo, (void**)&m_pIColumnsInfo );
	if( FAILED(hr) )
	{
		sprintf( strErr, "IID_IColumnsInfo Interface Failed(%ld) - UserInfoResultSet, ID(%s)", hr, lpSC->cID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return LOGFAIL_DBERROR;
	}

	m_pIColumnsInfo->GetColumnInfo( &lNumCols, &m_pDBColumnInfo, &m_pStringsBuffer );
	m_pIColumnsInfo->Release();

	pBinding = new DBBINDING[ lNumCols ];
	SetDBBInding( lNumCols, ConsumerBufColOffSel, pBinding );
	

	hr = m_pIRowset->QueryInterface( IID_IAccessor, (void**)&m_pIAccessor );
	if( FAILED(hr) )
	{
		sprintf( strErr, "IAccessor Interface Failed - UserInfoResultSet(%ld), ID(%s)", hr, lpSC->cID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return LOGFAIL_DBERROR;
	}

	m_pIAccessor->CreateAccessor( DBACCESSOR_ROWDATA, lNumCols, pBinding, 
													0, &m_hAccessor, NULL );
	m_pIRowset->GetNextRows( NULL,0, 1, &lNumRowsRetrieved, &m_pRows );

	pBuffer = new BYTE[ ConsumerBufColOffSel ];
	if( lNumRowsRetrieved > 0 )
	{
		memset( pBuffer, 0, ConsumerBufColOffSel );
		m_pIRowset->GetData( m_hRows[0], m_hAccessor, pBuffer );
		
		CopyMemory( &lpSC->iUserDBIndex, &pBuffer[ pBinding[0].obValue ], SQLNUM );
		CopyMemory( &lpSC->GameState.win, &pBuffer[ pBinding[1].obValue ], SQLRESULT_WIN );
		CopyMemory( &lpSC->GameState.lose, &pBuffer[ pBinding[2].obValue ], SQLRESULT_LOSE );
		CopyMemory( &lpSC->GameState.disconnect, &pBuffer[ pBinding[3].obValue ], SQLDISCONNECT );
		CopyMemory( &lpSC->GameState.money, &pBuffer[ pBinding[4].obValue ], SQLSCORE );
		lpSC->GameState.level = pBuffer[ pBinding[5].obValue ];
	}
	else
	{
		return LOGFAIL_NOID;
	}

	delete[] pBuffer;			pBuffer = NULL;
	m_pIAccessor->ReleaseAccessor( m_hAccessor, NULL );
	m_pIAccessor->Release();
	delete[] pBinding;		pBinding = NULL;	

	if( lpSC->GameState.money <= 0 ) return LOGFAIL_NOMONEY;

	return LOGIN_SUCCESS;
}



// ������ �ƹ�Ÿ ������ �����´�.
int CDataBase::GetDBUserAvatarInfo(const char* szID, int UserIndex)
{
	HRESULT hr;
	WCHAR szCmdString[ 128 ] = { 0, };
	DBROWCOUNT	lNumRows = 0;
	int		iResult;
	char	strErr[256]={0,};


	swprintf( szCmdString, L"{call GetDBUserAvatarInfo('%S')}", szID );

	hr = m_pICommandText->SetCommandText( DBGUID_DBSQL, szCmdString );
	if( FAILED(hr) )
	{
		sprintf( strErr, "SetCommandText Failed - GetDBUserAvatarInfo(%ld), ID(%s)", hr, szID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return LOGFAIL_DBERROR;
	}


	hr = m_pICommandText->Execute( NULL, IID_IRowset, NULL, &lNumRows, (IUnknown**)&m_pIRowset );
	if( FAILED(hr) )
	{
		sprintf( strErr, "Execute Failed - GetDBUserAvatarInfo(%ld), ID(%s)", hr, szID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return LOGFAIL_DBERROR;
	}

	iResult = UserAvatarResult( UserIndex );

	m_pIRowset->Release();

	return 1;
}


// GetDBUserAvatarInfo�� ���� ��û�� ���� ���̺��� ����Ÿ�� �����´�.
int CDataBase::UserAvatarResult( const int UserIndex )
{
	HRESULT		hr;
	LPSOCKETCONTEXT lpSC = &ServerContext.sc[ UserIndex ];
	DBORDINAL		lNumCols = 0;
	DBCOUNTITEM		lNumRowsRetrieved = 0;
	ULONG		ConsumerBufColOffSel, Len;
	DBBINDING*	pBinding;
	BYTE*		pBuffer;
	int			result = 1;
	char		strErr[256] = {0,};


	lNumCols = ConsumerBufColOffSel = lNumRowsRetrieved = Len = 0;

	hr = m_pIRowset->QueryInterface( IID_IColumnsInfo, (void**)&m_pIColumnsInfo );
	if( FAILED(hr) )
	{
		sprintf( strErr, "IID_IColumnsInfo Interface Failed - UserAvatarResult(%ld), ID(%s)", hr, lpSC->cID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return LOGFAIL_DBERROR;
	}

	m_pIColumnsInfo->GetColumnInfo( &lNumCols, &m_pDBColumnInfo, &m_pStringsBuffer );
	m_pIColumnsInfo->Release();

	pBinding = new DBBINDING[ lNumCols ];
	SetDBBInding( lNumCols, ConsumerBufColOffSel, pBinding );


	hr = m_pIRowset->QueryInterface( IID_IAccessor, (void**)&m_pIAccessor );
	if( FAILED(hr) )
	{
		sprintf( strErr, "IAccessor Interface Failed - UserAvatarResult(%ld), ID(%s)", hr, lpSC->cID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return LOGFAIL_DBERROR;
	}

	m_pIAccessor->CreateAccessor( DBACCESSOR_ROWDATA, lNumCols, pBinding, 
		0, &m_hAccessor, NULL );
	m_pIRowset->GetNextRows( NULL,0, 1, &lNumRowsRetrieved, &m_pRows );

	pBuffer = new BYTE[ ConsumerBufColOffSel ];
	if( lNumRowsRetrieved > 0 )
	{
		memset( pBuffer, 0, ConsumerBufColOffSel );
		m_pIRowset->GetData( m_hRows[0], m_hAccessor, pBuffer );


		CopyMemory( &lpSC->AvatarInfo[0], &pBuffer[ pBinding[0].obValue ], LAYER_LENGTH );	 // 1
		CopyMemory( &lpSC->AvatarInfo[1], &pBuffer[ pBinding[1].obValue ], LAYER_LENGTH );   // 2
		CopyMemory( &lpSC->AvatarInfo[2], &pBuffer[ pBinding[2].obValue ], LAYER_LENGTH );   // 3
		CopyMemory( &lpSC->AvatarInfo[3], &pBuffer[ pBinding[3].obValue ], LAYER_LENGTH );   // 4
		CopyMemory( &lpSC->AvatarInfo[4], &pBuffer[ pBinding[4].obValue ], LAYER_LENGTH );   // 5
		CopyMemory( &lpSC->AvatarInfo[5], &pBuffer[ pBinding[5].obValue ], LAYER_LENGTH );   // 6
		CopyMemory( &lpSC->AvatarInfo[6], &pBuffer[ pBinding[6].obValue ], LAYER_LENGTH );   // 7
		CopyMemory( &lpSC->AvatarInfo[7], &pBuffer[ pBinding[7].obValue ], LAYER_LENGTH );   // 8
		CopyMemory( &lpSC->AvatarInfo[8], &pBuffer[ pBinding[8].obValue ], LAYER_LENGTH );   // 9
		CopyMemory( &lpSC->AvatarInfo[9], &pBuffer[ pBinding[9].obValue ], LAYER_LENGTH );   // 10
		CopyMemory( &lpSC->AvatarInfo[10], &pBuffer[ pBinding[10].obValue ], LAYER_LENGTH ); // 11
		CopyMemory( &lpSC->AvatarInfo[11], &pBuffer[ pBinding[11].obValue ], LAYER_LENGTH ); // 12
		CopyMemory( &lpSC->AvatarInfo[12], &pBuffer[ pBinding[12].obValue ], LAYER_LENGTH ); // 13
		CopyMemory( &lpSC->AvatarInfo[13], &pBuffer[ pBinding[13].obValue ], LAYER_LENGTH ); // 14
		CopyMemory( &lpSC->AvatarInfo[14], &pBuffer[ pBinding[14].obValue ], LAYER_LENGTH ); // 15
		CopyMemory( &lpSC->AvatarInfo[15], &pBuffer[ pBinding[15].obValue ], LAYER_LENGTH ); // 16
		lpSC->AvatarType = pBuffer[ pBinding[16].obValue ];			                         // TYPE
	}
	else
	{
		result = LOGFAIL_DBERROR;
	}

	delete[] pBuffer;			pBuffer = NULL;
	m_pIAccessor->ReleaseAccessor( m_hAccessor, NULL );
	m_pIAccessor->Release();
	delete[] pBinding;		pBinding = NULL;	

	return result;
}



// ������ ������ �� ���ӿ��� ���� ����ϴ� �� ������ ���� �����۰�
// ���� �������� �ִ��� DB���� Ȯ���Ѵ�.
// STATE - 0(���), 1(���)
// �� ���� ������ - 1��(3), 5��(4), 10��(5)
// ���� ������    - 1��(6), 5��(7), 10��(8)
void CDataBase::GetDBUserItem( const char* szID, int iIndex)
{
	HRESULT	hr;
	WCHAR	szCmdString[ 128 ] = { 0, };
	DBROWCOUNT	lNumRows = 0;
	char	strErr[256] = {0,};
		
	
	//swprintf( szCmdString, L"select ITEM_NUM from GameItem where STATE='1' AND ID = '%S' \
	//			AND (ITEM_NUM=3 OR ITEM_NUM=4 OR ITEM_NUM=5 OR ITEM_NUM=6 OR ITEM_NUM=7 OR ITEM_NUM=8)",
	//						szID );
	swprintf( szCmdString, L"{call GetDBUserItem('%S', '%s')}", szID, ServerContext.wcGameID );
	
	hr = m_pICommandText->SetCommandText( DBGUID_DBSQL, szCmdString );
	if( FAILED(hr) )
	{
		sprintf( strErr, "SetCommandText Failed - GetDBUserItem(%ld), ID(%s)", hr, szID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return;
	}


	hr = m_pICommandText->Execute( NULL, IID_IRowset, NULL, &lNumRows,
									(IUnknown**)&m_pIRowset );
	if( FAILED(hr) )
	{
		sprintf( strErr, "Execute Failed - GetDBUserItem(%ld), ID(%s)", hr, szID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return;
	}

	UserItemResultSet( iIndex );
	
	m_pIRowset->Release();
}


void CDataBase::UserItemResultSet(int iIndex)
{
	HRESULT			hr;
	LPSOCKETCONTEXT lpSC = &ServerContext.sc[ iIndex ];
	ULONG			ConsumerBufColOffSel, i, Len;
	DBORDINAL		lNumCols = 0;
	DBCOUNTITEM		lNumRowsRetrieved = 0;
	DBBINDING*		pBinding;
	BYTE*			pBuffer;
	int				iItemCode;
	BOOL			bExitJumpItem, bExitPriItem;
	char			strErr[256] = {0,};


	lNumCols = ConsumerBufColOffSel = lNumRowsRetrieved = Len = 0;
	bExitJumpItem = bExitPriItem = FALSE;

	hr = m_pIRowset->QueryInterface( IID_IColumnsInfo, (void**)&m_pIColumnsInfo );
	if( FAILED(hr) )
	{
		sprintf( strErr, "IID_IColumnsInfo Interface Failed - UserItemResultSet(%ld), ID(%s)", hr, lpSC->cID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return;
	}

	m_pIColumnsInfo->GetColumnInfo( &lNumCols, &m_pDBColumnInfo, &m_pStringsBuffer );
	m_pIColumnsInfo->Release();

	pBinding = new DBBINDING[ lNumCols ];
	SetDBBInding( lNumCols, ConsumerBufColOffSel, pBinding );
	

	hr = m_pIRowset->QueryInterface( IID_IAccessor, (void**)&m_pIAccessor );
	if( FAILED(hr) )
	{
		sprintf( strErr,  "IAccessor Interface Failed - UserItemResultSet(%ld), ID(%s)", hr, lpSC->cID);
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return;
	}

	m_pIAccessor->CreateAccessor( DBACCESSOR_ROWDATA, lNumCols, pBinding, 
													0, &m_hAccessor, NULL );
	m_pIRowset->GetNextRows( NULL,0, 10, &lNumRowsRetrieved, &m_pRows );

	pBuffer = new BYTE[ ConsumerBufColOffSel ];
	while( lNumRowsRetrieved > 0 )
	{
		for( i = 0; i < lNumRowsRetrieved; ++i )
		{
			if( bExitPriItem == TRUE && bExitJumpItem == TRUE ) break;

			iItemCode = 0;
			memset( pBuffer, 0, ConsumerBufColOffSel );
			m_pIRowset->GetData( m_hRows[i], m_hAccessor, pBuffer );
		
			CopyMemory( &iItemCode, &pBuffer[ pBinding[0].obValue ], 4 );
		
			if( iItemCode == 3 || iItemCode == 4 || iItemCode == 5 )
			{
				lpSC->bCanJumpItem = TRUE;
				bExitJumpItem = TRUE;
			}
			else if( iItemCode == 6 || iItemCode == 7 || iItemCode == 8 )
			{
				lpSC->bCanPrivateItem = TRUE;
				bExitPriItem = TRUE;
			}
		}
		
		if( bExitPriItem == TRUE && bExitJumpItem == TRUE ) break;
		
		m_pIRowset->ReleaseRows( lNumRowsRetrieved, m_hRows, NULL, NULL, NULL );
		m_pIRowset->GetNextRows( NULL, 0, 10, &lNumRowsRetrieved, &m_pRows );
	}
	
	delete[] pBuffer;			pBuffer = NULL;
	m_pIAccessor->ReleaseAccessor( m_hAccessor, NULL );
	m_pIAccessor->Release();
	delete[] pBinding;			pBinding = NULL;	
}


void CDataBase::NotifyLogFaild(char cResult, int index )
{
	CPacketCoder	packetcoder;
	LPSOCKETCONTEXT	lpSC = &ServerContext.sc[ index ];
	char			cPacket[16] = { 0, };
	int				iSize = 0;

	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( cResult );
	iSize = packetcoder.SetHeader( ANSWER_LOGIN );
	PostTcpSend( 1, (int*)&lpSC, cPacket, iSize );

#ifdef _LOGFILELEVEL1_
	ServerUtil.LogInFailed( inet_ntoa(lpSC->remoteAddr.sin_addr), cResult );
#endif

	EnqueueClose( lpSC );
}


// ���� �ƹ�Ÿ�� �����ۿ� ���� ����� ������ �ʰ� �ִ�.
void CDataBase::NotifyLogSuccess( int index )
{
	CPacketCoder	packetcoder;
	LPSOCKETCONTEXT	lpSC = &ServerContext.sc[ index ];
	char			cPacket[128] = { 0, };
	int				iHigh, iLow, iSize = 0;
	
	
	lpSC->UserInfo.nicLen = static_cast<char>( strlen( lpSC->UserInfo.NicName ) );

	packetcoder.SetBuf( cPacket );
	packetcoder.PutChar( LOGIN_SUCCESS );
	packetcoder.PutChar( lpSC->UserInfo.nicLen );
	packetcoder.PutText( lpSC->UserInfo.NicName, lpSC->UserInfo.nicLen );
	packetcoder.PutChar( lpSC->UserInfo.sex );
	packetcoder.PutInt( lpSC->iUserDBIndex );
	
	iHigh = (int)((lpSC->GameState.money >> 32) & (__int64)0x00000000FFFFFFFF);
	iLow = (int)((lpSC->GameState.money) & (__int64)0x00000000FFFFFFFF);
	
	packetcoder.PutChar( lpSC->GameState.level );
	packetcoder.PutInt( lpSC->GameState.win );
	packetcoder.PutInt( lpSC->GameState.lose );
	packetcoder.PutInt( lpSC->GameState.disconnect );
	packetcoder.PutInt( iHigh );
	packetcoder.PutInt( iLow );
	packetcoder.PutChar( static_cast<char>(lpSC->bCanPrivateItem) );
	packetcoder.PutChar( static_cast<char>(lpSC->bCanJumpItem) );
	packetcoder.PutText( (char*)lpSC->AvatarInfo, sizeof(short)*MAXAVATARLAYER );
	packetcoder.PutChar( lpSC->AvatarType );
	iSize = packetcoder.SetHeader( ANSWER_LOGIN );
	PostTcpSend( 1, (int*)&lpSC, cPacket, iSize );
}


int OnRequestLoginDB( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	CPacketCoder		packetcoder;
	char				pwdLen, cPWD[20]={0,};
	char				cResult;
	short				sVersion;
	
	packetcoder.SetBuf( cpPacket );
	packetcoder.GetChar( &lpSockContext->idLen );	
	if( lpSockContext->idLen <= 0 || lpSockContext->idLen >= MAXIDLENGTH ) return 0;
	
	packetcoder.GetText( lpSockContext->cID, lpSockContext->idLen );

	packetcoder.GetChar( &pwdLen);	
	if( pwdLen <= 0 || pwdLen >= MAXPWDLENGTH ) return 0;

	packetcoder.GetText( cPWD, pwdLen );	 
	packetcoder.GetShort( &sVersion );
	
	
	if( ServerContext.iVersion != sVersion )
	{
		ServerContext.db->NotifyLogFaild( LOGFAIL_NOVERSION, lpSockContext->index );
		return 0;
	}
	
	if( ServerContext.ps->UserFindId( lpSockContext->cID, lpSockContext->idLen ) == TRUE )
	{
		ServerContext.db->NotifyLogFaild( LOGFAIL_DUPLEID, lpSockContext->index );
		return 0;
	}

	// �н������� ��ȣ�� Ǭ��.
	//char*				SECRET_KEY = "�ڢ�";
	//ServerContext.db->ThreeiDecrypt( cPWD, SECRET_KEY, pwdLen );
			
	CopyMemory( lpSockContext->cPWD, cPWD, pwdLen );
	lpSockContext->pwdLen = pwdLen;
	

	cResult = static_cast<char>( ServerContext.db->CheckPWD( lpSockContext->index ) );
	if( cResult != LOGIN_SUCCESS )
	{
		ServerContext.db->NotifyLogFaild( cResult, lpSockContext->index );
		return 0;
	}
		
	
	// �α��� ���� �̹Ƿ� ������ ����� ��ũ�� ����Ʈ�� �߰� �Ѵ�.
	lpSockContext->cPosition = WH_CHANNELSELECT;
	
	ServerContext.ps->SetUserLink( lpSockContext->index );
	ServerContext.db->StroageCurUserNumInServer();

	time( &lpSockContext->RecvTime );
	ServerContext.db->NotifyLogSuccess( lpSockContext->index );
	
#ifdef _LOGFILELEVEL2_
	ServerUtil.LogInSuccess( lpSockContext->cID );
#endif

	return 1;
}


int OnRequestLogoutDB( LPSOCKETCONTEXT lpSockContext, char* cpPacket )
{
	if( lpSockContext->cPosition == WH_CHANNELSELECT )
	{
		ServerContext.ps->KillUserLink( lpSockContext->index );

		// ���� �ο��� DB�� �����Ѵ�.
		ServerContext.db->StroageCurUserNumInServer();

		// �α� ���Ͽ� ����
#ifdef _LOGFILELEVEL2_
		ServerUtil.LogOut( lpSockContext->cID );
#endif

	}

	lpSockContext->cPosition = NOTLINKED;
	ReInitSocketContext( lpSockContext );

	return 1;
}



// ���� ������ �����Ѵ�.
void CDataBase::SaveUserData( LPSOCKETCONTEXT lpSC, BOOL IsDisConnect )
{
	HRESULT hr;
	WCHAR	szCmdString[ MAXQUERYLENGTH ] = { 0, };
	DBROWCOUNT	lNumRows = 0;
	int		cLevel;
	char	strErr[256]={0,};


	// ���� �ݾ��� ��� �̸� �¸�, �����̸� �� �����ȴ�.
	if( lpSC->ibetMoney < 0 )
		lpSC->GameState.lose++;
	else
		lpSC->GameState.win++;
	
	// ���� ���� ���� �ݾ��� �����ϰ� ���� 0�� �����̸� �α� �ƿ� �غ� ���·� �ٲ۴�.
	lpSC->GameState.money += lpSC->ibetMoney;
	if( lpSC->GameState.money <= 0 ) lpSC->cState = GAME_NOTMONEY;
	
	// �÷��̾ ���� �� �ִ� �ְ� �ݾ� ���� ũ�� �ְ�ݾ����� �ٲ۴�.
	if( lpSC->GameState.money > MAXMONEY ) lpSC->GameState.money = MAXMONEY;

	UserLevelChangeCheck( lpSC );
	cLevel = lpSC->GameState.level;
	
		
	if(	IsDisConnect == TRUE )
	{
		--lpSC->GameState.disconnect;
		
		//swprintf( szCmdString, L"UPDATE GameResult SET RESULT_WIN = %d, RESULT_LOSE = %d, DISCONNECT = %d, \
		//				SCORE = %I64d, LEVEL_NUM = %d where ID = '%S'", 
		//				lpSC->GameState.win, lpSC->GameState.lose, lpSC->GameState.disconnect, 
		//				lpSC->GameState.money, lpSC->GameState.level, lpSC->cID );
		swprintf( szCmdString, L"{call SaveAbNormalUserData(%d,%d,%d,%I64d,%d,'%S','%s')}",
						lpSC->GameState.win, lpSC->GameState.lose, lpSC->GameState.disconnect,
						lpSC->GameState.money, lpSC->GameState.level, lpSC->cID, ServerContext.wcGameID );
	}
	else 
	{
		//swprintf( szCmdString, L"UPDATE GameResult SET RESULT_WIN = %d, RESULT_LOSE = %d, SCORE = %I64d, \
		//			              LEVEL_NUM = %d where ID = '%S'", lpSC->GameState.win, lpSC->GameState.lose,
		//												lpSC->GameState.money, lpSC->GameState.level, lpSC->cID );
		swprintf( szCmdString, L"{call SaveUserData(%d,%d,%I64d,%d,'%S','%s')}",
						lpSC->GameState.win, lpSC->GameState.lose, 
						lpSC->GameState.money, lpSC->GameState.level, lpSC->cID, ServerContext.wcGameID );
	}
#ifdef _LOGFILELEVEL4_
	ServerUtil.UserSaveLog( lpSC->GameState.win, lpSC->GameState.lose, lpSC->GameState.disconnect,
							lpSC->GameState.money, lpSC->GameState.level, lpSC->cID );
#endif

	hr = m_pICommandText->SetCommandText( DBGUID_DBSQL, szCmdString );
	if( FAILED(hr) )
	{
		sprintf( strErr, "SetCommandText Failed - SaveUserData(%ld), ID(%s)", hr, lpSC->cID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return;
	}


	hr = m_pICommandText->Execute( NULL, IID_IRowset, NULL, &lNumRows,
									(IUnknown**)&m_pIRowset );
	if( FAILED(hr) )
	{
		sprintf( strErr, "Execute Failed - SaveUserData(%ld), ID(%s)", hr, lpSC->cID );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return;
	}

	if( m_pIRowset )
		m_pIRowset->Release();
}



void CDataBase::StroageCurUserNumInServer()
{
	HRESULT hr;
	WCHAR	szCmdString[ 128 ] = { 0, };
	DBROWCOUNT	lNumRows = 0;
	char	strErr[128]={0,};
		

	//swprintf( szCmdString, L"UPDATE GameCounter SET CUR_COUNTER = %d where SERVER_ID = '%s'", 
	//							  ServerContext.iCurUserNum, ServerContext.wcServerID );
	swprintf( szCmdString, L"{call StroageCurUserNumInServer(%d, '%s')}", 
							           ServerContext.iCurUserNum, ServerContext.wcServerID );

	hr = m_pICommandText->SetCommandText( DBGUID_DBSQL, szCmdString );
	if( FAILED(hr) )
	{
		sprintf( strErr, "SetCommandText Failed - StroageCurUserNumInServer(%ld)", hr );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return;
	}
	

	hr = m_pICommandText->Execute( NULL, IID_IRowset, NULL, &lNumRows,
									(IUnknown**)&m_pIRowset );
	if( FAILED(hr) )
	{
		sprintf( strErr, "Execute Failed - StroageCurUserNumInServer(%ld)", hr );
		ServerUtil.DBErrorLog( strErr, hr );
		ReConnectionAndInitDB();
		return;
	}

	if( m_pIRowset )
		m_pIRowset->Release();
}



// ���̳� ���ھ üũ�Ͽ� �׿� ���� ������ �����Ѵ�. 
void CDataBase::UserLevelChangeCheck(LPSOCKETCONTEXT lpSockContext)
{
	char j = 0;
	char ilevel = lpSockContext->GameState.level;
	__int64  money = lpSockContext->GameState.money;
	

	// ���� ���� ���� ���� ������ ���Ѽ� �� ���� �۴ٸ�
	if( money < ServerContext.ULBoundary[ ilevel ].LevelFrom )
	{
		if( ilevel > 1 ) 
		{
			// �� �������� �ϳ� �Ʒ� �������� ������ ������.
			for( j = (ilevel - 1); j >= 0; --j )
			{	
				// ���� ���� ���� ���� �Ʒ� ������ ���Ѽ� ���� ũ�ٸ� 
				if( ServerContext.ULBoundary[ j ].LevelFrom <= money )
				{
					// ���ο� ������ �Է��Ѵ�.
					lpSockContext->GameState.level = j;
					break;
				}
			}
		}
		else // ���� ������ 1 ���϶�� ������ 0 �̴�.
		{
			lpSockContext->GameState.level = 0;
		}

	} // ���� ���� ���� ���� ������ ���Ѽ� ���� ������ ���ٸ�
	else if( money >= ServerContext.ULBoundary[ ilevel ].LevelTo )
	{
		// �� �������� �ϳ� ���� �������� ������ ������.
		for( j = (ilevel + 1); j < 12; ++j )
		{
			// ���� ���� ���� ���� �� ������ ���Ѽ� ���� �޴ٸ�
			if( money < ServerContext.ULBoundary[ j ].LevelTo )
			{
				// ���ο� ������ �Է��Ѵ�.
				lpSockContext->GameState.level = j;
				break;
			}
		}

		// �߰��� ������ ����� �����Ƿ� �ְ� ������ �����ȴ�.
		lpSockContext->GameState.level = MAXIMUMUSERLEVEL;

	}
}

void CDataBase::ReConnectionAndInitDB()
{
	int		i;

	m_pICommandText->Release();
	m_pIDBCreateCommand->Release();
	m_pIDBCreateSession->Release();

	for( i = 0; i < 4; ++i )
	{
		SysFreeString( m_InitProperties[i].vValue.bstrVal );
	}

	m_pIDBInitialize->Uninitialize();
	m_pIDBInitialize->Release();

	// �ʱ�ȭ �Ѵ�.
	InitializeAndEstablishConnection();

	// OLE DB�� Session�� Command�� �����Ѵ�.
	if( InitSessionAndCommand() == FALSE )
	{
		ServerUtil.DBErrorLog( InitSessionAndCommand_Err, 0 );
		return;
	}
}


int CDataBase::ThreeiDecrypt(char* buf, const char* key, int len)
{
	//2016.08.09 64��Ʈ���� ������ �ȵ�
	// ��ȣȭ�� 4����Ʈ �����θ� �����ȴ�.  ���� len%4�� ���� ������ �װ��� ���õȴ�.
	//len = len / 4;

	//_asm
	//{
	//	mov   ecx, len    // how many copies
	//	mov   eax, key    // copy key
	//	mov   ebx, [eax]  // copy key

	//	mov   esi, buf    // set source

	//repeat:
	//	mov   eax, [esi]	// eax = (long)*(buf)

	//	xor   eax, ebx		// eax = eax ^ ebx
	//	mov   edx, eax
	//	and   edx, 0xfffff000
	//	shr   edx, 12
	//	and   eax, 0x00000fff
	//	shl   eax, 20
	//	or    eax, edx
	//	xor   eax, ebx		// eax = eax ^ ebx

	//	mov   [esi], eax	// (long)*(buf) = eax

	//	add   esi, 4		// buf = buf+4  // ���� ��ȣȭ �ּҷ� �̵�
	//	dec   len			// len--

	//	jg    repeat
	//}				

	return 0;
}


int CDataBase::ThreeiEncrypt(char *buf, const char *key, int len)
{
	//2016.08.09 64��Ʈ���� ������ �ȵ�
	// ��ȣȭ�� 4����Ʈ �����θ� �����ȴ�.  ���� len%4�� ���� ������ �װ��� ���õȴ�.
	//len = len / 4;

	//_asm
	//{
	//	mov   ecx, len    // how many copies
	//	mov   eax, key    // copy key
	//	mov   ebx, [eax]  // copy key

	//	mov   esi, buf    // set source

	//repeat:
	//	mov   eax, [esi]	// eax = (long)*(buf)

	//	xor   eax, ebx		// eax = eax ^ ebx
	//	mov   edx, eax
	//	and   edx, 0xfff00000
	//	shr   edx, 20
	//	and   eax, 0x000fffff
	//	shl   eax, 12
	//	or    eax, edx
	//	xor   eax, ebx		// eax = eax ^ ebx

	//	mov   [esi], eax	// (long)*(buf) = eax

	//	add   esi, 4		// buf = buf+4  // ���� ��ȣȭ �ּҷ� �̵�
	//	dec   len			// len--

	//	jg    repeat
	//}				


	// buf�� key 32bit ���� xor��Ų��.
	// buf�� ���� 1�� ������ �˾Ƴ���.
	// �װ���ŭ shift left�� �Ѵ�.
	// �Ǿպ��� �� ���� ��Ʈ���� �� �ڷ� ������.
	// key������ �ٽ��ѹ� xor�Ѵ�.

	return 0;
}
