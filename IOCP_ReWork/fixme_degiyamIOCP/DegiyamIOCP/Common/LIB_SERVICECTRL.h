
#pragma once

#ifndef __LIB_SERVICECTRL_H__
#define __LIB_SERVICECTRL_H__

#include "Common.h"

namespace COMMONLIB
{
	/*
	 *	Make a person	: �����
	 *	Make a date		: 2006.10.07
	 *	Class name		: LIB_SERVICECTRL
	 *	Class describe	: ���� ��ġ, ����, ���, ����, ����, �������縦 �� �� �ִ� class
	*/

	// winsvc.h �� ���ǵ� ���� ���°��� ��������
	enum eServiceState
	{
		SERVICESTATE_STOPPED		= 0x00000001,	// SERVICE_STOPPED
		SERVICESTATE_STARTPENDING	= 0x00000002,	// SERVICE_START_PENDING
		SERVICESTATE_STOPPENDING	= 0x00000003,	// SERVICE_STOP_PENDING
		SERVICESTATE_RUNNING		= 0x00000004,	// SERVICE_RUNNING

		SERVICESTATE_SCMERROR		= 0x00000100,	// SCM Open fail
		SERVICESTATE_NOTINSTALL		= 0x00000101,	// Service not installed
		SERVICESTATE_ERROR			= 0x00000102,	// unknown error
	};

	class LIB_SERVICECTRL
	{
	public :
		static	SERVICE_STATUS_HANDLE	theServiceStatusHandle;
		static	SERVICE_STATUS			theServiceStatus;
	public :
		static	const	BOOL	InstallServer(const LPCTSTR strServiceName, const LPCTSTR strServiceDir);
		static	const	BOOL	UnInstallServer(const LPCTSTR strServiceName);
		static	const	BOOL	ServiceStart(const LPCTSTR szServiceName);
		static	const	BOOL	ServiceStop(const LPCTSTR szServiceName);
		static	const	BOOL	RegisterService(const LPCTSTR strServiceName, const LPHANDLER_FUNCTION lpHandlerProc);
		static	const	DWORD	ServiceQueryState(const LPCTSTR szServiceName);
		static	const	VOID	UpdateServiceStatus(const DWORD dwStatus);
	};
}

#endif	// __LIB_SERVICECTRL_H__