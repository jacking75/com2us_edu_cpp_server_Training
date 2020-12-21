#pragma once
#include "ErrorCode.h"
#include <string>
#include <basetsd.h>

namespace OmokServerLib
{
	const int KEY_MAX_SIZE = 1024;
	const int VALUE_MAX_SIZE = 4096;
	const int REQUEST_MAX_SIZE = 4096;
	const int RESULT_MAX_SIZE = 4096;
	const int REDIS_TASK_ID_MAX = 100;

	//TODO �����
	// ���̹� �ϰ����� �����ϴ�. ù ������ �빮�ڷ� �������� �ʳ׿�
	// -> �ذ�
	struct CommandRequest
	{
		int sessionIndex = -1;
		int redisTaskID = 0;
		char* commandBody = nullptr;
	};

	enum class RedisTaskID : short
	{
		confirmLogin = 1

	};
}