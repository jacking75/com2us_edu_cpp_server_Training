#include "Asios.h"

// ���� AsynchIOService�� �Ʒ��� ��ü�� ����Ѵٸ�
//	assockuidGen ��ü�� synchronization�� �����ؾ� �Ѵ�.
ULONG_PTRGenerator			AsynchSocket::assockuidGen;
CriticalSectionLockWrapper	AsynchSocket::assockuidGenLock;

AsynchSocket* AsynchSocket::sAsynchSocketPrototype = NULL;

bool AsynchSocket::disconnect()
{
	if(INVALID_SOCKET == sockID) return false;

	// iocp�� ��û�� ��� �۾��� ��ҽ�Ų��.
	//CancelIo((HANDLE)getHashCode());
	closesocket(sockID);

	// �������� �����Ѵ�.
	closed = 1;	

	// �̰��� �������� ������, ���۷���ī���Ϳ� ������ �����.
	// ������ ������ ���Ͽ� �۾��� ��û�ϸ� iocp�� ���� �ش� ��û �۾��� ���� �Ϸᰡ �Ͼ�� �ʴ� ��찡 ���� �ִ�.
	sockID = INVALID_SOCKET;

	return true;
}

size_t AsynchSocket::makePacket(char* dest, size_t destmaxsize, char* src, size_t srcsize)
{
	// how validation destmaxsize???

	if(0 >= srcsize || destmaxsize < srcsize) return (size_t)0;

	CopyMemory(dest, src, srcsize);

	return srcsize;
}

int AsynchSocket::handleCompletionOfReceive(INetworkReceiver* receiver, INetworkSender* sender, DWORD bytesTransfer, char* datas)
{
	ASSOCKDESCEX desc;
	getASSOCKDESCEX(desc);
	//desc.psender
	receiver->notifyMessage(desc, (size_t)bytesTransfer, datas);
	return 0;
}