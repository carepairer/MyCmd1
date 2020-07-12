#include "stdafx.h"
#include <windows.h>
#include <stdio.h>


int main(int argc, char* argv[])
{
	//定义管道句柄
	HANDLE CmdReadPipe;
	HANDLE CmdWritePipe;
	HANDLE MyReadPipe;
	HANDLE MyWritePipe;

	//创建管道
	SECURITY_ATTRIBUTES sa = { 0 };
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	CreatePipe(&CmdReadPipe, &MyWritePipe, &sa, 0);
	CreatePipe(&MyReadPipe, &CmdWritePipe, &sa, 0);

	//创建进程 替换Cmd的标准输入输出
	PROCESS_INFORMATION pi = { 0 };
	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESTDHANDLES; //	标志位，开启句柄替换的标志
	si.hStdInput = CmdReadPipe;
	si.hStdOutput = CmdWritePipe;
	si.hStdError = CmdWritePipe;

	CreateProcess(
		_T("C:\\Windows\\System32\\cmd.exe"), //被启动的程序路径
		NULL, //命令行参数
		NULL, //进程句柄继承属性
		NULL, //线程句柄继承属性
		TRUE, //继承开关
		CREATE_NO_WINDOW, //无窗口，od调试器DEBUF_PROCESS
		NULL, //环境变量
		NULL, //当前路径 程序双击时的路径（相对路径）
		&si, //传出参数
		&pi  //传出参数
	);

	char szOutbuf[256] = { 0 };

	while (TRUE)
	{    //读文件
		DWORD nReadBytes = 0;
		Sleep(300);
		while (TRUE)
		{
			//初始化为零
			memset(szOutbuf, 0, 256);
			PeekNamedPipe(MyReadPipe, szOutbuf, sizeof(szOutbuf - 1), &nReadBytes, NULL, NULL);  //立即返回，不会清除管道输出端数据
			if (!nReadBytes) break;                                                              //没有数据可读了

			if (!ReadFile(MyReadPipe, szOutbuf, sizeof(szOutbuf) - 1, &nReadBytes, NULL))
			{
				printf("read pipe error");
				return 0;
			}
			printf(szOutbuf);
		}

		//写文件
		char szCmdBuf[256] = { 0 };
		gets_s(szCmdBuf);
		DWORD CmdBufLength = strlen(szCmdBuf);
		if (CmdBufLength > 255)
		{
			printf("stack overflow");
			return 0;
		}
		szCmdBuf[CmdBufLength++] = '\r'; //回车
		szCmdBuf[CmdBufLength++] = '\n'; //换行
		DWORD writeflag = 0;
		WriteFile(MyWritePipe, szCmdBuf, CmdBufLength, &writeflag, NULL);
	}

	printf("exit");
	//relese resource
	CloseHandle(MyWritePipe);
	CloseHandle(MyReadPipe);
	CloseHandle(CmdWritePipe);
	CloseHandle(CmdReadPipe);

	return 0;

}
