// loadlibrary.cpp : �������̨Ӧ�ó������ڵ㡣
//

#include "stdafx.h"
#include <windows.h>

DWORD WINAPI ThreadMain(LPVOID lpParameter)
{
	while (1)
	{
		printf("Hello!\n");
	}
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	DWORD	Tid;
	CreateThread(NULL,
		NULL,
		ThreadMain,
		NULL,
		0,
		&Tid);
	while (1)
	{
		printf("No\n");
	}
	return 0;
}
