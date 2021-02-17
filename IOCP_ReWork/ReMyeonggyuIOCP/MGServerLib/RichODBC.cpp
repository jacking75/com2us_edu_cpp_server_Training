// RichODBC.cpp: implementation of the RichODBC class.
//
//////////////////////////////////////////////////////////////////////

#include "RichODBC.h"


#define _SIZE_ERROR_MESSAGE

#pragma comment( lib, "winmm.lib" )
#pragma warning( disable:4996 )			// strcpy, strcat

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#define REGULAR_SERVER	TRUE // TRUE�� ���� ������ �ø��� ���� , FALSE�� �׽�Ʈ ������ �ø��� ����

void RichODBC::MakeTime( TIMESTAMP_STRUCT& src, time_t& dest )
{
	// make global time_t from mssql.datetime[that localtime]

	tm tmsrc;
	
	tmsrc.tm_hour = src.hour;
	tmsrc.tm_mday = src.day;
	tmsrc.tm_min = src.minute;
	tmsrc.tm_mon = src.month - 1;
	tmsrc.tm_sec = src.second;
	tmsrc.tm_year = src.year - 1900;

	// make time_t from localtime tm
	dest = mktime(&tmsrc);

	//// test	
	//tm tmtest;	
	//// make localtime tm from time_t
	////localtime_s(&tmtest, &dest);
	//time_t curtime = time(0);
	//localtime_s(&tmtest, &curtime);
	//tmtest.tm_hour = tmtest.tm_hour;

	//TIMESTAMP_STRUCT transtest;
	//MakeTIMESTAMP_STRUCT( dest, transtest );

	return;
	
}

void RichODBC::MakeTIMESTAMP_STRUCT( time_t& src, TIMESTAMP_STRUCT& dest )
{
	// make timestamp_struct [that localtime] form time_t
	tm tmsrc;
	localtime_s( &tmsrc, &src );

	dest.day = (SQLUSMALLINT)tmsrc.tm_mday;
	dest.fraction = 0;
	dest.hour = (SQLUSMALLINT)tmsrc.tm_hour;
	dest.minute = (SQLUSMALLINT)tmsrc.tm_min;
	dest.month = (SQLUSMALLINT)(tmsrc.tm_mon + 1);
	dest.second = (SQLUSMALLINT)tmsrc.tm_sec;
	dest.year = (SQLUSMALLINT)(tmsrc.tm_year + 1900);

	dest.year = dest.year;
}

RichODBC::RichODBC()
{
	// dataaccess ����üũ�� ���ϳ�? ����üũ��ĵ� �̻��ϴ�... 
	//if(CheckMDAC("2.8") == FALSE)
	//{
	//	::exit(0);
	//}

	m_pszQuery = new char[MAX_QUERY_SIZE];
	InitODBC();

	m_sicount = 0;

	m_tBEGINSP			=	0;	
	m_tEXECSP			=	0;
	m_tFETCH			=	0;
	m_tNEXTRECORDSET	=	0;

	m_pExternErrorHandler	=	NULL;

	bIsConnected = FALSE;
}

RichODBC::~RichODBC()
{
	ReleaseODBC();

	if(NULL != m_pszQuery )
		delete[] m_pszQuery;
}

/*////////////////////////////////////////////////////////////////////
Function Name : Destructor
Param : N/A
Desc : �޸� ����
////////////////////////////////////////////////////////////////////*/
void RichODBC::ReleaseODBC()
{
	if( m_hdbc )
		SQLFreeHandle(SQL_HANDLE_DBC, m_hdbc);

	if( m_henv )
		SQLFreeHandle(SQL_HANDLE_ENV, m_henv);
}


/*////////////////////////////////////////////////////////////////////
Function Name : Constructor
Param : N/A
Desc : ��� �������� �ʱ�ȭ
////////////////////////////////////////////////////////////////////*/
void RichODBC::InitODBC()
{
	m_henv		= SQL_NULL_HENV;
	m_hdbc		= SQL_NULL_HDBC;
	m_hstmt		= SQL_NULL_HSTMT;

	m_siNTS		= SQL_NTS;
	m_siZero	= 0;
	m_siNULL	= SQL_NULL_DATA;

	m_siParamNo = 0;

	SetPooling( FALSE );

	ZeroMemory( m_pszQuery, sizeof(m_pszQuery) );
	ZeroMemory( m_cErrQuery, sizeof(m_cErrQuery) );
}

/*////////////////////////////////////////////////////////////////////
Function Name : Connect
Param1 : Driver (char*) ex) "SQL Server"
Param2 : Server (char*) ex) "111.111.111.111"
Param3 : Database (char*) ex) "TestDB"
Param4 : USER ID (char*) ex) "sa"
Param5 : Password (char*) ex) "password"
Desc : Database�� ����
return : void
////////////////////////////////////////////////////////////////////*/
BOOL RichODBC::Connect(char *cDriver, char *cServer, char *cDatabase, char *cUserID, char *cPasswd)
{
	ZeroMemory(m_cConnectionString, sizeof(m_cConnectionString));

	strcpy_s( m_cConnectionString, "DRIVER={" );
	strcat_s( m_cConnectionString, cDriver );
	strcat_s( m_cConnectionString, "};SERVER={" );
	strcat_s( m_cConnectionString, cServer );
	strcat_s( m_cConnectionString, "};DATABASE=" );
	strcat_s( m_cConnectionString, cDatabase );
	strcat_s( m_cConnectionString, ";UID=" );
	strcat_s( m_cConnectionString, cUserID );
	strcat_s( m_cConnectionString, ";PWD=" );
	strcat_s( m_cConnectionString, cPasswd );
	strcat_s( m_cConnectionString, ";APP=RichODBC" );
	strcat_s( m_cConnectionString, ";Auto Translate=FALSE" );

	if(ConnectProcess() == TRUE)
	{
		SetReconnect(FALSE);
		bIsConnected = TRUE;
		return TRUE;
	}
	else
	{
		SetReconnect(TRUE);
		bIsConnected = FALSE;
		return FALSE;
	}
}

BOOL RichODBC::ConnectProcess()
{
	SQLTCHAR	szConnStrOut[ 1024+1 ];
	SQLSMALLINT	siStrLen;

	InitODBC();

	if( IsPooling )
	{
		m_retcode = SQLSetEnvAttr( NULL, SQL_ATTR_CONNECTION_POOLING, (PTR)SQL_CP_ONE_PER_DRIVER , SQL_IS_UINTEGER );
		if( !IsSuccess() )
		{
			PrintErrMsg("1\n");
			CheckError( SQL_HANDLE_ENV, m_henv );
			return FALSE;
		}
	}

	// ODBC ȯ�� ������ ���� �޸� �Ҵ�
	// Allocate the ODBC environment and save handle.
	m_retcode = SQLAllocHandle( SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_henv );
	if( !IsSuccess() )
	{
		PrintErrMsg("2\n");
		CheckError( SQL_HANDLE_ENV, m_henv );
		return FALSE;
	}

	// ODBC ȯ�� �Ӽ� ���� (ODBC���� 3.0)
	// Notify ODBC that this is an ODBC 3.0 app.
	m_retcode = SQLSetEnvAttr( m_henv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER );
	if( !IsSuccess() )
	{
		PrintErrMsg("3\n");
		CheckError( SQL_HANDLE_ENV, m_henv );
		return FALSE;
	}

	// ODBC ������ ���� �޸� �Ҵ�
	// Allocate ODBC connection handle and connect.
	m_retcode = SQLAllocHandle( SQL_HANDLE_DBC, m_henv, &m_hdbc );
	if( !IsSuccess() )
	{
		PrintErrMsg("4\n");
		CheckError( SQL_HANDLE_ENV, m_henv );
		return FALSE;
	}

	// ODBC�� �̿��� �����ͺ��̽����� ����Ӽ� ����
	//Make sure that autocommit is set to ON.
	m_retcode = SQLSetConnectAttr( m_hdbc, SQL_ATTR_AUTOCOMMIT, (void *)SQL_AUTOCOMMIT_ON, 0 );


	// Connect.
	m_retcode = SQLDriverConnect( m_hdbc, NULL, (SQLTCHAR*)m_cConnectionString, SQL_NTS, szConnStrOut, 1024, &siStrLen, SQL_DRIVER_NOPROMPT );
	if(!IsSuccess())
	{
		PrintErrMsg("5\n");
		CheckError( SQL_HANDLE_DBC, m_hdbc );

		char szMessage[1024] = "";
		sprintf_s( szMessage, 1024, "Connect Fail \n[%s]\n\n[%s]\n", m_cConnectionString, m_cErrQuery );
		//MessageBox( NULL, szMessage, "SQLDriverConnect", MB_OK );
		printf(szMessage);
		
		return FALSE;
	}

	return TRUE;
}


BOOL RichODBC::Reconnect()
{
	WriteMSG( "Try Reconnect", 15 );
	WriteMSG( m_cConnectionString, strnlen(m_cConnectionString, sizeof(m_cConnectionString)) );
	SetReconnect(true);
	Disconnect();
	ReleaseODBC();
	bIsConnected = ConnectProcess();
	return bIsConnected;
}



/*////////////////////////////////////////////////////////////////////
Function Name : Prepare
Param1 : N/A
Desc : �ٸ� ������ �����Ͽ� �����Ҷ� ȣ���Ѵ�.
return : N/A
////////////////////////////////////////////////////////////////////*/
void RichODBC::Prepare()
{
	SetReconnect(false);

	// Allocate statement handle.
	if(m_hstmt!=NULL)
	{
		m_retcode = SQLFreeHandle(SQL_HANDLE_STMT,m_hstmt);
		if(!IsSuccess())
		{
			CheckError( SQL_HANDLE_ENV, m_henv );
		}
	}

	m_retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);
	if(!IsSuccess())
	{
		CheckError( SQL_HANDLE_ENV, m_henv );
	}

	//// 20110118 dokebi DB���� �ѹ��� �о���� Row�� ������ Ǯ���ֱ� ���� �߰���
	//// �̰� �̻� �ѹ��� �о��
	//int values = 30000;
	//SQLSetStmtAttr( m_hstmt, SQL_ATTR_MAX_ROWS, (SQLPOINTER)&values, sizeof(values) );
}


/*////////////////////////////////////////////////////////////////////
Function Name : SetQuery
Param1 : ���� (char*)
Desc : ������ ������ �Է¹���.
return : int(������ ���, ���н� ����)
////////////////////////////////////////////////////////////////////*/
int RichODBC::SetQuery(char *pszQuery)
{
	Prepare();

	/*if (sizeof(pszQuery) >= MAX_QUERY_SIZE) {
		MessageBox(NULL, L"not enougth buffer.", L"Error", MB_OK);
	}*/

	strncpy_s( m_pszQuery, sizeof(m_pszQuery), pszQuery, MAX_QUERY_SIZE );

	PrintQuery(pszQuery);

	return 1;
}

/*////////////////////////////////////////////////////////////////////
Function Name : Disconnect
Param1 : N/A
Desc : ������ �����ϰ� DB�ڵ��� ���� �����ش�.
return : int(������ ���, ���н� ����)
////////////////////////////////////////////////////////////////////*/
int RichODBC::Disconnect()
{
	m_retcode = SQLAllocHandle(SQL_HANDLE_STMT, m_hdbc, &m_hstmt);
	m_retcode = SQLFreeHandle(SQL_HANDLE_STMT, m_hstmt);

	m_retcode = SQLDisconnect(m_hdbc);

	if(IsSuccess())
		return 1;
	else
		return m_retcode;
}


/*////////////////////////////////////////////////////////////////////
Function Name : GetData
Param1 : �÷� ��ȣ
Param2 : �޾ƿ� �������� ũ��
Param3 : ������ ������ 
Desc	  : ���� �������� ���� �����͸� �޴´�.
return : int(������ ���, ���н� ����)
////////////////////////////////////////////////////////////////////*/
int RichODBC::GetData(int iColNo, void* pvData, long lDataSize)
{
	SQLLEN ntd;
	if (IsSuccess()) {
		m_retcode = SQLGetData(m_hstmt, (SQLUSMALLINT)iColNo, SQL_C_DEFAULT, pvData, lDataSize, &ntd);
	}
	else 
	{
		return -1;
	}

	return 1;
}


/*////////////////////////////////////////////////////////////////////
Function Name : GetData
Param1 : �÷� ��ȣ
Param2 : ������ Ÿ�� (SQL_INTEGER, SQL_BINARY, SQL_CHAR)
Param3 : �޾ƿ� �������� ũ��
Param4 : ������ ������ 
Desc : ���� �������� ���� �����͸� �޴´�.
return : int(������ ���, ���н� ����)
////////////////////////////////////////////////////////////////////*/
int RichODBC::GetData(int iColNo, int iType, void* pvData, long lDataSize)
{
	SQLLEN ntd;
	if(IsSuccess())
	{
		m_retcode = SQLGetData(m_hstmt, (SQLUSMALLINT)iColNo, (SQLUSMALLINT)iType, pvData, lDataSize, &ntd);
		
		if (m_retcode == -1) {
			CheckError(SQL_HANDLE_STMT, m_hstmt, m_pszQuery);
		}
	}
	else
	{
		return -1;
	}

	return 1;
}

/*////////////////////////////////////////////////////////////////////
Function Name : FetchData
Param1 : N/A
Desc : ���� �����͸� Fetch��Ų��.
return : int(������ ���, ���н� ����)
////////////////////////////////////////////////////////////////////*/
bool RichODBC::FetchData()
{	
	time( &m_tFETCH );

	m_siParamNo = 1;

	m_retcode = SQLFetch(m_hstmt);
	if(IsSuccess())
	{
		return true;
	}
	else
	{
		if (m_retcode != SQL_NO_DATA) {
			CheckError(SQL_HANDLE_STMT, m_hstmt);
		}

		return false;
	}
}

/*////////////////////////////////////////////////////////////////////
Function Name : NextRecordSet
Param1 : N/A
Desc : ���� ���ڵ������ �̵��Ѵ�.
//return : int(������ ���, ���̻� �����Ͱ� ������ 0, ���н� ����)
return : ������(true), �����Ͱ� ���ų� ���н� false
////////////////////////////////////////////////////////////////////*/
bool RichODBC::NextRecordSet()
{
	time( &m_tNEXTRECORDSET );

	m_retcode = SQLMoreResults(m_hstmt);

	if (IsSuccess()) {
		return true;
	}
	else
	{
		if (m_retcode == SQL_NO_DATA) {
			return false;
		}

		CheckError(SQL_HANDLE_STMT, m_hstmt);
		return false;
	}
}

/*////////////////////////////////////////////////////////////////////
Function Name : IsSuccess
Param1 : N/A
Desc : ODBC���� ����� �����ߴ��� �����ߴ��� �Ǵ��Ѵ�.
return : BOOL (������ TRUE, ���н� FALSE)
////////////////////////////////////////////////////////////////////*/
BOOL RichODBC::IsSuccess(void)
{
	if(m_retcode == SQL_SUCCESS)
		return TRUE;
	else if(m_retcode == SQL_SUCCESS_WITH_INFO)
		return TRUE;
	else
		return FALSE;
}


/*////////////////////////////////////////////////////////////////////
Function Name : CheckError
Param1 : �ڵ��� ���� (SQL_HANDLE_STMT, SQL_HANDLE_DBC, SQL_HANDLE_ENV���)
Param2 : �ڵ�
Param3 : �����޽����� �߰��� ǥ�õ� ��Ʈ...
Desc : ���� �߻��� �߻��� ������ ǥ���Ѵ�.
return : N/A
////////////////////////////////////////////////////////////////////*/
BOOL RichODBC::CheckError(SQLSMALLINT hType, SQLHANDLE handle, char* hint, HWND hwnd)
{
	UNREFERENCED_PARAMETER(hwnd);

	// bug 2011-2-9 err�� ���ڿ� ������ �ϸ鼭 ������ �Ѽյȴ�.
	SQLCHAR     SqlState[16];
	SQLCHAR		Msg[SQL_MAX_MESSAGE_LENGTH];
	char		caption[256] = {0,};
	char		err[2048] = {0,};
	SQLINTEGER  NativeError =0;
	SQLSMALLINT i = 1;
	SQLSMALLINT	MsgLen;
	SQLRETURN	thisRet ;
	BOOL		ret = TRUE ;	
	bool		bNeedReconnect = false;

	if( hint )
	{
		strncpy( caption, hint, sizeof(caption) );		
	}
	while(true)
	{
		// ���� ���� �켱 �ʱ�ȭ �Ѵ�.
		ZeroMemory ( SqlState, sizeof(SqlState ) );
		ZeroMemory ( Msg, sizeof(Msg ) );
		ZeroMemory ( err, sizeof(Msg ) );
		NativeError = 0;
		MsgLen = 0;
		thisRet = SQLGetDiagRecA(hType, handle, i, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen);

		if( SQL_NO_DATA == thisRet || SQL_INVALID_HANDLE == thisRet )
			break;

		if( thisRet == SQL_ERROR )
		{
			strncpy( err, "RecNumber was negative or 0", sizeof(err) );
			strncat( err, "BufferLength was less than zero", sizeof(err) );
			break;
		}

		if( hType == SQL_HANDLE_DBC && handle == m_hdbc && m_retcode == SQL_SUCCESS_WITH_INFO && !strcmp( (char*)SqlState , "01000" ) )
		{
			ret = FALSE ;
		}
		else
		{
			ret = TRUE ;
			strncpy( err, (char*)SqlState, sizeof(err) );
			strncat( err, " ", sizeof(err) );
			strncat( err, (char*)Msg, sizeof(err) );
		}
		i++;
		PrintErrMsg(err);

		if( 0 == strcmp( (char*)SqlState, "08S01" ) )
		{
			bNeedReconnect = true;
		}

	}
	//while( (thisRet = SQLGetDiagRec(hType, handle, i, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen) != SQL_NO_DATA) )
	//{
	//	if( thisRet == SQL_INVALID_HANDLE )
	//	{	
	//		//strncpy(
	//		//strcpy( err, "Invalid Handle\n" );
	//		break;
	//	}
	//	else if( thisRet == SQL_ERROR )
	//	{
	//		strcpy( err, "RecNumber was negative or 0 \n" );
	//		strcat( err, "BufferLength was less than zero \n" );
	//		break;
	//	} 

	//	if( hType == SQL_HANDLE_DBC && handle == m_hdbc && m_retcode == SQL_SUCCESS_WITH_INFO
	//		&& !strcmp( (char*)SqlState , "01000" ) )
	//	{
	//		ret = FALSE ;
	//	}
	//	else
	//	{
	//		ret = TRUE ;
	//		sprintf_s(err, "%s %s\n", SqlState, Msg);
	//		strncpy( err, (char*)SqlState, sizeof(err) );
	//		strcat_s( err, " " );
	//		strcat_s( err, (char*)Msg );
	//	}
	//	i++;
	//	PrintErrMsg(err);
	//	
	//}

	//if( !strcmp( (char*)SqlState, "08S01" ) )
	if(true == bNeedReconnect)
	{
		Reconnect();
	}
	//if( strcmp( (char*)SqlState, "01000" )	&& strcmp( (char*)SqlState, "24000" ) && strcmp( (char*)SqlState, "23000" ) && strcmp( (char*)SqlState, "42000" ) )
	//{
	//	//MessageBox(hwnd, m_cErrQuery, hint, MB_OK);
	//}
	return ret;
}

BOOL RichODBC::CheckMDAC(wchar_t *cRequire)
{
	HKEY hKey;
	wchar_t cData[256] = { 0, };
	wchar_t msg[1024] = { 0, };
	LONG result = 0;
	BOOL ret = FALSE;
	DWORD size = sizeof(cData);

	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\DataAccess", 0, KEY_READ, &hKey);
	if(result != ERROR_SUCCESS)
	{
		printf("Registry Open Failed : %d\n", result);
	}

	result = RegQueryValueEx(hKey, L"FullInstallVer", 0, NULL, (BYTE*)cData, &size);
	if(result != ERROR_SUCCESS)
	{
		printf("Registry Query Failed : %d\n", result);
	}
	else
	{
		if (wcscmp(cData, cRequire) >= 0) {
			ret = TRUE;
		}
		else
		{
			wsprintf(msg, L"�䱸���� : %s\n������� : %S", cRequire, cData);
			MessageBox(NULL, msg, L"MDAC Version", MB_OK);
		}
	}

	RegCloseKey(hKey);
	return ret;
}

/*////////////////////////////////////////////////////////////////////
Function Name : GetResult
Param1 : ����� ���� ������
Param2 : �������� �ִ�ũ��.
Desc : ������ ������ ������� ��´�. ( ������ ����� 1�ο�/1�÷��� ��츸.)
return : N/A
////////////////////////////////////////////////////////////////////*/
VOID RichODBC::GetResult(VOID *pvResult, long siDatasize)
{
	while( FetchData() )
	{
		GetData(1, SQL_C_DEFAULT, pvResult, siDatasize);
	}
}



/*////////////////////////////////////////////////////////////////////
Function Name : GetRowcount
Param1 : N/A
Desc : ���ϵ� ����� �ִ� �� ���� ��´�.
return : int
////////////////////////////////////////////////////////////////////*/
const INT64 RichODBC::GetRowcount()
{
	return m_siRowcount;
}



/*////////////////////////////////////////////////////////////////////
Function Name : SetParam
Param1 : �Ķ��Ÿ ��ȣ
Param2 : ����Ÿ Ÿ��
Param3 : ����Ÿ
Param4 : ����Ÿ ����
Desc : ������ ȣ���Ҷ� ����� �Ķ��Ÿ�� �����Ѵ�.
return : N/A
////////////////////////////////////////////////////////////////////*/
VOID RichODBC::SetParam(int siParamNo, int siType, void *pData, int siDatasize, SQLSMALLINT siParamType, bool bNull)
{
	/*///////////////////////////////////////////////////////////////////
	siType�� ����
	SQL_CHAR		: char * (���ڿ�)
	SQL_VARCHAR		: char * (���ڿ�)
	SQL_TINYINT		: unsigned char (1byte)
	SQL_SMALLINT	: small int (2byte)
	SQL_INT			: long int	(4byte)
	SQL_BIGINT		: _int64	(8byte)
	SQL_REAL		: float		(4byte)
	SQL_BINARY		: unsigned char * (�������ڿ�, �ִ���� 8000����Ʈ)
	SQL_TYPE_TIMESTAMP : TIMESTAMP_STRUCT	(16byte)
	///////////////////////////////////////////////////////////////////*/
	char buffer[100];
	ZeroMemory(buffer, sizeof(buffer));

	SQLLEN *pStrLen_or_IndPtr;


	switch(siType)
	{
	case SQL_WCHAR:
		{
			pStrLen_or_IndPtr = bNull? &m_siNULL : &m_siNTS;
			m_retcode = SQLBindParameter(m_hstmt, (SQLUSMALLINT)siParamNo, siParamType, SQL_C_WCHAR, SQL_WCHAR, siDatasize, 0, pData, 0, pStrLen_or_IndPtr);
			//PrintParam((char*)pData);
		}
		break;

	case SQL_CHAR:
		{
			pStrLen_or_IndPtr = bNull?&m_siNULL:&m_siNTS;
			m_retcode = SQLBindParameter(m_hstmt, (SQLUSMALLINT)siParamNo, siParamType, SQL_C_CHAR, SQL_CHAR, siDatasize, 0, pData, 0, pStrLen_or_IndPtr);
			PrintParam((char*)pData);
		}
		break;

	case SQL_VARCHAR:
		{
			pStrLen_or_IndPtr = bNull?&m_siNULL:&m_siNTS;
			m_retcode = SQLBindParameter(m_hstmt, (SQLUSMALLINT)siParamNo, siParamType, SQL_C_TCHAR, SQL_VARCHAR, siDatasize, 0, pData, 0, pStrLen_or_IndPtr);
			PrintParam((char*)pData);
		}
		break;

	case SQL_BIT:
		{
			pStrLen_or_IndPtr = bNull?&m_siNULL:&m_siZero;
			_itoa_s( *((char*)(pData)), buffer, 10);
#ifdef _SIZE_ERROR_MESSAGE
			if(siDatasize != sizeof(char))
			{
				wchar_t err[1024] = { 0, };
				wsprintf( err, L"ParamNo = %d, RequireSize = SQL_BIT(1Byte), InputSize = %d\n%s\n", siParamNo, siDatasize, m_pszQuery);
				
				BOOL bServer = REGULAR_SERVER ;

				if (!bServer) {
					MessageBox(NULL, err, L"DataType is Wrong", MB_OK);
				}
			}
#endif
			m_retcode = SQLBindParameter(m_hstmt, (SQLUSMALLINT)siParamNo, siParamType, SQL_C_BIT, SQL_BIT, 0, 0, pData, 0, pStrLen_or_IndPtr);
			PrintParam(buffer);
		}
		break;

	case SQL_TINYINT:
		{
			pStrLen_or_IndPtr = bNull?&m_siNULL:&m_siZero;
			_itoa_s( *((char*)(pData)), buffer, 10);
#ifdef _SIZE_ERROR_MESSAGE
			if(siDatasize != sizeof(char))
			{
				wchar_t err[1024] = { 0, };
				wsprintf( err, L"ParamNo = %d, RequireSize = SQL_TINYINT(1Byte), InputSize = %d\n%s\n", siParamNo, siDatasize, m_pszQuery);
				
				BOOL bServer = REGULAR_SERVER ;
				
				if (!bServer) {
					MessageBox(NULL, err, L"DataType is Wrong", MB_OK);
				}
			}
#endif
			m_retcode = SQLBindParameter(m_hstmt, (SQLUSMALLINT)siParamNo, siParamType, SQL_C_TINYINT, SQL_TINYINT, 0, 0, pData, 0, pStrLen_or_IndPtr);
			PrintParam(buffer);
		}
		break;

	case SQL_SMALLINT:
		{
			pStrLen_or_IndPtr = bNull?&m_siNULL:&m_siZero;
			_itoa_s( *((short*)(pData)), buffer, 10);
#ifdef _SIZE_ERROR_MESSAGE
			if(siDatasize != sizeof(short int))
			{
				wchar_t err[1024] = { 0, };
				wsprintf( err, L"ParamNo = %d, RequireSize = SQL_SMALLINT(2Byte), InputSize = %d\n%s\n", siParamNo, siDatasize, m_pszQuery);
				
				BOOL bServer = REGULAR_SERVER ;

				if (!bServer) {
					MessageBox(NULL, err, L"DataType is Wrong", MB_OK);
				}
			}
#endif
			m_retcode = SQLBindParameter(m_hstmt, (SQLUSMALLINT)siParamNo, siParamType, SQL_C_SHORT, SQL_SMALLINT, 0, 0, pData, 0,pStrLen_or_IndPtr);
			PrintParam(buffer);
		}
		break;

	case SQL_INTEGER:
		{
			pStrLen_or_IndPtr = bNull ? &m_siNULL : &m_siZero;
			_itoa_s( *((int*)(pData)), buffer, 10);
#ifdef _SIZE_ERROR_MESSAGE
			if(siDatasize != sizeof(int))
			{
				wchar_t err[2048] = { 0, };
				wsprintf( err, L"ParamNo = %d, RequireSize = SQL_INTEGER(4Byte), InputSize = %d\n%s\n", siParamNo, siDatasize, m_pszQuery);
				
				BOOL bServer = REGULAR_SERVER ;
				
				if (!bServer) {
					MessageBox(NULL, err, L"DataType is Wrong", MB_OK);
				}
			}
#endif
			m_retcode = SQLBindParameter(m_hstmt, (SQLUSMALLINT)siParamNo, siParamType, SQL_C_LONG, SQL_INTEGER, 0, 0, pData, 0, pStrLen_or_IndPtr);
			PrintParam(buffer);
		}
		break;

	case SQL_BIGINT:
		{
			pStrLen_or_IndPtr = bNull?&m_siNULL:&m_siZero;
			// bugfix 2011-2-8 18:09 2020110208 dokebi sizeof(__int64)�� 8�̶� _i64toa_s�Լ������� 8����Ʈ�ڿ� '\0'�� ���̴� �κп��� ���� �������� �߻��Ͽ��� ������ ������
			//_i64toa_s( *((__int64*)(pData)), buffer, sizeof(__int64), 10 );
			_i64toa_s( *((__int64*)(pData)), buffer, sizeof(buffer), 10 );
#ifdef _SIZE_ERROR_MESSAGE
			if(siDatasize != sizeof(__int64))
			{
				wchar_t err[2048] = { 0, };
				wsprintf( err, L"ParamNo = %d, RequireSize = SQL_BIGINT(8Byte), InputSize = %d\n%s\n", siParamNo, siDatasize, m_pszQuery);
				
				BOOL bServer = REGULAR_SERVER ;

				if (!bServer) {
					MessageBox(NULL, err, L"DataType is Wrong", MB_OK);
				}
			}
#endif
			m_retcode = SQLBindParameter(m_hstmt, (SQLUSMALLINT)siParamNo, siParamType, SQL_C_SBIGINT, SQL_BIGINT, 0, 0, pData, 0, pStrLen_or_IndPtr);
			PrintParam(buffer);
		}
		break;

	case SQL_REAL:
		{
			pStrLen_or_IndPtr = bNull?&m_siNULL:&m_siZero;
			_itoa_s( *((int*)(pData)), buffer, 10);
#ifdef _SIZE_ERROR_MESSAGE
			if(siDatasize != sizeof(float))
			{
				wchar_t err[2048] = { 0, };
				wsprintf( err, L"ParamNo = %d, RequireSize = SQL_REAL(1Byte), InputSize = %d\n%s\n", siParamNo, siDatasize, m_pszQuery);
				
				BOOL bServer = REGULAR_SERVER ;
				
				if (!bServer) {
					MessageBox(NULL, err, L"DataType is Wrong", MB_OK);
				}
			}
#endif
			m_retcode = SQLBindParameter(m_hstmt, (SQLUSMALLINT)siParamNo, siParamType, SQL_C_FLOAT, SQL_REAL, 0, 0, pData, 0, pStrLen_or_IndPtr);
			PrintParam(buffer);
		}
		break;

	case SQL_BINARY:
		{
			pStrLen_or_IndPtr = bNull?&m_siNULL:&m_siParamSize;
			m_siParamSize = siDatasize; // StrLen_or_IndPtr���ڴ� ���� Execute�ÿ� �����ϹǷ� ���������� ���� �����ϸ� �ʵȴ�.
			m_retcode = SQLBindParameter(m_hstmt, (SQLUSMALLINT)siParamNo, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_BINARY, siDatasize, 0, pData, siDatasize, pStrLen_or_IndPtr);
			PrintParam("binary data");
		}
		break;

	case SQL_TIMESTAMP:
		{
			pStrLen_or_IndPtr = bNull?&m_siNULL:&m_siZero;
			sprintf_s(buffer, 100, "%04d-%02d-%02d %02d:%02d:%02d.%03d", ((TIMESTAMP_STRUCT*)pData)->year, ((TIMESTAMP_STRUCT*)pData)->month, ((TIMESTAMP_STRUCT*)pData)->day, ((TIMESTAMP_STRUCT*)pData)->hour, ((TIMESTAMP_STRUCT*)pData)->minute, ((TIMESTAMP_STRUCT*)pData)->second, ((TIMESTAMP_STRUCT*)pData)->fraction/1000000);
#ifdef _SIZE_ERROR_MESSAGE
			if(siDatasize != sizeof(TIMESTAMP_STRUCT))
			{
				wchar_t err[2048] = { 0, };
				wsprintf( err, L"ParamNo = %d, RequireSize = SQL_TIMESTAMP(16Byte), InputSize = %d\n%s\n", siParamNo, siDatasize, m_pszQuery);
				
				BOOL bServer = REGULAR_SERVER ;
				
				if (!bServer) {
					MessageBox(NULL, err, L"DataType is Wrong", MB_OK);
				}
			}
#endif
			m_retcode = SQLBindParameter(m_hstmt, (SQLUSMALLINT)siParamNo, siParamType, SQL_C_TYPE_TIMESTAMP, SQL_TYPE_TIMESTAMP, 23, 0, pData, 16, pStrLen_or_IndPtr);
			PrintParam(buffer);
		}
		break;

	}

	if(!IsSuccess())
	{
		bCanFetchRow = FALSE;
		CheckError(SQL_HANDLE_STMT, m_hstmt, m_pszQuery);
		WriteQuery();
	}
}


void RichODBC::PrintQuery(char *msg)
{
	ZeroMemory(m_cErrQuery, sizeof(m_cErrQuery));
	sprintf_s(m_cErrQuery, 8192, "%s\n", msg);
}

void RichODBC::PrintErrMsg(char *msg)
{
	strcat(m_cErrQuery, "\n\t");
	strcat(m_cErrQuery, msg);
	strcat(m_cErrQuery, "\n");
}

void RichODBC::PrintParam(char *msg)
{
	strcat(m_cErrQuery, "\t\t");
	strcat(m_cErrQuery, msg);
	strcat(m_cErrQuery, "\n");
}

void RichODBC::WriteQuery()
{
	SYSTEMTIME CT;
	GetLocalTime(&CT);

	char szFileName[256] = "";
	sprintf_s(szFileName, 256, ".\\Log\\ErrorQuery_%02d%02d%02d.log", CT.wYear, CT.wMonth, CT.wDay);

	fopen_s( &m_Queryfp, szFileName, "a+" );

	fprintf(m_Queryfp, "%02d/%02d/%02d %02d:%02d:%02d %04d ==>> %s \n", CT.wYear, CT.wMonth, CT.wDay, CT.wHour, CT.wMinute, CT.wSecond, CT.wMilliseconds, m_cErrQuery );

	fclose(m_Queryfp);
}

void RichODBC::WriteMSG(char* msg, size_t length)
{
	UNREFERENCED_PARAMETER(length);

	SYSTEMTIME CT;
	GetLocalTime(&CT);

	char szFileName[256] = "";
	sprintf_s(szFileName, 256, ".\\Log\\ErrorQuery_%02d%02d%02d.log", CT.wYear, CT.wMonth, CT.wDay);

	fopen_s( &m_Queryfp, szFileName, "a+" );

	fprintf(m_Queryfp, "%02d/%02d/%02d %02d:%02d:%02d %04d ==>> %s \n", CT.wYear, CT.wMonth, CT.wDay, CT.wHour, CT.wMinute, CT.wSecond, CT.wMilliseconds, msg );

	fclose(m_Queryfp);
}

//*/////////////////////////////////////////////////////////
void RichODBC::BeginSP( char *pszQuery )
{
	if(FALSE == bIsConnected)
	{
		Reconnect();
	}
	m_tEXECSP			=	0;
	m_tFETCH			=	0;
	m_tNEXTRECORDSET	=	0;

	time( &m_tBEGINSP );	

	Prepare();

	m_siParamNo = 0;

	strcpy( m_pszQuery, "{call " );
	strcat( m_pszQuery, pszQuery );

	PrintQuery( pszQuery );
}

/*////////////////////////////////////////////////////////////////////
Function Name : ExecSQL
Param1 : N/A
Desc : ���õ� ������ �����Ѵ�.
return : int(������ ���, ���н� ����)
////////////////////////////////////////////////////////////////////*/
int RichODBC::ExecSQL()
{
	DWORD	dwtimegettime;
	DWORD	dwtimeGap;

	dwtimegettime	=	timeGetTime();

	m_retcode = SQLExecDirectA(m_hstmt,(SQLCHAR*)(LPSTR)(LPCSTR)m_pszQuery, SQL_NTS);

	time( &m_tEXECSP );

	if( ( dwtimeGap = timeGetTime() - dwtimegettime ) >= 1000 )
	{
		if( m_pExternErrorHandler )
		{
			m_pExternErrorHandler( m_pszQuery, dwtimeGap );
		}
	}

	if(!IsSuccess())
	{
		bCanFetchRow = FALSE;
		CheckError(SQL_HANDLE_STMT, m_hstmt, m_pszQuery);
		WriteQuery();
		return 0;
	}
	else
	{
		return 1;
	}
}

void RichODBC::EndSP()
{
	if( m_siParamNo == 0 )
	{
		strcat( m_pszQuery, "}" );
	}
	else
	{
		strcat( m_pszQuery, " )}" );
	}
}

void RichODBC::SetParam( int siType, void *pData, int siDatasize, SQLSMALLINT siParamType, bool bNull )
{
	if( m_siParamNo == 0 )
		strcat( m_pszQuery, " ( ?" );
	else
		strcat( m_pszQuery, ", ?" );

	SetParam( ++m_siParamNo, siType, pData, siDatasize, siParamType, bNull );
}


int RichODBC::GetData(void *pvData, long siDataSize)
{
	return GetData( m_siParamNo++, pvData, siDataSize);
}


VOID RichODBC::SetExternErrorHandler( pExternErrorHandler pexternerrorhandler )
{
	m_pExternErrorHandler	=	pexternerrorhandler;
}
