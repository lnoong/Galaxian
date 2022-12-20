#include "stdafx.h"
#include "Tank.h"
#include"Resource.h"
#include <iostream>
#include <time.h>
#include<windows.h>
#include<mmsystem.h> //windows中与多媒体有关的大多数接口

#pragma comment(lib,"WINMM.LIB")
#define MAX_LOADSTRING 100

HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

HBITMAP		hbmp_object;
BITMAP		bmp_object;
HDC			hdc_object;
SIZE		sz_scr = { GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) };//获取屏幕大小
RECT        rectDlg;


//int hpipe[2];

enum Dir{UP, DOWN, LEFT, RIGHT};	// 枚举类型，运动物体可能的运动方向
typedef struct				// 结构体类型，游戏对象结构体
{
	int x, y;				// 位置
	Dir dir;				// 方向
	int v;					// 速率
	int s;					// 边长，正方形
	int b;					// 是否是子弹
	int f;					// 是否是玩家
	int g;                  // 是否是能量石
	int p;					// 是否停止,只有玩家才可能停止
	int e;					// 是否是敌人
	COLORREF c;				// 颜色
}Entity;

#define MAX_ENEMY 18		// 最大敌人数目
#define MAX_BULLETS 32		// 最大子弹数目
#define MAX_NLS 30           // 最大能量石数目
int nLife = 3;				// 玩家生命
int nScore = 0;				// 玩家得分
int nBullet = 0;			// 玩家打出的子弹数量
int nEnemyBullet = 0;		// 敌人发出的子弹数量
int nnls = 0;               // 掉落能量石的数量
int nEnemy = 0;				// 当前的敌人数量
int timeStep = 20;			// 定时器时间间隔
int sz = 50;				// 坦克尺寸
int velf = 4;				// 快速坦克速率
int vels = 2;				// 慢速坦克速率
int szb = 20;				// 子弹原尺寸
int velb = 6;				// 子弹速率
int enemyFirePer = 300;		// 敌人发射子弹的随机比例
int enemyDir = 200;			// 敌人改变方向的随机比例
int nlssj = 100;            // 能量石出现的随机比例
int m = 0;                  // 是否吸收能量石升级
int bFire = 0;				// 玩家是否处于射击状态
Entity enemys[MAX_ENEMY];	// 敌人数组
Entity nls[MAX_NLS];        // 能量石数组
Entity player;				// 玩家
Entity bullets[MAX_BULLETS];// 玩家子弹数组
Entity enemyBullets[MAX_BULLETS];// 敌人子弹数组
int wndWidth = 0;	int wndHeight = 0;  // 窗口尺寸
	
// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

// 重置玩家信息,恢复到初始状态
void ResetPlayer()
{
	player.s = sz;
	player.b = 0;
	player.c = RGB(122,30,0);
	player.dir = UP;
	player.v = vels;
	player.x = wndWidth/2;
	player.y = wndHeight-sz;
	player.p = 1;
	player.e = 0;
	player.f = 1;
	m = 0;
}

// 游戏初始化
void Init()
{
	int line = 3;
	nEnemy = 0;
	for (int i = 0; i < line; i++) {
		int count = 0;
		for (; count < MAX_ENEMY / line; nEnemy++, count++) {
			enemys[nEnemy].s = sz;
			enemys[nEnemy].b = 0;
			enemys[nEnemy].e = 1;
			enemys[nEnemy].dir = RIGHT;//Dir(UP+rand()%4);		// 随机选择一个前进方向
			enemys[nEnemy].v = vels;//rand()%2==0?velf:vels;	// 在两种速度之间以50%的几率随机选择一种
			enemys[nEnemy].c = i % 2 == 0 ? RGB(0, 122, 122) : RGB(0, 60, 30);//enemys[nEnemy].v == velf?RGB(0,122,122):RGB(0,60,30);	// 两种速度坦克的颜色不一样
			enemys[nEnemy].x = (wndWidth - sz) / 2 - sz * MAX_ENEMY / line + 2 * count * sz;//(rand()%3)*(wndWidth-sz)/2 + sz/2;	// 随机出生点
			enemys[nEnemy].y = sz / 2 + 2 * i * sz;//sz;
			enemys[nEnemy].p = 0;
		}
	}

	ResetPlayer();
}

// 对特定游戏实体依据朝向和速率进行移动
void Move(Entity *ent, int ts)
{
	if (ent->p)
		return;
	switch(ent->dir)
	{
	case UP:
		ent->y -= ent->v * ts;
		break;
	case DOWN:
		ent->y += ent->v * ts;
		break;
	case LEFT:
		ent->x -= ent->v * ts;
		break;
	case RIGHT:
		ent->x += ent->v * ts;
		break;
	}
}
void ChangeEnemyDir(Entity* ents)
{
	for (int i = 0; i < nEnemy; i++)
		ents[i].dir = ents[i].dir == LEFT ? RIGHT : LEFT;
}

// 特定实体进行射击
void Fire(const Entity* ent)
{
	Entity *pBulletes = (ent->e)?enemyBullets:bullets;	// 判断是敌人还是玩家发射炮弹
	int nB = (ent->e)?nEnemyBullet:nBullet;			
	if (nB >= MAX_BULLETS)
		return;
	if (!(ent->e) && m == 1)
	{
		(pBulletes + nB)->s = szb*2;
	
	}
	else
	{
		(pBulletes + nB)->s = szb;
	}
	(pBulletes+nB)->b = 1;
	(pBulletes+nB)->e = 0;
	(pBulletes+nB)->c = (ent->e)?RGB(0,0,255):RGB(255,0,0);	// 敌人发出炮弹和玩家发出的不一样
	(pBulletes + nB)->dir = (ent->e) ? DOWN : UP;//ent->dir;
	(pBulletes+nB)->v = velb;
	(pBulletes+nB)->x = ent->x;
	(pBulletes+nB)->p = 0;
	(pBulletes+nB)->y = ent->y;
	switch(/*ent->dir*/ (pBulletes+nB)->dir)	// 按照炮弹的方向来调整位置
	{
	case UP:
		(pBulletes+nB)->y -= ent->s;
		break;
	case DOWN:
		(pBulletes+nB)->y += ent->s;
		break;
	case LEFT:
		(pBulletes+nB)->x -= ent->s;
		break;
	case RIGHT:
		(pBulletes+nB)->x += ent->s;
		break;
	}
	if (ent->e)
		nEnemyBullet++;
	else
		nBullet++;
}
// 将特定的实体从数组中删除,后续元素向前移动
void Destroy(Entity ents[], int n, int *num)
{
	memcpy(ents+n, ents+n+1, sizeof(Entity)*((*num)-1-n));
	(*num)--;
}

// 判断两个实体是否发生碰撞,以正方形之间发生碰撞来判断
int IsCollide(const Entity *ent1, const Entity *ent2)
{

	if (ent1->x+ent1->s/2 <= ent2->x-ent2->s/2 || ent1->x-ent1->s/2 >= ent2->x + ent2->s/2)
		return 0;
	if (ent1->y+ent1->s/2 <= ent2->y-ent2->s/2 || ent1->y-ent1->s/2 >= ent2->y + ent2->s/2)
		return 0;
	return 1;
}

// 判断特定实体是否和边界发生碰撞
int WallCollide(Entity *ent)
{
	int bC = 0;
	switch(ent->dir)
	{
	case UP:
		if ((ent->y - ent->s/2) < 0)			// 上边界
		{
			bC = 1;
			ent->y = ent->s/2;
		}
		break;
	case DOWN:
		if ((ent->y + ent->s/2) > wndHeight)	// 下边界
		{
			bC = 1;
			ent->y = wndHeight - ent->s/2;
		}
		break;
	case LEFT:
		if((ent->x - ent->s/2) < 0)				// 左边界
		{
			bC = 1;
			ent->x = ent->s/2;
		}
		break;
	case RIGHT:
		if((ent->x + ent->s/2) > wndWidth)		// 右边界
		{
			bC = 1;
			ent->x = wndWidth - ent->s/2;
		}
		break;
	}
	if (bC)
	{
		if (!ent->e)
			ent->p = 1;
	}
	return bC;
}

// 更新各种游戏信息,定时器消息中会调用这个函数
int dif = 1;
void Update(int ts)
{	
	// 可移动物体位置进行更新
	Entity* ent = NULL;
	for (int i = 0; i < nEnemy; i++)	// 敌人位置更新
	{
		ent = enemys+i;
		Move(ent, ts);
		if ((rand()%enemyFirePer) == 0)
			Fire(ent);
	}
	for (int i = 0; i < nBullet; i++)	// 玩家炮弹位置进行更新
	{
		ent = bullets+i;
		Move(ent, ts);
	}
	for (int i = 0; i < nEnemyBullet; i++)	// 敌人炮弹位置进行更新
	{
		ent = enemyBullets+i;
		Move(ent, ts);
	}
	for (int i = 0; i < nnls; i++)	// 能量石位置进行更新
	{
		ent = nls + i;
		Move(ent, ts);
	}
	Move(&player, ts);					// 玩家位置进行更新

	if (bFire)							// 如果玩家处于射击状态,则发射炮弹
	{
		Fire(&player);
		PlaySound(L"shot.wav", NULL, SND_FILENAME | SND_ASYNC);
		bFire = 0;
	}

	// 判断子弹是否和敌人碰撞
	for(int i = 0; i < nBullet; i++)
	{
		for (int j = 0; j < nEnemy; j++)
		{
			if (IsCollide(&bullets[i], &enemys[j]))
			{
				m = 0;
				Destroy(bullets, i, &nBullet);
				PlaySound(L"boom.wav", NULL, SND_FILENAME | SND_ASYNC);
				Destroy(enemys, j, &nEnemy);
				if (rand()%2==0) //随机生成能量石 
				{
					nnls++;
					for (i = 0; i < MAX_NLS; i++)
					{
						nls[i].s = 50;
						nls[i].b = 0;
						nls[i].g = 1;
						nls[i].dir = DOWN;		// 前进方向为下落
						nls[i].v = vels + 2;	    // 速度
						nls[i].c = RGB(88, 177, 122);// 颜色
						nls[i].x = enemys[j].x;//出现点
						nls[i].y = enemys[j].y;//
						nls[i].p = 0;
					}
					PlaySound(L"stone.wav", NULL, SND_FILENAME | SND_ASYNC);
				}
				nScore++;
				i--;
				j--;
				break;
			}
		}
	}
	// 判断敌人的子弹是否和玩家碰撞
	for(int i = 0; i < nEnemyBullet; i++)
	{
		if (IsCollide(&enemyBullets[i], &player))
		{	
			Destroy(enemyBullets, i, &nEnemyBullet);
			PlaySound(L"boom.wav", NULL, SND_FILENAME | SND_ASYNC);
			ResetPlayer();
			nLife--;
			i--;
			break;
		}
	}
	// 判断能量石是否和玩家碰撞
	for (int i = 0; i < nnls; i++)
	{
		if (IsCollide(&nls[i], &player))
		{	
			PlaySound(L"up.wav", NULL, SND_FILENAME | SND_ASYNC);
			Destroy(nls, i, &nnls);
			m = 1;
		}
	}
	// 判断敌人是否和玩家碰撞
	for (int i = 0; i < nEnemy; i++)
	{
		if (IsCollide(&player, &enemys[i]))
		{
			PlaySound(L"boom.wav", NULL, SND_FILENAME | SND_ASYNC);
			ResetPlayer();
			nLife--;
		}
	}
	// 判断各种实体是否和游戏边界发生碰撞
	if (dif%2== 0)
	{
		for (int i = 0; i < nEnemy; i++)	// 敌人
		{
			ent = enemys + i;
			if (!WallCollide(ent))		// 有一定几率改变方向
			{
				if (rand() % enemyDir == 0)
					ent->dir = Dir((ent->dir + 1 + rand() % 3) % 4);
			}
		}
	}
	for (int i = 0; i < nBullet; i++)	// 玩家炮弹
	{
		ent = bullets+i;
		if (WallCollide(ent))
		{
			Destroy(bullets, i, &nBullet);
			i--;
		}
	}
	for (int i = 0; i < nEnemyBullet; i++)	// 敌人炮弹
	{
		ent = enemyBullets+i;
		if (WallCollide(ent))
		{	
			Destroy(enemyBullets, i, &nEnemyBullet);
			i--;
		}
	}
	for (int i = 0; i < nnls; i++)	// 能量石
	{
		ent = nls + i;
		if (WallCollide(ent))
		{
			Destroy(nls, i, &nnls);
			i--;
		}
	}
	WallCollide(&player);			// 玩家
}
// 绘制参数指定的游戏实体
void DrawEntity(HDC hdc, const Entity *ent)
{
	HBRUSH brush;
	brush = CreateSolidBrush(ent->c);		// 按照实体指定的颜色创建笔刷
	RECT rc;								// 实体长方形
	rc.top = ent->y-ent->s/2;
	rc.left = ent->x-ent->s/2;
	rc.bottom = ent->y+ent->s/2;
	rc.right = ent->x+ent->s/2;
	StretchBlt(hdc, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, hdc_object, 1, 1, 16, 16, SRCCOPY);
	if (!ent->b)							// 如果这个实体不是子弹,则依据角色类型绘制炮筒
	{
		if (ent->e)
		{
			rc.top = rc.bottom;
			rc.bottom = rc.bottom + ent->s / 2;
			rc.left = rc.left + ent->s / 4;
			rc.right = rc.right - ent->s / 4;
		}
		else
		{
			rc.bottom = rc.top;
			rc.top = rc.bottom - ent->s / 2;
			rc.left = rc.left + ent->s / 4;
			rc.right = rc.right - ent->s / 4;
		}
	}
}
void Drawplayer(HDC hdc, const Entity* ent)
{
	HBRUSH brush;
	brush = CreateSolidBrush(ent->c);		// 按照实体指定的颜色创建笔刷
	RECT rc;								// 实体长方形
	rc.top = ent->y - ent->s / 2;
	rc.left = ent->x - ent->s / 2;
	rc.bottom = ent->y + ent->s / 2;
	rc.right = ent->x + ent->s / 2;
	StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc_object, 1, 70, 16, 16, SRCCOPY);
	if (!ent->b)							// 如果这个实体不是子弹,则依据角色类型绘制炮筒
	{
		if (ent->e)
		{
			rc.top = rc.bottom;
			rc.bottom = rc.bottom + ent->s / 2;
			rc.left = rc.left + ent->s / 4;
			rc.right = rc.right - ent->s / 4;
		}
		else
		{
			rc.bottom = rc.top;
			rc.top = rc.bottom - ent->s / 2;
			rc.left = rc.left + ent->s / 4;
			rc.right = rc.right - ent->s / 4;
		}

	}
}
void Drawplayerbullent(HDC hdc, const Entity* ent)
{
	HBRUSH brush;
	brush = CreateSolidBrush(ent->c);		// 按照实体指定的颜色创建笔刷
	RECT rc;								// 实体长方形
	rc.top = ent->y - ent->s / 2;
	rc.left = ent->x - ent->s / 2;
	rc.bottom = ent->y + ent->s / 2;
	rc.right = ent->x + ent->s / 2;
	StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc_object, 65, 196, 3, 3, SRCCOPY);
	if (!ent->b)							// 如果这个实体不是子弹,则依据角色类型绘制炮筒
	{
		if (ent->e)
		{
			rc.top = rc.bottom;
			rc.bottom = rc.bottom + ent->s / 2;
			rc.left = rc.left + ent->s / 4;
			rc.right = rc.right - ent->s / 4;
		}
		else
		{
			rc.bottom = rc.top;
			rc.top = rc.bottom - ent->s / 2;
			rc.left = rc.left + ent->s / 4;
			rc.right = rc.right - ent->s / 4;
		}
	}
}
void Drawenemybullent(HDC hdc, const Entity* ent)
{
	HBRUSH brush;
	brush = CreateSolidBrush(ent->c);		// 按照实体指定的颜色创建笔刷
	RECT rc;								// 实体长方形
	rc.top = ent->y - ent->s / 2;
	rc.left = ent->x - ent->s / 2;
	rc.bottom = ent->y + ent->s / 2;
	rc.right = ent->x + ent->s / 2;
	StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc_object, 138, 196, 3, 3, SRCCOPY);
	if (!ent->b)							// 如果这个实体不是子弹,则依据角色类型绘制炮筒
	{
		if (ent->e)
		{
			rc.top = rc.bottom;
			rc.bottom = rc.bottom + ent->s / 2;
			rc.left = rc.left + ent->s / 4;
			rc.right = rc.right - ent->s / 4;
		}
		else
		{
			rc.bottom = rc.top;
			rc.top = rc.bottom - ent->s / 2;
			rc.left = rc.left + ent->s / 4;
			rc.right = rc.right - ent->s / 4;
		}
	}
}
void Drawstone(HDC hdc, const Entity* ent)
{
	HBRUSH brush;
	brush = CreateSolidBrush(ent->c);		// 按照实体指定的颜色创建笔刷
	RECT rc;								// 实体长方形
	rc.top = ent->y - ent->s / 2;
	rc.left = ent->x - ent->s / 2;
	rc.bottom = ent->y + ent->s / 2;
	rc.right = ent->x + ent->s / 2;
	StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc_object, 202, 175, 16, 16, SRCCOPY);
	if (!ent->b)							// 如果这个实体不是子弹,则依据角色类型绘制炮筒
	{
		if (ent->e)
		{
			rc.top = rc.bottom;
			rc.bottom = rc.bottom + ent->s / 2;
			rc.left = rc.left + ent->s / 4;
			rc.right = rc.right - ent->s / 4;
		}
		else
		{
			rc.bottom = rc.top;
			rc.top = rc.bottom - ent->s / 2;
			rc.left = rc.left + ent->s / 4;
			rc.right = rc.right - ent->s / 4;
		}
	}
}
// 绘制整个游戏场景,在其中调用各自的绘制函数完整绘制
void DrawScene(HDC hdc)
{
	// 绘制游戏提示信息
	HFONT hf;
	WCHAR str[32];
	long lfHeight;
	lfHeight = -MulDiv(16, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	hf = CreateFont(lfHeight, 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
	HFONT hfOld = (HFONT)SelectObject(hdc, hf);
	if (nLife <= 0)							// 如果玩家生命为0,则显示结束画面
	{
		SetTextColor(hdc, RGB(122, 0, 0));
		TextOut(hdc, wndWidth/2-100, wndHeight/2-40,L"Game Over",9);
		SelectObject(hdc, hfOld);
		mciSendString(L"close background.wav", NULL, 0, NULL);
		mciSendString(L"play gameover.wav", NULL, 0, NULL);
		return;
	}
	if (nScore >= 18)						// 如果玩家得分18,则显示下一关
	{
		SetTextColor(hdc, RGB(122, 0, 0));
		TextOut(hdc, wndWidth / 2 - 100, wndHeight / 2 - 40, L"Next Level", 9);
		SelectObject(hdc, hfOld);
		mciSendString(L"close background.wav", NULL, 0, NULL);
		mciSendString(L"play next.wav", NULL, 0, NULL);
		return;
	}
	// 显示游戏统计信息
	SetTextColor(hdc, RGB(100, 100, 100));
	wsprintf(str, L"Life:%d Score:%d", nLife, nScore);	
	TextOut(hdc, 0, 0,str,wcslen(str));
	SelectObject(hdc, hfOld);				// 恢复默认字体
	DeleteObject(hf);

	// 绘制各种游戏实体
	Entity* ent = NULL;
	for (int i = 0; i < nEnemy; i++)		// 敌人
	{
		ent = enemys+i;
		DrawEntity(hdc, ent);
	}
	for (int i = 0; i < nBullet; i++)		// 玩家发出的炮弹
	{
		ent = bullets+i;
		Drawplayerbullent(hdc, ent);
	}
	for (int i = 0; i < nEnemyBullet; i++)	// 敌人发出的炮弹
	{
		ent = enemyBullets+i;
		Drawenemybullent(hdc, ent);
	}
	for (int i = 0; i < nnls; i++)	// 能量石
	{
		ent = nls + i;
		Drawstone(hdc, ent);
	}

	Drawplayer(hdc, &player);				// 玩家
}
//DWORD WINAPI BE(LPVOID DO)//多线程调用函数
//{
//	if (DO == (LPVOID)1)
//	{
//		PlaySound(LPWSTR(IDR_BACKGROUND), NULL, SND_RESOURCE | SND_ASYNC);
//	}
//	return 0;
//
//}



int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TANK, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}
	
	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TANK));
	//创建新线程
	//HANDLE hBE=CreateThread(NULL, 0, BE,(LPVOID)1 , 0, NULL); 
	srand(time(NULL));		// 生成随机种子
	Init();					// 游戏信息初始化

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TANK));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_TANK);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; 

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

   mciSendString(L"play background.wav", NULL, 0, NULL);
	
   if (!hWnd)
   {
      return FALSE;
   }
   hbmp_object = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_OBJECT));
   GetObject(hbmp_object, sizeof(bmp_object), &bmp_object);
   HDC dc = GetDC(hWnd);
   hdc_object = CreateCompatibleDC(dc);
   SelectObject(hdc_object, hbmp_object);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_CREATE:			// 程序启动后,开始设置一个定时器
		SetTimer(hWnd,1,timeStep,NULL);
		SetTimer(hWnd, 2, 1000, NULL);	// 用于改变敌人运动方向的定时器
		break;
	case WM_TIMER:			// 定时器响应
		InvalidateRect(hWnd, NULL, TRUE);	// 让窗口变为无效,从而触发重绘消息
		if (wParam == 1)	// 对游戏进行更新
		{
			if (nLife > 0)
				Update(timeStep / 10);
		}
		else if (wParam == 2)
			ChangeEnemyDir(enemys);
		break;
	case WM_SIZE:			// 获取窗口的尺寸
		wndWidth = LOWORD(lParam);
		wndHeight = HIWORD(lParam);
		break;
	case WM_KEYDOWN:		// 玩家按下键盘按键
		{
			InvalidateRect(hWnd, NULL, TRUE);
			switch (wParam)	// 依据玩家输入的信息调整玩家控制的坦克状态
			{
			case VK_LEFT:
				player.dir = LEFT;
				player.p = 0;
				break;
			case VK_RIGHT:
				player.dir = RIGHT;
				player.p = 0;
				break;
			case VK_UP:
				player.dir = UP;
				player.p = 0;
				break;
			case VK_DOWN:
				player.dir = DOWN;
				player.p = 0;
				break;
			case VK_TAB:
				dif++;
				break;
			case VK_SPACE:	// 射击
				bFire = 1;;
				break;
			}
			break;
		}
	case WM_KEYUP:
		InvalidateRect(hWnd, NULL, TRUE);
		switch (wParam) {
		case VK_LEFT:
		case VK_RIGHT:
			player.p = 1;
			break;
		}
		break;
	case WM_ERASEBKGND:		// 不擦除背景,避免闪烁
		break;
	case WM_PAINT:
		{	
			hdc = BeginPaint(hWnd, &ps);
			HDC memHDC = CreateCompatibleDC(hdc);
			RECT rectClient;
			GetClientRect(hWnd, &rectClient);

			//创建位图
			HBITMAP bmpBuff = CreateCompatibleBitmap(hdc,wndWidth,wndHeight);
			HBITMAP pOldBMP = (HBITMAP)SelectObject(memHDC, bmpBuff);
			PatBlt(memHDC,0,0,wndWidth,wndHeight, BLACKNESS);	// 设置背景为黑色

			// 进行真正的绘制
			DrawScene(memHDC);

			//拷贝内存HDC内容到实际HDC
			BOOL tt = BitBlt(hdc, rectClient.left, rectClient.top, wndWidth,
				wndHeight, memHDC, rectClient.left, rectClient.top, SRCCOPY);

			//内存回收
			SelectObject(memHDC, pOldBMP);
			DeleteObject(bmpBuff);
			DeleteDC(memHDC);

			EndPaint(hWnd, &ps);
			break;
		}
	case WM_DESTROY:
		KillTimer(hWnd,1);		// 程序退出时，将定时器删除
		KillTimer(hWnd, 2);
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}