/*-------------------------------------------------------------------------------------------------*/
//	File			:	GameStruct.h
//  Date			:	08.02.11
//  Author			:	�����
//	Description		:	Struct ����
/*-------------------------------------------------------------------------------------------------*/


#pragma once
struct P2P_PACKET_BASE
{
	unsigned short usLength;
	unsigned short usType;
};

//Peer To Peer �⺻ ���
struct PTPPACKET_BASE : public P2P_PACKET_BASE
{
	DWORD		   dwSequenceNo;		//UDP��Ŷ�� ���� ����
	DWORD		   dwPKey;				//��Ŷ�� ���� ������ PKey
	DWORD		   dwOtherPKey;			//��Ŷ�� ���� ������ PKey
	
	PTPPACKET_BASE()
	{
		dwSequenceNo = 0;
		dwPKey = 0;
		dwOtherPKey = 0;
	}
};



/*--------------------------------------------------------------------------------------------------*/
//���� ���� ���� ����ü
/*--------------------------------------------------------------------------------------------------*/

//���� ���� �Ӽ�
struct SERVERSTART_ATTRIBUTE
{
	//DWORD			dwServerKey;					//����Ű
	eServerType		ServerType;						//����Ÿ�� (Ű�� ���)
	
	char			szLogFileName[MAX_FILENAME];	//�α������̸�
	
	INT32 				nProcessPacketCnt;
	INT32 				nSendBufCnt;
	INT32 				nRecvBufCnt;
	INT32 				nSendBufSize;
	INT32 				nRecvBufSize;
	
	INT32 				nMaxConnectionCnt;
	INT32 				nMaxServerConnCnt;
	INT32 				nMaxTempUserInfoCnt;
	
	//�ִ� ����� �� (Ŀ�´�Ƽ���� ���)
	INT32 				nMaxUserCnt;

	INT32 				nStartClientPort;
	INT32 				nStartServerPort;
	INT32 				nStartUDPPort;

	//�ִ� UDP  ���� (�����̼��� ���)
	INT32 				nMaxUDPCnt;

	INT32 				nWorkerThreadCnt;
	INT32 				nProcessThreadCnt;

	//���� ���� (���Ӽ��� ���)
	INT32 				nMaxGameRoomPageCnt;
	INT32 				nMaxGameRoomPerPageCnt;

	//ODBC ����
	char			szODBCNickName[MAX_ODBCNAME];
	char			szODBCId[MAX_ODBCNAME];
	char			szODBCPw[MAX_ODBCNAME];
	
	//���Ӽ��� ���� ���� (�α伭�� ���)
	INT32 				nGS_ProcessPacketCnt;
	INT32 				nGS_SendBufCnt;
	INT32 				nGS_RecvBufCnt;
	INT32 				nGS_SendBufSize;
	INT32 				nGS_RecvBufSize;

	SERVERSTART_ATTRIBUTE()
	{
		Init();
	}
	
	void Init()
	{
		ZeroMemory( this, sizeof( SERVERSTART_ATTRIBUTE ) );
	}
};

//���� ���μ��� ���� �Ӽ�
struct SERVERCONNECT_ATTRIBUTE
{
	eServerType		ServerType;						
	char			szConnectIP[MAX_IP_LENGTH];
	INT32 				nConnectPort;

	INT32 				nProcessPacketCnt;			
	INT32 				nSendBufCnt;
	INT32 				nRecvBufCnt;
	INT32 				nSendBufSize;
	INT32 				nRecvBufSize;

	SERVERCONNECT_ATTRIBUTE()
	{
		Init();
	}

	void Init()
	{
		ZeroMemory( this, sizeof( SERVERCONNECT_ATTRIBUTE ) );
	}

};


//���� ���� ���� �Ӽ� (���� ������ ���)
struct CONNSERVERSTATUS_ATTRIBUTE
{
	eConnectionType			ConnType;						//����Ÿ��
	char					szConnIP[ MAX_IP_LENGTH ];		//����IP
	INT32 						nConnPort;						//������Ʈ
	short					sConnCnt;						//����ȼ��ϰ���
 
	CONNSERVERSTATUS_ATTRIBUTE()
	{
		Init();
	}
	
	void Init()
	{
		ZeroMemory( this, sizeof( CONNSERVERSTATUS_ATTRIBUTE ) );
	}
};


//���� ���μ��� ���� �Ӽ� (���� ������ ���)
struct SERVERSTATUS_ATTRIBUTE
{
	eServerType		ServerType;								//����Ÿ�� (Ű�λ��)
	eServerStatus	ServerStatus;							//��������

	INT32 				nUsedCpu;								//CPU ��뷮
	INT32 				nUsedRam;								//�޸𸮻�뷮
	INT32 				nUserCnt;								//���������ڼ�(�������ӵȼ�������)
	
	CONNSERVERSTATUS_ATTRIBUTE	ConnServerStatusAttr[ MAX_CONNSERVER ];
	
	SERVERSTATUS_ATTRIBUTE()
	{
		Init();
	}
	
	void Init()
	{
		ZeroMemory( this, sizeof( SERVERSTATUS_ATTRIBUTE ) );
	}
};

// ĳ���� ������
struct CharItem
{
	void ClearCharItem()
	{
		cIsLocked		= 0;
		i64CharItemCd	= 0;
		i64CharCd		= 0;
		iItemCd			= 0;
		iGetTime		= 0;
		iUseStartTime	= 0;
		iUsedTime		= 0;
	}

	char			cIsLocked;		// ��ŷ ����.
	UINT64			i64CharItemCd;	// ĳ���� ������ �ڵ�
	UINT64			i64CharCd;		// ĳ���� �ڵ�
	INT32 			iItemCd;		// ������ �ڵ�		
	INT32 			iGetTime;		// �Լ� �ð�(��)
	INT32 			iUseStartTime;	// ����� ������ �ð� �ð�(��)
	INT32			iUsedTime;		// ����� �ð�.(������� ���� �� ���)
};
// ĳ���� ������ - ����
struct CharItemGame : CharItem
{
	void Clear()
	{
		ClearCharItem();

		sCurUseCount = 0;
	}

	short			sCurUseCount;	// ���� ��� Ƚ��	
};
// ĳ���� ������ - ���
struct CharItemState : CharItem
{
	void Clear()
	{
		ClearCharItem();

		cIsCurUse = 0;
	}

	char			cIsCurUse;		// ���� ��� ����	
};
// ĳ���� ������ - ����
struct CharItemArrangement : CharItem
{
	void Clear()
	{
		ClearCharItem();

		cIsCurUse	= 0;
		fPosX		= 0;
		fPosY		= 0;
		fPosZ		= 0;
	}

	char			cIsCurUse;		// ���� ��� ����	
	float			fPosX;			// ��ġ X
	float			fPosY;			// ��ġ Y
	float			fPosZ;			// ��ġ Z
};

// �������� ������ ����
struct WearCharItem
{
	UINT64	i64CharItemCd;
	short	sInvenArrayNum;	// ������ �κ������� ��ġ.
	INT32 		iItemCd;		// ������ �ڵ�.
};


// �ְ��� ���� �亯
struct SubjectiveQuizAnswer
{
	SubjectiveQuizAnswer()
	{
		i64Code = 0;
		memset( acAnswer, 0, sizeof(acAnswer) );
		iValue = 0;
	}

	UINT64 i64Code;
	char	acAnswer[MAX_SUBJECTIVE_QUESTION_ANSWER_LEN];
	INT32 		iValue;
};

// �ְ��� ���� ����
struct SubjectiveQuiz
{
	void Init()
	{
		bEnableUse = true;
		iQuizCd = 0;
		cKind = 0;
		sQuestionLen = 0;
		memset(acQuestion, 0, MAX_SUBJECTIVE_QUESTION_LEN);
		Answers.clear();
	}

	bool	bEnableUse;								// ��� ���� ����
	INT32 	iQuizCd;								// ���� �ڵ�
	char	cKind;									// ���� ����
	short	sQuestionLen;							// ���� ���� ����
	char	acQuestion[MAX_SUBJECTIVE_QUESTION_LEN];// ����

	char	acMixedAnswerNum[ SUBJECTIVE_QUIZ_ANSWER_TOTAL_COUNT+1 ]; // �亯 �ε����� ���� ����
	vector<SubjectiveQuizAnswer> Answers;			// �亯
};