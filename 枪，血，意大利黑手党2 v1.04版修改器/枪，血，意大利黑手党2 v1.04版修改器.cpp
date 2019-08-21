#include <iostream>
#include <windows.h>
#include <tlhelp32.h> 
#include <tchar.h> 
#include "atlbase.h"
#include "atlstr.h"
#include "ChectCode.h"
//#include "vector"
using namespace std;
//#pragma comment( linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" ) 
HANDLE hStdin;
HANDLE hOut;
COORD pos;//控制台相关

BOOL FindProcessPid(wchar_t * ProcessName, DWORD& dwPid);
bool test(char *p, char * q, int len);
int qtest(char *p, char *temp, int dlen, int len, int offsetStep);
DWORD findAddress(HANDLE hProcess, char *ipcode, int len, DWORD start, DWORD end, int offsetStep);
void GOTO(int x, int y);
void initCons();//初始化控制台和菜单
BOOL isUp(int a);//检查按键是否弹起
bool ActiveCode(ChectCode * obj);//激活作弊代码
bool run, start, close;//音乐相关
void Decode(char *p);
void StartTip();
void EndTip();
DWORD WINAPI Start(LPVOID lpParamter);
DWORD WINAPI Close(LPVOID lpParamter);

DWORD WINAPI Update(LPVOID lpParamter);//游戏状态监控线程
HANDLE hProcess=0;
wchar_t pname[20] = L"ggc2.exe";
DWORD pid;

//标志着游戏是否开启
bool gameisStart=false;

//属性注册处
ChectCode Shell;
ChectCode BShell;
ChectCode Hp;
ChectCode Killer;
//vector <ChectCode*> ChectList;//加入缓存表效率提升不大，体积增大2KB
int main()
{
	initCons();
	run = 1;
	char BShellobjcode[7] = { 0xFF ,0xC8 ,0x89 ,0x46 ,0x1C ,0x48 ,0x8B };
	char BShellActive[7]  = { 0x90 ,0x90 ,0x90 ,0x90, 0x90, 0x48 ,0x8B };
	BShell.init(
		7, 
		BShellobjcode,
		BShellActive,
		0x00410001,
		0x4FFFFFF1,
		0x10);

	
	char Shellobjcode[9] = { 0x48, 0x8B ,0xC2 ,0x89 ,0x47 ,0x24 ,0x48 ,0x8B ,0xCF };
	char ShellActive[9]  = { 0x90, 0x90, 0x90, 0x89 ,0x47 ,0x24 ,0x48 ,0x8B ,0xCF };
	Shell.init(
		9,
		Shellobjcode,
		ShellActive,
		0x00410000,
		0x4FFFFFF0,
		0x10);

	char Hpobjcode[11] = { 0xF3 ,0x0F ,0x11 ,0xAE ,0xC0 ,0x01 ,0x00 ,0x00 ,0x48 ,0x8B ,0xCE };
	char HpActive[11]  = { 0xC7 ,0x86 ,0xC0 ,0x01 ,0x00 ,0x00 ,0xFF ,0xFF ,0xFF ,0xFF ,0x90 };
	Hp.init(
		11,
		Hpobjcode,
		HpActive,
		0x0041000F,
		0x4FFFFFFF,
		0x10);
	//ChectList.push_back(&BShell);//加入缓存表
	char Killobjcode[11] = { 0xF3 ,0x0F ,0x11 ,0xAE ,0xC0 ,0x01 ,0x00 ,0x00 ,0x48 ,0x8B ,0xCE };
	char KillActive[11] = { 0xC7 ,0x86 ,0xC0 ,0x01 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x90 };
	Killer.init(
		11,
		Killobjcode,
		KillActive,
		0x0041000F,
		0x4FFFFFFF,
		0x10);
	while (true)
	{
		if (isUp(VK_F6)) {
			GOTO(39, 1);
			if (ActiveCode(&BShell)){
				start = true;
				SetConsoleTextAttribute(hOut,
					0x00A0);
				printf("开");
				SetConsoleTextAttribute(hOut,
					0x00F0);
			}
			else {
				SetConsoleTextAttribute(hOut,
					0x00FC);
				printf("关");
				close = true;
				SetConsoleTextAttribute(hOut,
					0x00F0);
			}
		}
		if (isUp(VK_F7)) {
			GOTO(39, 2);
			if (ActiveCode(&Shell)){
				start = true;
				SetConsoleTextAttribute(hOut,
					0x00A0);
				printf("开");
				SetConsoleTextAttribute(hOut,
					0x00F0);
			}
			else {
				close = true;
				SetConsoleTextAttribute(hOut,
					0x00FC);
				printf("关");
				SetConsoleTextAttribute(hOut,
					0x00F0);
			}
		}
		if (isUp(VK_F8)) {
			GOTO(39, 3);
			if (ActiveCode(&Hp)){
				start = true;
				SetConsoleTextAttribute(hOut,
					0x00A0);
				printf("开");
				SetConsoleTextAttribute(hOut,
					0x00F0);
			}
			else {
				close = true;
				SetConsoleTextAttribute(hOut,
					0x00FC);
				printf("关");
				SetConsoleTextAttribute(hOut,
					0x00F0);
			}
		}
		if (isUp(VK_F9)) {
			GOTO(39, 4);
			if (ActiveCode(&Killer)) {
				start = true;
				SetConsoleTextAttribute(hOut,
					0x00A0);
				printf("开");
				SetConsoleTextAttribute(hOut,
					0x00F0);
			}
			else {
				close = true;
				SetConsoleTextAttribute(hOut,
					0x00FC);
				printf("关");
				SetConsoleTextAttribute(hOut,
					0x00F0);
			}
		}
		if (isUp(VK_F12)) {
			break;
		}
		Sleep(140);
	}
	run = 0;//关闭音乐线程 return 之前赋值一下
	return 0;
}
BOOL isUp(int a) {
	return (GetAsyncKeyState(a) & 0xffff) && (GetAsyncKeyState(VK_CONTROL) & 0xffff);
}
bool ActiveCode(ChectCode * obj) {
	DWORD dwSize;
	if (!gameisStart) {
		obj->address = 0;//地址缓存过期
		obj->isActive = 0;//游戏关闭所以激活取消
		return gameisStart;//如何游戏没开启激活失败
	}
	DWORD address=0;
	if (obj->address != 0)address = obj->address;//查询到地址缓存
	else{
		address = findAddress(hProcess, obj->objcode, obj->len, obj->start, obj->end, obj->offset);
		if (address) {
			obj->address = address;//缓存
			obj->isActive = 0;//找到了原初代码
		}
		else {
			address = findAddress(hProcess, obj->activeCode, obj->len, obj->start, obj->end, obj->offset);
			obj->address = address;//缓存
			obj->isActive = 1;//找到了激活代码
		}
	}
	if (obj->isActive){
		if (!WriteProcessMemory(hProcess, (LPVOID)(address), obj->objcode, obj->len, &dwSize)){
			obj->isActive = false;//写入失败
			return false;
		}
		obj->isActive = false;//写入原初代码
		return false;
	}else{
		if (!WriteProcessMemory(hProcess, (LPVOID)(address), obj->activeCode, obj->len, &dwSize)) {
			obj->isActive = false;//写入失败
			return false;
		}
		obj->isActive = true;//写入了激活代码
		return true;
	}
	return true;
		
}
DWORD WINAPI Start(LPVOID lpParamter)
{
	while (run)
	{
		while (run && !start)
		{
			Sleep(100);
		}
		StartTip();
		start = false;
	}
	return 0L;
}

DWORD WINAPI Close(LPVOID lpParamter)
{
	while (run)
	{
		while (run && !close)
		{
			Sleep(100);
		}
		EndTip();
		close = false;
	}
	return 0L;
}
void GOTO(int x, int y) {
	pos.X = x;
	pos.Y = y;
	SetConsoleCursorPosition(hOut, pos);
}
void Decode(char *p) {
	for (int a = 0; p[a]; a++) {
		//p[a]= ((p[a] & 0x0f) << 4 | (p[a] & 0xf0) >> 4)&0xff;
		p[a]--;
	}
	/*
	char temp;
	for (int a = 0; p[a]&&p[a+1]; a+=2) {
		temp = p[a]&0xff;
		p[a] = p[a + 1] & 0xff;
		p[a + 1] = temp & 0xff;
	}
	*/
}
void initCons() {
	//AllocConsole();						//打开控制台窗口以显示调试信息
	ShowWindow(GetConsoleWindow(), SW_HIDE);
	//freopen("conout$", "w", stdout);

	system("color F0");
	char  p[80] = "\x48\x97\x48\xC7\x57\x03\x7D\x9C\x3B\xCB\x1E\xAB\x3B\xCB\x2E\x2F\x4C\x40\x0D\xC0\xAC\xAE\xAD\x6E\x5C\x3C\x24\x03\x68\x14\xE3\x04\x44\x0C\x6F\x0E\xEE\x8C\x4D\x6D\x80\x03\x27\x98\xED\x4E\x1D\xB0\x0D\xBD\xAD\xFC";
	for (int a = 0; p[a]; a++) {
		p[a]--;
		p[a]= ((p[a] & 0x0f) << 4 | (p[a] & 0xf0) >> 4)&0xff;
	}

	//printf("%s", p);
	system(p);
	printf("意大利黑手党2 v1.04版四项修改器\n");
	printf("后备弹药无限                   Ctrl+F6    开启条件:换个弹夹\n");
	printf("不用换弹夹                     Ctrl+F7    开启条件:开一枪  \n");
	printf("受到攻击的人将变成不死的幽灵   Ctrl+F8    开启条件:主角被攻击\n");
	printf("所有人受到伤害就会死主角也一样 Ctrl+F9    开启条件:主角被攻击\n");
	printf("Ctrl+F12退出\n\n");
	printf("听到滴的声音说明功能开启成功，嘟则说明功能关闭，或者开启失败。\n"\
		   "功能3和功能4只能开启一个，多开以最后一个算\n刚进游戏需要满足条件才能开启功能!\n");
	hStdin = GetStdHandle(STD_INPUT_HANDLE);
	DWORD mode;
	GetConsoleMode(hStdin, &mode);
	mode &= ~ENABLE_QUICK_EDIT_MODE;  //移除快速编辑模式
	mode &= ~ENABLE_INSERT_MODE;      //移除插入模式
	mode &= ~ENABLE_MOUSE_INPUT;
	SetConsoleMode(hStdin, mode);
	pos = { 0,0 };
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cci;
	SMALL_RECT rc;
	rc.Left = 0;
	rc.Top = 0;
	rc.Right = 69;
	rc.Bottom = 11;
	COORD size = { rc.Right , rc.Bottom };
	SetConsoleScreenBufferSize(hOut, size);
	SetConsoleWindowInfo(hOut, TRUE, &rc);
	GetConsoleCursorInfo(hOut, &cci);    //获取光标信息
	cci.dwSize = 1;    //设置光标大小
	cci.bVisible = 0; //设置光标不可见
	SetConsoleCursorInfo(hOut, &cci);    //设置(应用)光标信息
	ShowWindow(GetConsoleWindow(), SW_SHOW);
	//以上初始化显示

	//开启数据更新线程
	HANDLE hThread = CreateThread(NULL, 0, Update, NULL, 0, NULL);
	CloseHandle(hThread);

	//开启音乐线程
	hThread = CreateThread(NULL, 0, Start, NULL, 0, NULL);
	CloseHandle(hThread);
	hThread = CreateThread(NULL, 0, Close, NULL, 0, NULL);
	CloseHandle(hThread);
}
DWORD WINAPI Update(LPVOID lpParamter)
{
	bool t = true;
	while (run)
	{
		int line = 10;
		GOTO(0, line);
		SetConsoleTextAttribute(hOut,
			0x00FC);
		printf("等待游戏出现");
		SetConsoleTextAttribute(hOut,
			0x00F0);
		while (!FindProcessPid(pname, pid) && run) {//等待游戏出现
			Sleep(4000);
		}
		GOTO(0, line);
		SetConsoleTextAttribute(hOut,
			0x00FC);
		printf("游戏已经打开,开始初始化数据");
		SetConsoleTextAttribute(hOut,
			0x00F0);
		//初始化相关数据
		if (t)t = FindProcessPid(pname, pid);//获取游戏进程pid
		if (t)hProcess = OpenProcess(PROCESS_ALL_ACCESS, false, pid);
		if (t) {
			t = false;
			gameisStart = true;
			//printf("\n\n----%08X--%08X----\n\n", hProcess,baseAddress);
		}
		Sleep(1000);
		GOTO(0, line);
		SetConsoleTextAttribute(hOut,
			0x00F0);

		//缓存地址,对于先开游戏的情况，这样可以加速使用
		/*
		for (int t = 0; t < ChectList.size(); t++) {
			ChectList[t]->address = findAddress(hProcess, ChectList[t]->objcode, ChectList[t]->len, ChectList[t]->start, ChectList[t]->end, ChectList[t]->offset);
			if (!ChectList[t]->address)
				ChectList[t]->address = findAddress(hProcess, ChectList[t]->activeCode, ChectList[t]->len, ChectList[t]->start, ChectList[t]->end, ChectList[t]->offset);
		}
		*/

		printf("游戏始初始化完成                     ");
		if (!t&&run) {//监控游戏是否正常运行
			while (run&&FindProcessPid(pname, pid)) {//两秒钟重检查一次
				Sleep(2000);
			}
			GOTO(0, line);
			SetConsoleTextAttribute(hOut,
				0x00FC);
			printf("游戏关闭          ");
			SetConsoleTextAttribute(hOut,
				0x00F0);
			//监控游戏异常结束
			gameisStart = false;
			t = true;//重新初始化
		}
		Sleep(1000);
	}
	return 0L;
}

void StartTip() {
	Beep(1046, 100);
}

void EndTip() {
	//Beep(700, 50);
	//Beep(600, 50);
	//Beep(500, 50);
	Beep(400, 500);
}
bool test(char *p, char * q, int len) {
	for (int a = 0; a < len; a++)
	{
		if (p[a] != q[a])return false;
	}
	return true;
}
int qtest(char *p, char *temp, int dlen, int len, int offsetStep) {
	//if (offsetStep == 0x1);
	for (int offset = 0; offset < dlen; offset += offsetStep) {
		if (test(p, temp + offset, len))return offset;
	}
	return -1;
}
DWORD findAddress(HANDLE hProcess, char *ipcode, int len ,DWORD start, DWORD end,int offsetStep) {
	const unsigned cacheLen = 0x40000; //申请80KB的搜索缓存
	DWORD baseAddress = start, dwSize;
	char temp[cacheLen];
	while (true)
	{
		if (baseAddress > end) {
			return 0;
		}
		baseAddress += cacheLen;
		if (!ReadProcessMemory(hProcess, (LPVOID)(baseAddress), &temp, cacheLen, &dwSize)) {
			continue;
		}
		int t = qtest(ipcode, temp, cacheLen, len, offsetStep);
		if (t != -1) {
			baseAddress += t;
			break;
		}
	}
	return baseAddress;
}
BOOL FindProcessPid(wchar_t * ProcessName, DWORD& dwPid)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		return(FALSE);
	}
	pe32.dwSize = sizeof(PROCESSENTRY32);
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return(FALSE);
	}
	BOOL    bRet = FALSE;
	do
	{
		if (!wcscmp(ProcessName, pe32.szExeFile))
		{
			dwPid = pe32.th32ProcessID;
			bRet = TRUE;
			break;
		}

	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return bRet;
}