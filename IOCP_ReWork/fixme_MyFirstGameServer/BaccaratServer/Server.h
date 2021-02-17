// Server.h: interface for the CServer class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _SERVER_H_
#define _SERVER_H_

//#include <windows.h>
#include "define.h"


class CProcess;
class CChannel;
class CRoom;
class CDataBase;
class CGame;

class CServer  
{
public:
	int Initialize();
	void FinalCleanup();
	
	CServer();
	virtual ~CServer();

private:
	int InitServerInfo();
	
private:
	void DeleteCS();
	void CloseServerSocket();
	
	
};




// ������ ���� ���ؽ�Ʈ
typedef struct
{
	SOCKET			sockListener;		// Ŭ���̾�Ʈ�� ������ �޴� ����
	HANDLE			hIocpWorkTcp,		// IOCP �۾� �ڵ�,
					hIocpAcpt;			// ICOP Accept �ڵ�

	SOCKETCONTEXT	*sc;

	OBJECTNODE		*pn,				// ������ ���
					*rn;				// ���� ���

	CProcess        *ps;
	CChannel        *ch;
	CRoom           *rm;
	CDataBase		*db;
	CGame			*gameproc;

	WCHAR			wcServerID[MAXSERVERID];		// ���� �ӽ��� ID
	WCHAR			wcGameID[MAXGAMEID];			// �������� �����ϴ� ������ ID
	WCHAR			wcDBServerIP[20];				// ���� DB ������ IP
	WCHAR			wcDataBase[20];					// ���� DB�� ����Ÿ���̽� �̸�
	int				iVersion,						// Ŭ���̾�Ʈ ����( ���� 1.0 - 1000 )
					iPortNum;    					// ��Ʈ��ȣ
	
	int				iMaxUserNum,			// �ִ� ������ ��
					iCurUserNum;			// ���� ������ ��
	
	char			iInWorkerTNum,			// �ִ� ��Ŀ������ ����
					iInDataBaseTNum;		// �ִ� ����Ÿ���̽� ������ ����
	BOOL			bThreadStop;			// ������ �۵��� ���� ��Ų��.
					
	char			iMaxProcess,			// �ִ� ���μ��� ��
					iMaxChannelInProcess,	// ���μ����� �����ϴ� �ִ� ä�� ��
					iMaxChannel,			// �ִ� ä�� ��
					iMaxRoomInChannel,		// ä�δ� �ִ� ���� ��
					iMaxUserInRoom;			// ���� �ִ� ���� ��
					
	int				iMaxRoom,				// ������ �ִ� �� ��
					iMaxUserInChannel;		// ä���� �ִ� ���� ��
						
	char			cChannelHighNum,		// ���������� ��� ä�� ��
					cCnannelMiddleNum,		// ���������� �߼� ä�� ��
					cChannelLowNum,			// ���������� �ϼ� ä�� ��
					cLevel_Middle,			// �߼� ä�ο� ��� �� �� �ִ� ����	
					cLevel_High;			// ��� ä�ο� ��� �� �� �ִ� ����

	USERLEVELBOUNDARY	ULBoundary[12];      // ��,��,�� ������ �ּҿ� �ִ��� ���
	BASEBETTINGMONEY	BaseBettingMoney;	// ��ī ���ӿ����� ������ ���� �⺻ ���� �ݾ�.
	
	time_t	tMeasureLifeTime;	// ���� ���μ������� Ŭ���̾�Ʈ ���� ��/���� üũ�� �ð�.
	time_t	tMeasureRoomTime;	// ���� ���μ������� ���� ���¸� üũ�� �ð�.

	int		iCheckLifeTime;		// Ŭ���̾�Ʈ�� ��� �ִ��� üũ �ϴ� �ð�.
	int		iGameReadyTime;		// ��⿡�� ���� ���� �� �������� �ð�(�� ����)
	int		iGameOnTime;		// Ŭ���̾�Ʈ���� ���� ���� �غ� �� �� �ִ� �ð�.
	int		iThinkTime;			// ���� �Ͽ��� �ִ� ���� �ð�.

	CRITICAL_SECTION	csKickUserIndexList; // ű�� ��ų ������ �ε��� ����Ʈ�Ӱ迵��.
	deque<int>	dKickUserIndexList;			 // ű�� ��ų ������ �ε��� ����Ʈ.	

}SERVERCONTEXT, *LPSERVERCONTEXT;

extern	SERVERCONTEXT	ServerContext;

#endif // !defined(AFX_SERVER_H__C2166BB5_0356_4499_9A35_981EEF764368__INCLUDED_)
