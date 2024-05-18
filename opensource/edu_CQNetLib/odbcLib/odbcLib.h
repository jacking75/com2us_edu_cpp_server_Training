#pragma once

#include <iostream>

#include <windows.h>
#include <sql.h>
#include <sqlext.h>

#define ERRORSQL_RETSIZE	-101
#define ERRORSQL_NORMAL		-100
#define MAX_ERRORMSG		1024 * 4



namespace MyODBCLib
{
	/*
	DBWorker Ŭ����:ODBC�� ���� Ŭ����. 
	- API ���ؿ��� ���� �����ϵ��� �ۼ��Ͽ���.
	- SQL���� ���ϰ� �����ϰ� ����� ���� �дµ� ������ ���߾����� SQL ������ �׼������� �Ϻ��ϰ� �׽�Ʈ �Ǿ���.
	- 255�� ������ ����, ���ڿ� �÷��� ���ؼ��� ��� �����ϸ� �� �̻��� ���̸� ������ �ʵ�� "������ �߸�" ���� ��� �߻��ϹǷ� �� Ŭ������ ���� �� ������ ODBC API �Լ��� ���� ����ؾ� �Ѵ�.
	- �ִ� �÷� ������ 100���̴�. �ʰ��� �������� ���� �޽��� �ڽ��� ����ϹǷ� �� �ѵ��� �˾Ƽ� ���� �ʵ��� �����ؾ� �Ѵ�. Ư�� �ʵ���� ���� ���̺��� select * �� �д� ���� �ﰡ�ϴ� ���� ����.
	- ���� ���� Ŀ���� ����ϹǷ� �̹� ���� ���ڵ�� ������ �ٽ� �����ؾ߸� ���� �� �ִ�.

	����
	1.DBWorker Ŭ������ ��ü�� �����Ѵ�. �������̸� �������� �����ϴ� ���� ������ �ʿ��� ����ŭ ��ü�� �����. ���� �� �� ������ ����ϴ�.
	2.������ Connect �Լ��� ȣ���Ͽ� ������ �ҽ��� �����Ѵ�.
	3.Exec �Լ��� SQL���� �����Ѵ�. ���� ó���� Exec �Լ� ���ο��� ó���Ѵ�.
	4.Fetch �Լ��� ��� �� �ϳ��� �����´�. �������� ������� �ִ� ���� while ������ ���鼭 ���ʴ�� ������ �� �ִ�.
	5.Get* �Լ��� �÷� ���� �д´�.
	6.Clear �Լ��� ��� �ڵ��� �ݴ´�.(Select���� ��츸)
	7.��ü�� �ı��Ѵ�. ���� ��ü�� ���� �Ϻη� �ı��� �ʿ䰡 ����.
	*/
	class DBWorker
	{
		// �ִ� �÷���, BLOB ����� ����, NULL �ʵ尪
		enum {
			MAXCOL = 100, BLOBBATCH = 10000, cQueryNULL = -100, cQueryEOF = -101,
			cQueryNOCOL = -102, cQueryERROR = -103
		};

	public:
		DBWorker()
		{
			std::cout << "SYSTEM | cQuery::cQuery() | cQuery ������ ȣ��" << std::endl;
			AffectCount = -1;
			ret = SQL_SUCCESS;
			hStmt = NULL;
			hDbc = NULL;
			hEnv = NULL;
			m_bIsConnect = false;
			ZeroMemory(m_szErrorMsg, MAX_ERRORMSG);
		}

		virtual ~DBWorker()
		{
			std::cout << "SYSTEM | cQuery::~cQuery() | cQuery �Ҹ��� ȣ��" << std::endl;
			DisConnect();
		}
		
		// ���� �߻��� ���� ������ ���ڿ��� ����� �ش�
		void PrintDiag()
		{
			INT32 ii;
			SQLRETURN Ret;
			SQLINTEGER NativeError;
			SQLCHAR SqlState[6], Msg[255];
			SQLSMALLINT MsgLen;

			ii = 1;
			/*while (Ret=SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, ii, SqlState, &NativeError,
				Msg, sizeof(Msg), &MsgLen)!=SQL_NO_DATA) {*/
			bool bLoop = true;
			while (bLoop)
			{
				Ret = SQLGetDiagRec(SQL_HANDLE_STMT, hStmt, ii, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen);
				if (SQL_NO_DATA == Ret) {
					break;
				}

				//������ ����ٸ�
				if (_strcmpi((LPCTSTR)SqlState, "08S01") == 0)
					m_bIsConnect = false;

				sprintf(m_szErrorMsg, "SQLSTATE(%s) ��������(%s)", (LPCTSTR)SqlState, (LPCTSTR)Msg);
				printf("[DBWorker::PrintDiag] %s", m_szErrorMsg);

				++ii;
			}
		}

		inline char* GetErrorMsg() { return m_szErrorMsg; }
		inline bool GetIsConnect() { return m_bIsConnect; }
		
		void DisConnect()
		{
			if (hStmt) {
				SQLFreeHandle(SQL_HANDLE_STMT, hStmt);
			}

			if (hDbc) {
				SQLDisconnect(hDbc);
			}

			if (hDbc) {
				SQLFreeHandle(SQL_HANDLE_DBC, hDbc);
			}

			if (hEnv) {
				SQLFreeHandle(SQL_HANDLE_ENV, hEnv);
			}

			m_bIsConnect = false;
		}

		// ������ �ҽ��� �����Ѵ�
		// ���� �ڵ��� �Ҵ��ϰ� ������ �� ����ڵ���� ���� �Ҵ��Ѵ�.
		// Type=1:ConStr�� MDB ������ ��θ� ������. ��� ������ ���� ���丮���� MDB�� ã�´�.
		// Type=2:ConStr�� SQL ������ ���� ������ ������ DSN ������ ��θ� ������. 
		//        ��δ� �ݵ�� ���� ��η� �����ؾ� �Ѵ�.
		// Type=3:SQLConnect �Լ��� DSN�� ���� �����Ѵ�.
		// ���� �Ǵ� ��� �ڵ� �Ҵ翡 �����ϸ� FALSE�� �����Ѵ�.
		BOOL Connect(INT32 Type, const char* ConStr, const char* UID = NULL, const char* PWD = NULL, bool bIsPrint = true)
		{
			ZeroMemory(m_szErrorMsg, MAX_ERRORMSG);

			//�̹� ������ �Ǿ��ִٸ� ������ ���´�.
			if (m_bIsConnect)
				DisConnect();

			SQLCHAR InCon[255];
			SQLCHAR OutCon[255];
			SQLSMALLINT cbOutCon;
			m_bIsPrint = bIsPrint;
			INT32 ii = 1;
			SQLRETURN Ret;
			SQLINTEGER NativeError;
			SQLCHAR SqlState[6], Msg[255];
			SQLSMALLINT MsgLen;

			// ȯ�� �ڵ��� �Ҵ��ϰ� ���� �Ӽ��� �����Ѵ�.
			SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &hEnv);
			SQLSetEnvAttr(hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, SQL_IS_INTEGER);
			// ���� Ÿ�Կ� ���� MDB �Ǵ� SQL ����, �Ǵ� DSN�� �����Ѵ�.
			SQLAllocHandle(SQL_HANDLE_DBC, hEnv, &hDbc);
			switch (Type) {
			case 1:
				wsprintf((char*)InCon, "DRIVER={Microsoft Access Driver (*.mdb)};DBQ=%s;", ConStr);
				ret = SQLDriverConnect(hDbc, NULL, (SQLCHAR*)InCon, sizeof(InCon), OutCon,
					sizeof(OutCon), &cbOutCon, SQL_DRIVER_NOPROMPT);
				break;
			case 2:
				wsprintf((char*)InCon, "FileDsn=%s", ConStr);
				ret = SQLDriverConnect(hDbc, NULL, (SQLCHAR*)InCon, sizeof(InCon), OutCon,
					sizeof(OutCon), &cbOutCon, SQL_DRIVER_NOPROMPT);
				break;
			case 3:
				ret = SQLConnect(hDbc, (SQLCHAR*)ConStr, SQL_NTS, (SQLCHAR*)UID, SQL_NTS,
					(SQLCHAR*)PWD, SQL_NTS);
				break;
			}

			// ���� ������ ���� ������ �����ش�.
			if ((ret != SQL_SUCCESS) && (ret != SQL_SUCCESS_WITH_INFO))
			{
				/*while (Ret=SQLGetDiagRec(SQL_HANDLE_DBC, hDbc, ii, SqlState, &NativeError,
					Msg, sizeof(Msg), &MsgLen)!=SQL_NO_DATA)*/
				bool bLoop = true;
				while (bLoop)
				{
					Ret = SQLGetDiagRec(SQL_HANDLE_DBC, hDbc, ii, SqlState, &NativeError, Msg, sizeof(Msg), &MsgLen);
					if (SQL_NO_DATA == Ret) {
						break;
					}

					printf("SYSTEM | cQuery::Connect() | SQLSTATE(%s), ��������(%s)", (LPCTSTR)SqlState, (LPCTSTR)Msg);

					++ii;
				}
				return FALSE;
			}

			// ��� �ڵ��� �Ҵ��Ѵ�.
			ret = SQLAllocHandle(SQL_HANDLE_STMT, hDbc, &hStmt);
			if ((ret != SQL_SUCCESS) && (ret != SQL_SUCCESS_WITH_INFO)) 
			{
				hStmt = 0;
				return FALSE;
			}

			//Exec("f");
			m_bIsConnect = true;
			return TRUE;
		}

		// SQL���� �����Ѵ�. ���н� ���� ������ ����ϰ� FALSE�� �����Ѵ�.
		BOOL Exec(LPCTSTR szSQL)
		{
			INT32 c;

			// SQL���� �����Ѵ�. SQL_NO_DATA�� ������ ��쵵 �ϴ� �������� ����Ѵ�. 
			// �� ��� Fetch���� EOF�� �����ϵ��� �Ǿ� �ֱ� ������ ���� ������ ����� �ʿ�� ����.
			ret = SQLExecDirect(hStmt, (SQLCHAR*)szSQL, SQL_NTS);
			if ((ret != SQL_SUCCESS) && (ret != SQL_NO_DATA)) {
				PrintDiag();
				return FALSE;
			}

			// Update, Delete, Insert ��ɽ� ������� ���ڵ� ������ ���� ���´�.
			SQLRowCount(hStmt, &AffectCount);
			SQLNumResultCols(hStmt, &nCol);
			if (nCol > MAXCOL) {
				printf("SYSTEM | cQuery::Exec() | �ִ� �÷� ���� �ʰ��߽��ϴ�");
				return FALSE;
			}

			// nCol�� 0�� ���� Select�� �̿��� �ٸ� ����� ������ ����̹Ƿ� 
			// ���ε��� �� �ʿ䰡 ����.
			if (nCol == 0) {
				Clear();
				return TRUE;
			}

			// ��� �÷��� ���ڿ��� ���ε��� ���´�. Col�迭�� zero base, 
			// �÷� ��ȣ�� one base�ӿ� ������ ��
			for (c = 0; c < nCol; c++) {
				SQLBindCol(hStmt, c + 1, SQL_C_CHAR, Col[c], 255, &lCol[c]);

			}
			return TRUE;
		}

		// SQL���� �����ϰ� ������� ù Į������ �������� �о� ������ �ش�. 
		// ����� ���� �� �������� �� �ش�.
		INT32 ExecGetInt(LPCTSTR szSQL)
		{
			INT32 i;

			if (Exec(szSQL) == FALSE)
				return cQueryERROR;

			// �����Ͱ� ���� ��� EOF�� �����Ѵ�.
			if (Fetch() == SQL_NO_DATA) {
				Clear();
				return cQueryEOF;
			}
			i = GetInt(1);
			Clear();
			return i;
		}

		// SQL���� �����ϰ� ������� ù Į������ ���ڿ��� �о� ������ �ش�.
		INT32 ExecGetStr(LPCTSTR szSQL, char* buf)
		{
			// SQL�� ������ ������ �߻��� ��� ���ڿ� ���ۿ� ������ ���� ������.
			if (Exec(szSQL) == FALSE) {
				return -1;
			}

			// �����Ͱ� ���� ��� EOF�� �����Ѵ�.
			if (Fetch() == SQL_NO_DATA) {
				Clear();
				return SQL_NO_DATA;
			}
			GetStr(1, buf);
			Clear();

			return 0;
		}

		// SQL���� �����ϰ� ������ ���ν������� ���ϰ��� ������.
		INT32 ExecGetReturn(LPCTSTR szSQL)
		{
			INT32 c;

			INT32 nRetProc = 0;	//������ ���ν������� ���� ��ȯ��
			SQLLEN nRetSize = 0;//��ȯ���� ũ��

			ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT_OUTPUT, SQL_INTEGER, SQL_INTEGER, 0, 0,
				&nRetProc, 0, (SQLLEN*)& nRetSize);
			if ((ret != SQL_SUCCESS)) {
				PrintDiag();
				return ERRORSQL_NORMAL;
			}
			// SQL���� �����Ѵ�. SQL_NO_DATA�� ������ ��쵵 �ϴ� �������� ����Ѵ�. 
			// �� ��� Fetch���� EOF�� �����ϵ��� �Ǿ� �ֱ� ������ ���� ������ ����� �ʿ�� ����.
			ret = SQLExecDirect(hStmt, (SQLCHAR*)szSQL, SQL_NTS);

			//���⼭ �������� �ʴ� ������ �����ڵ带 Ȯ���ϱ� ���ؼ� �׷���.
			//�׷��� �� �ؿ��� �Ѳ����� ���� �ʴ� ������ PrintDiag()���� OUTPUTSTRING�� �ϸ�
			//���� �޽����� �� ���󰡼� �׷���.. 
			//�� ������ �������� �����ν� cQuery�� ��ü������ �ٲ���ϰڴ�.
			if ((ret != SQL_SUCCESS) && (ret != SQL_NO_DATA)) {
				PrintDiag();

			}
#ifdef ERROROUT
			OUTPUTSTRING;
#endif
			if ((0 != nRetProc) || (0 == nRetSize))
			{
				PrintDiag();
				//�Լ� ��ü�� �����ؼ� ���ϰ��� ���� ���ߴٸ�
				if (0 == nRetSize)
					return ERRORSQL_RETSIZE;
				return nRetProc;
			}
			if ((ret != SQL_SUCCESS) && (ret != SQL_NO_DATA)) {
				return ret;
			}


			// Update, Delete, Insert ��ɽ� ������� ���ڵ� ������ ���� ���´�.
			SQLRowCount(hStmt, &AffectCount);
			SQLNumResultCols(hStmt, &nCol);
			if (nCol > MAXCOL) 
			{
				printf("cQuery::ExecGetReturn() | �ִ� �÷� ���� �ʰ��߽��ϴ�");
				return ERRORSQL_NORMAL;
			}

			// nCol�� 0�� ���� Select�� �̿��� �ٸ� ����� ������ ����̹Ƿ� 
			// ���ε��� �� �ʿ䰡 ����.
			if (nCol == 0) {
				Clear();
				return 0;
			}

			// ��� �÷��� ���ڿ��� ���ε��� ���´�. Col�迭�� zero base, 
			// �÷� ��ȣ�� one base�ӿ� ������ ��
			for (c = 0; c < nCol; c++) {
				SQLBindCol(hStmt, c + 1, SQL_C_CHAR, Col[c], 255, &lCol[c]);

			}
			return 0;
		}

		// SQL���� �����ϰ� ������ ���ν������� ���ϰ��� �Ѱ��� ��°��� ������.
		INT32 ExecGetReturnAndOutput(LPCTSTR szSQL, __int64& n64Output)
		{
			INT32 c;
			//������ ���ν������� ���� ��ȯ��
			INT32 nRetProc1 = 0;
			__int64 n64RetProc2 = 0;
			//��ȯ���� ũ��
			SQLLEN nRetSize1 = 0, nRetSize2 = 0;

			ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
				&nRetProc1, 0, (SQLLEN*)& nRetSize1);
			ret = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT_OUTPUT, SQL_C_SBIGINT, SQL_INTEGER, 0, 0,
				&n64RetProc2, 0, (SQLLEN*)& nRetSize2);

			if ((ret != SQL_SUCCESS))
			{
				PrintDiag();
				return ERRORSQL_NORMAL;
			}
			// SQL���� �����Ѵ�. SQL_NO_DATA�� ������ ��쵵 �ϴ� �������� ����Ѵ�. 
			// �� ��� Fetch���� EOF�� �����ϵ��� �Ǿ� �ֱ� ������ ���� ������ ����� �ʿ�� ����.
			ret = SQLExecDirect(hStmt, (SQLCHAR*)szSQL, SQL_NTS);

			if ((ret != SQL_SUCCESS) && (ret != SQL_NO_DATA)) {
				PrintDiag();
			}
#ifdef ERROROUT
			OUTPUTSTRING;
#endif
			//��ȯ���� ��ȯ���� ũ�⸦ �Ǵ��Ͽ� ������ ����
			if ((0 != nRetProc1) || (0 == nRetSize1) || (0 == nRetSize2))
			{
				PrintDiag();
				//�Լ� ��ü�� �����ؼ� ���ϰ��� ���� ���ߴٸ�
				if (0 == nRetSize1)
					return ERRORSQL_RETSIZE;
				return nRetProc1;
			}

			n64Output = n64RetProc2;
			// Update, Delete, Insert ��ɽ� ������� ���ڵ� ������ ���� ���´�.
			SQLRowCount(hStmt, &AffectCount);
			SQLNumResultCols(hStmt, &nCol);
			if (nCol > MAXCOL) 
			{
				printf("cQuery::ExecGetReturn() | �ִ� �÷� ���� �ʰ��߽��ϴ�");
				return ERRORSQL_NORMAL;
			}

			// nCol�� 0�� ���� Select�� �̿��� �ٸ� ����� ������ ����̹Ƿ� 
			// ���ε��� �� �ʿ䰡 ����.
			if (nCol == 0) {
				Clear();
				return 0;
			}

			// ��� �÷��� ���ڿ��� ���ε��� ���´�. Col�迭�� zero base, 
			// �÷� ��ȣ�� one base�ӿ� ������ ��
			for (c = 0; c < nCol; c++) {
				SQLBindCol(hStmt, c + 1, SQL_C_CHAR, Col[c], 255, &lCol[c]);

			}
			return 0;
		}
		
		INT32 ExecGetReturnAndOutput3(LPCTSTR szSQL, __int64& n64Output1, __int64& n64Output2, __int64& n64Output3)
		{
			INT32 c;
			INT32 nRetProc1 = 0;
			__int64 n64OutPut1 = 0, n64OutPut2 = 0, n64OutPut3 = 0;
			SQLLEN nRetSize1 = 0, nRetSize2 = 0, nRetSize3 = 0, nRetSize4 = 0;


			ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0, &nRetProc1, 0, (SQLLEN*)& nRetSize1);
			if ((ret != SQL_SUCCESS)) {
				PrintDiag();
				return ERRORSQL_NORMAL;
			}

			ret = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT_OUTPUT, SQL_C_SBIGINT, SQL_INTEGER, 0, 0, &n64OutPut1, 0, (SQLLEN*)& nRetSize2);
			if ((ret != SQL_SUCCESS)) {
				PrintDiag();
				return ERRORSQL_NORMAL;
			}

			ret = SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT_OUTPUT, SQL_C_SBIGINT, SQL_INTEGER, 0, 0, &n64OutPut2, 0, (SQLLEN*)& nRetSize3);
			if ((ret != SQL_SUCCESS)) {
				PrintDiag();
				return ERRORSQL_NORMAL;
			}

			ret = SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT_OUTPUT, SQL_C_SBIGINT, SQL_INTEGER, 0, 0, &n64OutPut3, 0, (SQLLEN*)& nRetSize4);
			if ((ret != SQL_SUCCESS)) {
				PrintDiag();
				return ERRORSQL_NORMAL;
			}


			// Update, Delete, Insert ��ɽ� ������� ���ڵ� ������ ���� ���´�.
			SQLRowCount(hStmt, &AffectCount);
			SQLNumResultCols(hStmt, &nCol);
			if (nCol > MAXCOL) 
			{
				printf("cQuery::ExecGetReturn() | �ִ� �÷� ���� �ʰ��߽��ϴ�");
				return ERRORSQL_NORMAL;
			}
			INT32 nColResult = nCol;

			// SQL���� �����Ѵ�. SQL_NO_DATA�� ������ ��쵵 �ϴ� �������� ����Ѵ�. 
			// �� ��� Fetch���� EOF�� �����ϵ��� �Ǿ� �ֱ� ������ ���� ������ ����� �ʿ�� ����.
			ret = SQLExecDirect(hStmt, (SQLCHAR*)szSQL, SQL_NTS);

			if ((ret != SQL_SUCCESS) && (ret != SQL_NO_DATA)) {
				PrintDiag();
			}

#ifdef ERROROUT
			OUTPUTSTRING;
#endif

			//��ȯ���� ��ȯ���� ũ�⸦ �Ǵ��Ͽ� ������ ����
			if ((0 != nRetProc1) || (0 == nRetSize1) || (0 == nRetSize2) || (0 == nRetSize3) || (0 == nRetSize4))
			{
				PrintDiag();

				//�Լ� ��ü�� �����ؼ� ���ϰ��� ���� ���ߴٸ�
				if (0 == nRetSize1)
					return ERRORSQL_RETSIZE;
				return nRetProc1;
			}

			n64Output1 = n64OutPut1;
			n64Output2 = n64OutPut2;
			n64Output3 = n64OutPut3;


			// nCol�� 0�� ���� Select�� �̿��� �ٸ� ����� ������ ����̹Ƿ� 
			// ���ε��� �� �ʿ䰡 ����.
			if (nColResult == 0) {
				Clear();
				return 0;
			}

			// ��� �÷��� ���ڿ��� ���ε��� ���´�. Col�迭�� zero base, 
			// �÷� ��ȣ�� one base�ӿ� ������ ��
			for (c = 0; c < nColResult; c++) {
				SQLBindCol(hStmt, c + 1, SQL_C_CHAR, Col[c], 255, &lCol[c]);

			}

			return 0;
		}
		
		INT32 ExecGetReturnAndOutput4(LPCTSTR szSQL, INT32& nOutput1, __int64& n64Output2, __int64& n64Output3, INT32& nOutput4)
		{
			INT32 c;
			INT32 nRetProc1 = 0, nRetProc2 = 0, nRetProc5 = 0;	//������ ���ν������� ���� ��ȯ��
			__int64 n64RetProc3 = 0, n64RetProc4 = 0;
			SQLLEN nRetSize1 = 0, nRetSize2 = 0, nRetSize3 = 0, nRetSize4 = 0, nRetSize5 = 0;//��ȯ���� ũ��

			ret = SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
				&nRetProc1, 0, (SQLLEN*)& nRetSize1);
			if ((ret != SQL_SUCCESS)) {
				PrintDiag();
				return ERRORSQL_NORMAL;
			}
			ret = SQLBindParameter(hStmt, 2, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
				&nRetProc2, 0, (SQLLEN*)& nRetSize2);
			if ((ret != SQL_SUCCESS)) {
				PrintDiag();
				return ERRORSQL_NORMAL;
			}
			ret = SQLBindParameter(hStmt, 3, SQL_PARAM_INPUT_OUTPUT, SQL_C_SBIGINT, SQL_INTEGER, 0, 0,
				&n64RetProc3, 0, (SQLLEN*)& nRetSize3);
			if ((ret != SQL_SUCCESS)) {
				PrintDiag();
				return ERRORSQL_NORMAL;
			}
			ret = SQLBindParameter(hStmt, 4, SQL_PARAM_INPUT_OUTPUT, SQL_C_SBIGINT, SQL_INTEGER, 0, 0,
				&n64RetProc4, 0, (SQLLEN*)& nRetSize4);
			if ((ret != SQL_SUCCESS)) {
				PrintDiag();
				return ERRORSQL_NORMAL;
			}
			ret = SQLBindParameter(hStmt, 5, SQL_PARAM_INPUT_OUTPUT, SQL_C_LONG, SQL_INTEGER, 0, 0,
				&nRetProc5, 0, (SQLLEN*)& nRetSize5);
			if ((ret != SQL_SUCCESS)) {
				PrintDiag();
				return ERRORSQL_NORMAL;
			}
			// Update, Delete, Insert ��ɽ� ������� ���ڵ� ������ ���� ���´�.
			SQLRowCount(hStmt, &AffectCount);
			SQLNumResultCols(hStmt, &nCol);
			if (nCol > MAXCOL) 
			{
				printf("cQuery::ExecGetReturn() | �ִ� �÷� ���� �ʰ��߽��ϴ�");
				return ERRORSQL_NORMAL;
			}
			INT32 nColResult = nCol;

			// SQL���� �����Ѵ�. SQL_NO_DATA�� ������ ��쵵 �ϴ� �������� ����Ѵ�. 
			// �� ��� Fetch���� EOF�� �����ϵ��� �Ǿ� �ֱ� ������ ���� ������ ����� �ʿ�� ����.
			ret = SQLExecDirect(hStmt, (SQLCHAR*)szSQL, SQL_NTS);

			if ((ret != SQL_SUCCESS) && (ret != SQL_NO_DATA)) {
				PrintDiag();
			}
#ifdef ERROROUT
			OUTPUTSTRING;
#endif
			//��ȯ���� ��ȯ���� ũ�⸦ �Ǵ��Ͽ� ������ ����
			if ((0 != nRetProc1) || (0 == nRetSize1) || (0 == nRetSize2) || (0 == nRetSize3) || (0 == nRetSize4) || (0 == nRetSize5))
			{
				PrintDiag();
				//�Լ� ��ü�� �����ؼ� ���ϰ��� ���� ���ߴٸ�
				if (0 == nRetSize1)
					return ERRORSQL_RETSIZE;
				return nRetProc1;
			}

			nOutput1 = nRetProc2;
			n64Output2 = n64RetProc3;
			n64Output3 = n64RetProc4;
			nOutput4 = nRetProc5;


			// nCol�� 0�� ���� Select�� �̿��� �ٸ� ����� ������ ����̹Ƿ� 
			// ���ε��� �� �ʿ䰡 ����.
			if (nColResult == 0) {
				Clear();
				return 0;
			}

			// ��� �÷��� ���ڿ��� ���ε��� ���´�. Col�迭�� zero base, 
			// �÷� ��ȣ�� one base�ӿ� ������ ��
			for (c = 0; c < nColResult; c++) {
				SQLBindCol(hStmt, c + 1, SQL_C_CHAR, Col[c], 255, &lCol[c]);

			}
			return 0;
		}
		
		// ����¿��� �� ���� �����´�.
		SQLRETURN Fetch()
		{
			ret = SQLFetch(hStmt);
			return ret;
		}
		
		// Ŀ���� �ݰ� ���ε� ������ �����Ѵ�.
		void Clear()
		{
#ifdef ERROROUT
			OUTPUTSTRING;
#endif
			SQLCloseCursor(hStmt);
			SQLFreeStmt(hStmt, SQL_UNBIND);
		}
		
		// nCol�� �÷����� ������ �о��ش�. NULL�� ��� cQueryNULL�� �����Ѵ�.
		INT32 GetInt(INT32 nCol)
		{
			if (nCol > this->nCol)
				return cQueryNOCOL;

			if (lCol[nCol - 1] == SQL_NULL_DATA) {
				return cQueryNULL;
			}
			else {
				return atoi(Col[nCol - 1]);

			}
		}
		
		// __int64 ������ �÷� �б�
		unsigned __int64 GetInt64(INT32 nCol)
		{
			if (nCol > this->nCol)
				return static_cast<unsigned __int64>(cQueryNOCOL);

			if (blCol[nCol - 1] == SQL_NULL_DATA) {
				return static_cast<unsigned __int64>(cQueryNULL);
			}
			else {
				unsigned __int64 ret = _atoi64(Col[nCol - 1]);
				return ret;
			}
		}
		
		// ���ڿ��� �÷� �б�
		// nCol�� �÷����� ���ڿ��� �о��ش�. NULL�� ��� ���ڿ��� NULL�� ä���ش�. 
		// buf�� ���̴� �ּ��� 256�̾�� �ϸ� ���� ������ ���� �ʴ´�.
		bool GetStr(INT32 nCol, char* buf)
		{
			if (nCol > this->nCol) {
				return false;
			}

			if (lCol[nCol - 1] == SQL_NULL_DATA) {
				return false;
			}
			else {
				lstrcpy(buf, Col[nCol - 1]);
			}

			return true;
		}
		
		char GetChar(INT32 nCol)
		{
			if (nCol > this->nCol) {
				return 0x00;
			}

			if (lCol[nCol - 1] == SQL_NULL_DATA) {
				return 0x00;
			}

			return Col[nCol - 1][0];
		}
		
		// BLOB �����͸� buf�� ä���ش�. �̶� buf�� ����� ũ���� �޸𸮸� �̸� �Ҵ��� ���ƾ� �Ѵ�. NULL�� ��� 0�� �����ϰ� ���� �߻��� -1�� �����Ѵ�. 
		// ������ ���� �� ����Ʈ ���� �����Ѵ�. szSQL�� �ϳ��� BLOB �ʵ带 �д� Select SQL���̾�� �Ѵ�.
		INT32 ReadBlob(LPCTSTR szSQL, void* buf)
		{
			SQLCHAR BinaryPtr[BLOBBATCH];
			SQLLEN LenBin;
			char* p;
			INT32 nGet;
			INT32 TotalGet = 0;

			ret = SQLExecDirect(hStmt, (SQLCHAR*)szSQL, SQL_NTS);
			if (ret != SQL_SUCCESS) {
				PrintDiag();
				return -1;
			}

			while ((ret = SQLFetch(hStmt)) != SQL_NO_DATA) {
				p = (char*)buf;
				while ((ret = SQLGetData(hStmt, 1, SQL_C_BINARY, BinaryPtr, sizeof(BinaryPtr),
					&LenBin)) != SQL_NO_DATA) {
					if (LenBin == SQL_NULL_DATA) {
						Clear();
						return 0;
					}
					if (ret == SQL_SUCCESS)
						nGet = LenBin;
					else
						nGet = BLOBBATCH;
					TotalGet += nGet;
					memcpy(p, BinaryPtr, nGet);
					p += nGet;
				}
			}
			Clear();
			return TotalGet;
		}
		
		// buf�� BLOB �����͸� �����Ѵ�. szSQL�� �ϳ��� BLOB �����͸� �����ϴ� Update, Insert SQL���̾�� �Ѵ�.
		void WriteBlob(LPCTSTR szSQL, void* buf, INT32 size)
		{
			SQLLEN cbBlob;
			char tmp[BLOBBATCH], * p;
			SQLPOINTER pToken;
			INT32 nPut;

			cbBlob = SQL_LEN_DATA_AT_EXEC(size);
			SQLBindParameter(hStmt, 1, SQL_PARAM_INPUT, SQL_C_BINARY, SQL_LONGVARBINARY,
				size, 0, (SQLPOINTER)1, 0, &cbBlob);
			SQLExecDirect(hStmt, (SQLCHAR*)szSQL, SQL_NTS);
			ret = SQLParamData(hStmt, &pToken);
			while (ret == SQL_NEED_DATA) {
				if (ret == SQL_NEED_DATA) {
					if (pToken == (SQLPOINTER)1) {
						for (p = (char*)buf; p < (char*)buf + size; p += BLOBBATCH) {
							nPut = min(BLOBBATCH, (char*)buf + size - p);
							memcpy(tmp, p, nPut);
							SQLPutData(hStmt, (PTR)tmp, nPut);
						}
					}
				}
				ret = SQLParamData(hStmt, &pToken);
			}
			Clear();
		}

		double GetFloat(INT32 nCol)
		{
			if (nCol > this->nCol)
				return cQueryNOCOL;

			if (lCol[nCol - 1] == SQL_NULL_DATA) {
				return cQueryNULL;
			}
			else {
				return atof(Col[nCol - 1]);

			}
		}


		

		
		SQLLEN AffectCount;					// ������� ���ڵ� ����
		SQLHSTMT hStmt;							// ��� �ڵ�. ���� ����� ���� �����Ƿ� public���� ����
		SQLSMALLINT nCol;						// �÷� ����
		SQLCHAR ColName[MAXCOL][50];			// �÷��� �̸���
		SQLLEN lCol[MAXCOL];				// �÷��� ����/���� ����
		SQLUBIGINT blCol[MAXCOL];				// �÷��� ����/���� ����

	protected:
		SQLHENV hEnv;							// ȯ�� �ڵ�
		SQLHDBC hDbc;							// ���� �ڵ�
		SQLRETURN ret;							// ���� ������ SQL���� �����
		char Col[MAXCOL][255];					// ���ε��� �÷� ����
		bool m_bIsPrint;						//	����� �޼����� �����ų�?
		bool m_bIsConnect;						//	������ �Ǿ� �ִ���
		char m_szErrorMsg[MAX_ERRORMSG];				// ���� ��Ʈ���� ����ִ�.		
	};



	class QueryCleaner
	{
	public:

		explicit QueryCleaner(DBWorker& worker) : m_pDBWorker(&worker) {}

		explicit QueryCleaner(DBWorker* worker) : m_pDBWorker(worker) {}

		~QueryCleaner() { m_pDBWorker->Clear(); }

	private:
		DBWorker* m_pDBWorker;
	};


	// ======================================================================
	// ������ ��� ������ ó���ϴ� ����, SQL_NO_DATA�� ��ȯ�� ������ ��� ������ ó���Ѵ�.
	// ======================================================================
	// �����͸� sp���� ���� ������ �̰��� �̿��ؼ� ���Ϲ����� �ȴ�.
	// �ȱ׷��� ���Ϲ������� �߸��������� �ִ�.
	//while ( (sRetCode=SQLMoreResults(hStmt)) != SQL_NO_DATA ) ;

#ifdef ERRORSQL
#define OUTPUTSTRING while( SQLMoreResults(hStmt) != SQL_NO_DATA );
#else
#define OUTPUTSTRING 
#endif

}