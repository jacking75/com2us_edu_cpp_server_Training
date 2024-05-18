#pragma once

#include <Windows.h>


class AsyncIOException
{
	DWORD		errorcode;
	char		msg[128];
public:

	AsyncIOException(DWORD error, char* amsg) : errorcode(error)
	{
		ZeroMemory(msg, sizeof(msg));
		if (amsg)
		{
			size_t len = strlen(amsg);
			if (sizeof(msg) > len)
			{
				strncpy_s(msg, amsg, len);
			}
		}

		// �߻��ڵ��α�
		//	�߻��� Ư�����Ͽ� �α׸� �����,
		//	�ݽ����� �������ر��� ����� �ش�.
	}

	virtual ~AsyncIOException()
	{
	}

	const char* toMsg()
	{
		return msg;
	}

	DWORD toError()
	{
		return errorcode;
	}
};
