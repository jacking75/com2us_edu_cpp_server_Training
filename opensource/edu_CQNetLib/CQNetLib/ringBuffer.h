#pragma once

#include "commonDef.h"
#include "spinLock.h"

namespace CQNetLib
{

	class RingBuffer
	{
	public:
		RingBuffer(void)
		{
		}

		virtual ~RingBuffer(void)
		{
			if (m_pBeginMark != nullptr)
			{
				delete[] m_pBeginMark;
			}
		}

		//������ �޸� �Ҵ�
		bool Create(const INT32 nBufferSize = MAX_RINGBUFSIZE)
		{
			SAFE_DELETE_ARRAY(m_pBeginMark);

			m_pBeginMark = new char[nBufferSize];

			if (m_pBeginMark == nullptr)
			{
				return false;
			}

			m_pEndMark = m_pBeginMark + nBufferSize - 1;
			m_nBufferSize = nBufferSize;

			Initialize();
			return true;
		}
		
		//�ʱ�ȭ
		bool Initialize()
		{
			SpinLockGuard lock(m_Lock);

			m_nUsedBufferSize = 0;
			m_pCurrentMark = m_pBeginMark;
			m_pGettedBufferMark = m_pBeginMark;
			m_pLastMoveMark = m_pEndMark;
			m_uiAllUserBufSize = 0;

			return true;
		}

		//�Ҵ�� ���� ũ�⸦ ��ȯ�Ѵ�.
		inline INT32 		GetBufferSize() { return m_nBufferSize; }

		//�ش��ϴ� ������ �����͸� ��ȯ
		inline char*	GetBeginMark() { return m_pBeginMark; }
		inline char*	GetCurrentMark() { return m_pCurrentMark; }
		inline char*	GetEndMark() { return m_pEndMark; }

		char* ForwardMark(INT32 nForwardLength)
		{
			SpinLockGuard lock(m_Lock);

			char* pPreCurrentMark = nullptr;

			//������ �����÷� üũ
			if (m_nUsedBufferSize + nForwardLength > m_nBufferSize)
			{
				return nullptr;
			}

			if ((m_pEndMark - m_pCurrentMark) >= nForwardLength)
			{
				pPreCurrentMark = m_pCurrentMark;
				m_pCurrentMark += nForwardLength;
			}
			else
			{
				//��ȯ �Ǳ� �� ������ ��ǥ�� ����
				m_pLastMoveMark = m_pCurrentMark;
				m_pCurrentMark = m_pBeginMark + nForwardLength;
				pPreCurrentMark = m_pBeginMark;
			}

			return pPreCurrentMark;
		}

		char* ForwardMark(INT32 nForwardLength, INT32 nNextLength, UINT32 dwRemainLength)
		{
			SpinLockGuard lock(m_Lock);

			//������ �����÷� üũ
			if (m_nUsedBufferSize + nForwardLength + nNextLength > m_nBufferSize)
			{
				return nullptr;
			}

			if ((m_pEndMark - m_pCurrentMark) > (nNextLength + nForwardLength))
			{
				m_pCurrentMark += nForwardLength;
			}			
			else //���� ������ ���̺��� ������ ���� �޼����� ũ�� 
			{
				//���� �޼����� ó������ ������ ���� ��ȯ �ȴ�.

				//��ȯ �Ǳ� �� ������ ��ǥ�� ����
				m_pLastMoveMark = m_pCurrentMark;
				CopyMemory(m_pBeginMark, m_pCurrentMark - (dwRemainLength - nForwardLength), dwRemainLength);
				m_pCurrentMark = m_pBeginMark + dwRemainLength;
			}

			return m_pCurrentMark;
		}

		void BackwardMark(const char* pBackwardMark)
		{
			SpinLockGuard lock(m_Lock);
			m_pCurrentMark = const_cast<char*>(pBackwardMark);
		}

		//���� ���� ����
		void ReleaseBuffer(INT32 nReleaseSize)
		{
			SpinLockGuard lock(m_Lock);
			m_nUsedBufferSize -= nReleaseSize;
		}

		//���� ���� ũ�� ��ȯ
		INT32 GetUsedBufferSize() { return m_nUsedBufferSize; }
		
		//���� ���۾� ����(�̰��� �ϴ� ������ SendPost()�Լ��� ��Ƽ �����忡�� ���ư��⶧����
		//PrepareSendPacket()����(ForwardMark()����) ���� ���� �÷������� PrepareSendPacket�Ѵ����� �����͸�
		//ä�� �ֱ����� �ٷ� �ٸ� �����忡�� SendPost()�� �Ҹ��ٸ� ������ ������ �����Ͱ� �� �� �ִ�.
		//�װ� �����ϱ� ���� �����͸� �� ä�� ���¿����� ���� ���� ����� ������ �� �־���Ѵ�.
		//���Լ��� sendpost�Լ����� �Ҹ��� �ȴ�.
		void SetUsedBufferSize(INT32 nUsedBufferSize)
		{
			SpinLockGuard lock(m_Lock);

			m_nUsedBufferSize += nUsedBufferSize;
			m_uiAllUserBufSize += nUsedBufferSize;
		}

		//���� ���� ���� ��ȯ
		INT32 GetAllUsedBufferSize() { return m_uiAllUserBufSize; }


		//���� ������ �о ��ȯ
		char* GetBuffer(INT32 nReadSize, INT32& refReadSize)
		{
			SpinLockGuard lock(m_Lock);

			char* pRet = nullptr;

			//���������� �� �о��ٸ� �� �о���� ������ �����ʹ� �Ǿ����� �ű��.
			if (m_pLastMoveMark == m_pGettedBufferMark)
			{
				m_pGettedBufferMark = m_pBeginMark;
				m_pLastMoveMark = m_pEndMark;
			}

			//���� ���ۿ� �ִ� size�� �о���� size���� ũ�ٸ�
			if (m_nUsedBufferSize > nReadSize)
			{
				//�������� ������ �Ǵ�.
				if ((m_pLastMoveMark - m_pGettedBufferMark) >= nReadSize)
				{
					refReadSize = nReadSize;
					pRet = m_pGettedBufferMark;
					m_pGettedBufferMark += nReadSize;
				}
				else
				{
					refReadSize = (INT32)(m_pLastMoveMark - m_pGettedBufferMark);
					pRet = m_pGettedBufferMark;
					m_pGettedBufferMark += refReadSize;
				}
			}
			else if (m_nUsedBufferSize > 0)
			{
				//�������� ������ �Ǵ�.
				if ((m_pLastMoveMark - m_pGettedBufferMark) >= m_nUsedBufferSize)
				{
					refReadSize = m_nUsedBufferSize;
					pRet = m_pGettedBufferMark;
					m_pGettedBufferMark += m_nUsedBufferSize;
				}
				else
				{
					refReadSize = (INT32)(m_pLastMoveMark - m_pGettedBufferMark);
					pRet = m_pGettedBufferMark;
					m_pGettedBufferMark += refReadSize;
				}
			}

			return pRet;
		}


	private:
		SpinLock m_Lock;

		char*			m_pBeginMark = nullptr;			//������ ó���κ��� ����Ű�� �ִ� ������
		char*			m_pEndMark = nullptr;				//������ �������κ��� ����Ű�� �ִ� ������
		char*			m_pCurrentMark = nullptr;			//������ ���� �κ��� ����Ű�� �ִ� ������
		char*			m_pGettedBufferMark = nullptr;	//������� �����͸� ���� ������ ������
		char*			m_pLastMoveMark = nullptr;		//ForwardMark�Ǳ� ���� ������ ������

		INT32 				m_nBufferSize = 0;		//������ �� ũ��
		INT32 			m_nUsedBufferSize = 0;	//���� ���� ������ ũ��
		UINT32			m_uiAllUserBufSize = 0; //�� ó���� �����ͷ�


	private:
		// No copies do not implement
		RingBuffer(const RingBuffer &rhs);
		RingBuffer &operator=(const RingBuffer &rhs);
	};

}