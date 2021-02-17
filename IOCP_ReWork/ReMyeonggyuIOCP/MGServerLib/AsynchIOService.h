#pragma once


#include "Asios.h"
#include "Thread.h"
#include "Synchronized.h"
#include <deque>
#include <map>


// assocuid�� ���� �����Ұ��ΰ�?
//	�������� AsynchSocketŬ���� �������� �߾Ӱ����޾���.
//		�׷��� �� Ư¡�� ���������� ������� ��ȣ�� �߾����߰����Ǿ����� ���·� �Ǿ� �ִ�.
//		�� AsynchIOService�� AsynchSocket���� assockid�� ����� ���·� �����Ǿ� �ִ�.
//		������ �����ÿ� �۷ι��ϰ� ����ȭ�� �Ͼ��.
//		�Ȱ��� ���ù��� ������ ������ ��ȣ�� ����� �������� asio ��ü�� �����Ұ��ΰ�?

class AsynchIOService : public INetworkSender, public Runnable
{
protected:
	INetworkReceiver*		receiver;
	DWORD					conThread;
	std::deque< Thread* >	workers;
	size_t					ioMaxSize;
	HANDLE					iocpHandle;
	
	typedef std::map<ULONG_PTR, AsynchSocket*> tASSOCKMAP;
	typedef std::map<ULONG_PTR, AsynchSocket*>::iterator tASSOCKMAPITER;
	tASSOCKMAP theASSocks;

	CriticalSectionLockWrapper		entireSynch;
	CriticalSectionLockWrapper		ioFrameSynch;
	CriticalSectionLockWrapper		logSynch;

	std::deque< OverlappedOperation* >	ioFramePools;

	char						name[ 128 ];

private:
	static size_t			cIOMINSIZE;

protected:
	OverlappedOperation* retrieveIOFrame();
	void releaseIOFrame(OverlappedOperation* op);

	void releaseSocket(AsynchSocket* socket, DWORD why);


public:
	static size_t GetIOMinSize()
	{
		return cIOMINSIZE;
	}

public:
	// receiver, 
	AsynchIOService(INetworkReceiver* receiver, size_t ioMaxSize = cIOMINSIZE, DWORD conThread=1, char* aname = NULL);
	virtual ~AsynchIOService();

	DWORD start();

	DWORD stop();

protected:
	void dispatchError(DWORD error, DWORD bytesTransfer, LPOVERLAPPED lpoverlapped, ULONG_PTR keyValue);
	void dispatchCompletion(DWORD bytesTransfer, LPOVERLAPPED lpoverlapped, ULONG_PTR keyValue);
	DWORD postingRead(AsynchSocket* socket, OverlappedOperation* op);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementation for INetworkSender
public:
	//virtual DWORD postingSend(ASSOCKUID socketUniqueId, LARGE_INTEGER* tick, size_t length, char* data);
	virtual DWORD postingSend(ASSOCKDESC& sockdesc, size_t length, char* data);
	virtual DWORD disconnectSocket(ULONG_PTR uniqueId, LARGE_INTEGER* tick);
	virtual DWORD disconnectSocket(ASSOCKDESC& sockdesc);
	virtual DWORD connectSocket(INT32 reqeusterID, AsynchSocket* prototype, char* ip, u_short port);
	virtual DWORD releaseSocketUniqueId(ULONG_PTR socketUniqueId );
	virtual DWORD registerSocket(SOCKET sockid, AsynchSocket* prototype, SOCKADDR_IN& addrin);
	virtual size_t getIOMaxSize() { return ioMaxSize; }

	void logmsg(char * format,...);

//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
// implementation for Runnable
public:
	void Run(Thread* info);

//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////



};