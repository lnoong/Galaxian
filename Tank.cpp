#include "stdafx.h"
#include "Tank.h"
#include"Resource.h"
#include <iostream>
#include <time.h>
#include<windows.h>
#include<mmsystem.h> //windows�����ý���йصĴ�����ӿ�

#pragma comment(lib,"WINMM.LIB")
#define MAX_LOADSTRING 100

HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name

HBITMAP		hbmp_object;
BITMAP		bmp_object;
HDC			hdc_object;
SIZE		sz_scr = { GetSystemMetrics(SM_CXSCREEN),GetSystemMetrics(SM_CYSCREEN) };//��ȡ��Ļ��С
RECT        rectDlg;


//int hpipe[2];

enum Dir{UP, DOWN, LEFT, RIGHT};	// ö�����ͣ��˶�������ܵ��˶�����
typedef struct				// �ṹ�����ͣ���Ϸ����ṹ��
{
	int x, y;				// λ��
	Dir dir;				// ����
	int v;					// ����
	int s;					// �߳���������
	int b;					// �Ƿ����ӵ�
	int f;					// �Ƿ������
	int g;                  // �Ƿ�������ʯ
	int p;					// �Ƿ�ֹͣ,ֻ����Ҳſ���ֹͣ
	int e;					// �Ƿ��ǵ���
	COLORREF c;				// ��ɫ
}Entity;

#define MAX_ENEMY 18		// ��������Ŀ
#define MAX_BULLETS 32		// ����ӵ���Ŀ
#define MAX_NLS 30           // �������ʯ��Ŀ
int nLife = 3;				// �������
int nScore = 0;				// ��ҵ÷�
int nBullet = 0;			// ��Ҵ�����ӵ�����
int nEnemyBullet = 0;		// ���˷������ӵ�����
int nnls = 0;               // ��������ʯ������
int nEnemy = 0;				// ��ǰ�ĵ�������
int timeStep = 20;			// ��ʱ��ʱ����
int sz = 50;				// ̹�˳ߴ�
int velf = 4;				// ����̹������
int vels = 2;				// ����̹������
int szb = 20;				// �ӵ�ԭ�ߴ�
int velb = 6;				// �ӵ�����
int enemyFirePer = 300;		// ���˷����ӵ����������
int enemyDir = 200;			// ���˸ı䷽����������
int nlssj = 100;            // ����ʯ���ֵ��������
int m = 0;                  // �Ƿ���������ʯ����
int bFire = 0;				// ����Ƿ������״̬
Entity enemys[MAX_ENEMY];	// ��������
Entity nls[MAX_NLS];        // ����ʯ����
Entity player;				// ���
Entity bullets[MAX_BULLETS];// ����ӵ�����
Entity enemyBullets[MAX_BULLETS];// �����ӵ�����
int wndWidth = 0;	int wndHeight = 0;  // ���ڳߴ�
	
// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);

// ���������Ϣ,�ָ�����ʼ״̬
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

// ��Ϸ��ʼ��
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
			enemys[nEnemy].dir = RIGHT;//Dir(UP+rand()%4);		// ���ѡ��һ��ǰ������
			enemys[nEnemy].v = vels;//rand()%2==0?velf:vels;	// �������ٶ�֮����50%�ļ������ѡ��һ��
			enemys[nEnemy].c = i % 2 == 0 ? RGB(0, 122, 122) : RGB(0, 60, 30);//enemys[nEnemy].v == velf?RGB(0,122,122):RGB(0,60,30);	// �����ٶ�̹�˵���ɫ��һ��
			enemys[nEnemy].x = (wndWidth - sz) / 2 - sz * MAX_ENEMY / line + 2 * count * sz;//(rand()%3)*(wndWidth-sz)/2 + sz/2;	// ���������
			enemys[nEnemy].y = sz / 2 + 2 * i * sz;//sz;
			enemys[nEnemy].p = 0;
		}
	}

	ResetPlayer();
}

// ���ض���Ϸʵ�����ݳ�������ʽ����ƶ�
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

// �ض�ʵ��������
void Fire(const Entity* ent)
{
	Entity *pBulletes = (ent->e)?enemyBullets:bullets;	// �ж��ǵ��˻�����ҷ����ڵ�
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
	(pBulletes+nB)->c = (ent->e)?RGB(0,0,255):RGB(255,0,0);	// ���˷����ڵ�����ҷ����Ĳ�һ��
	(pBulletes + nB)->dir = (ent->e) ? DOWN : UP;//ent->dir;
	(pBulletes+nB)->v = velb;
	(pBulletes+nB)->x = ent->x;
	(pBulletes+nB)->p = 0;
	(pBulletes+nB)->y = ent->y;
	switch(/*ent->dir*/ (pBulletes+nB)->dir)	// �����ڵ��ķ���������λ��
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
// ���ض���ʵ���������ɾ��,����Ԫ����ǰ�ƶ�
void Destroy(Entity ents[], int n, int *num)
{
	memcpy(ents+n, ents+n+1, sizeof(Entity)*((*num)-1-n));
	(*num)--;
}

// �ж�����ʵ���Ƿ�����ײ,��������֮�䷢����ײ���ж�
int IsCollide(const Entity *ent1, const Entity *ent2)
{

	if (ent1->x+ent1->s/2 <= ent2->x-ent2->s/2 || ent1->x-ent1->s/2 >= ent2->x + ent2->s/2)
		return 0;
	if (ent1->y+ent1->s/2 <= ent2->y-ent2->s/2 || ent1->y-ent1->s/2 >= ent2->y + ent2->s/2)
		return 0;
	return 1;
}

// �ж��ض�ʵ���Ƿ�ͱ߽緢����ײ
int WallCollide(Entity *ent)
{
	int bC = 0;
	switch(ent->dir)
	{
	case UP:
		if ((ent->y - ent->s/2) < 0)			// �ϱ߽�
		{
			bC = 1;
			ent->y = ent->s/2;
		}
		break;
	case DOWN:
		if ((ent->y + ent->s/2) > wndHeight)	// �±߽�
		{
			bC = 1;
			ent->y = wndHeight - ent->s/2;
		}
		break;
	case LEFT:
		if((ent->x - ent->s/2) < 0)				// ��߽�
		{
			bC = 1;
			ent->x = ent->s/2;
		}
		break;
	case RIGHT:
		if((ent->x + ent->s/2) > wndWidth)		// �ұ߽�
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

// ���¸�����Ϸ��Ϣ,��ʱ����Ϣ�л�����������
int dif = 1;
void Update(int ts)
{	
	// ���ƶ�����λ�ý��и���
	Entity* ent = NULL;
	for (int i = 0; i < nEnemy; i++)	// ����λ�ø���
	{
		ent = enemys+i;
		Move(ent, ts);
		if ((rand()%enemyFirePer) == 0)
			Fire(ent);
	}
	for (int i = 0; i < nBullet; i++)	// ����ڵ�λ�ý��и���
	{
		ent = bullets+i;
		Move(ent, ts);
	}
	for (int i = 0; i < nEnemyBullet; i++)	// �����ڵ�λ�ý��и���
	{
		ent = enemyBullets+i;
		Move(ent, ts);
	}
	for (int i = 0; i < nnls; i++)	// ����ʯλ�ý��и���
	{
		ent = nls + i;
		Move(ent, ts);
	}
	Move(&player, ts);					// ���λ�ý��и���

	if (bFire)							// �����Ҵ������״̬,�����ڵ�
	{
		Fire(&player);
		PlaySound(L"shot.wav", NULL, SND_FILENAME | SND_ASYNC);
		bFire = 0;
	}

	// �ж��ӵ��Ƿ�͵�����ײ
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
				if (rand()%2==0) //�����������ʯ 
				{
					nnls++;
					for (i = 0; i < MAX_NLS; i++)
					{
						nls[i].s = 50;
						nls[i].b = 0;
						nls[i].g = 1;
						nls[i].dir = DOWN;		// ǰ������Ϊ����
						nls[i].v = vels + 2;	    // �ٶ�
						nls[i].c = RGB(88, 177, 122);// ��ɫ
						nls[i].x = enemys[j].x;//���ֵ�
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
	// �жϵ��˵��ӵ��Ƿ�������ײ
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
	// �ж�����ʯ�Ƿ�������ײ
	for (int i = 0; i < nnls; i++)
	{
		if (IsCollide(&nls[i], &player))
		{	
			PlaySound(L"up.wav", NULL, SND_FILENAME | SND_ASYNC);
			Destroy(nls, i, &nnls);
			m = 1;
		}
	}
	// �жϵ����Ƿ�������ײ
	for (int i = 0; i < nEnemy; i++)
	{
		if (IsCollide(&player, &enemys[i]))
		{
			PlaySound(L"boom.wav", NULL, SND_FILENAME | SND_ASYNC);
			ResetPlayer();
			nLife--;
		}
	}
	// �жϸ���ʵ���Ƿ����Ϸ�߽緢����ײ
	if (dif%2== 0)
	{
		for (int i = 0; i < nEnemy; i++)	// ����
		{
			ent = enemys + i;
			if (!WallCollide(ent))		// ��һ�����ʸı䷽��
			{
				if (rand() % enemyDir == 0)
					ent->dir = Dir((ent->dir + 1 + rand() % 3) % 4);
			}
		}
	}
	for (int i = 0; i < nBullet; i++)	// ����ڵ�
	{
		ent = bullets+i;
		if (WallCollide(ent))
		{
			Destroy(bullets, i, &nBullet);
			i--;
		}
	}
	for (int i = 0; i < nEnemyBullet; i++)	// �����ڵ�
	{
		ent = enemyBullets+i;
		if (WallCollide(ent))
		{	
			Destroy(enemyBullets, i, &nEnemyBullet);
			i--;
		}
	}
	for (int i = 0; i < nnls; i++)	// ����ʯ
	{
		ent = nls + i;
		if (WallCollide(ent))
		{
			Destroy(nls, i, &nnls);
			i--;
		}
	}
	WallCollide(&player);			// ���
}
// ���Ʋ���ָ������Ϸʵ��
void DrawEntity(HDC hdc, const Entity *ent)
{
	HBRUSH brush;
	brush = CreateSolidBrush(ent->c);		// ����ʵ��ָ������ɫ������ˢ
	RECT rc;								// ʵ�峤����
	rc.top = ent->y-ent->s/2;
	rc.left = ent->x-ent->s/2;
	rc.bottom = ent->y+ent->s/2;
	rc.right = ent->x+ent->s/2;
	StretchBlt(hdc, rc.left, rc.top, rc.right-rc.left, rc.bottom-rc.top, hdc_object, 1, 1, 16, 16, SRCCOPY);
	if (!ent->b)							// ������ʵ�岻���ӵ�,�����ݽ�ɫ���ͻ�����Ͳ
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
	brush = CreateSolidBrush(ent->c);		// ����ʵ��ָ������ɫ������ˢ
	RECT rc;								// ʵ�峤����
	rc.top = ent->y - ent->s / 2;
	rc.left = ent->x - ent->s / 2;
	rc.bottom = ent->y + ent->s / 2;
	rc.right = ent->x + ent->s / 2;
	StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc_object, 1, 70, 16, 16, SRCCOPY);
	if (!ent->b)							// ������ʵ�岻���ӵ�,�����ݽ�ɫ���ͻ�����Ͳ
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
	brush = CreateSolidBrush(ent->c);		// ����ʵ��ָ������ɫ������ˢ
	RECT rc;								// ʵ�峤����
	rc.top = ent->y - ent->s / 2;
	rc.left = ent->x - ent->s / 2;
	rc.bottom = ent->y + ent->s / 2;
	rc.right = ent->x + ent->s / 2;
	StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc_object, 65, 196, 3, 3, SRCCOPY);
	if (!ent->b)							// ������ʵ�岻���ӵ�,�����ݽ�ɫ���ͻ�����Ͳ
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
	brush = CreateSolidBrush(ent->c);		// ����ʵ��ָ������ɫ������ˢ
	RECT rc;								// ʵ�峤����
	rc.top = ent->y - ent->s / 2;
	rc.left = ent->x - ent->s / 2;
	rc.bottom = ent->y + ent->s / 2;
	rc.right = ent->x + ent->s / 2;
	StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc_object, 138, 196, 3, 3, SRCCOPY);
	if (!ent->b)							// ������ʵ�岻���ӵ�,�����ݽ�ɫ���ͻ�����Ͳ
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
	brush = CreateSolidBrush(ent->c);		// ����ʵ��ָ������ɫ������ˢ
	RECT rc;								// ʵ�峤����
	rc.top = ent->y - ent->s / 2;
	rc.left = ent->x - ent->s / 2;
	rc.bottom = ent->y + ent->s / 2;
	rc.right = ent->x + ent->s / 2;
	StretchBlt(hdc, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc_object, 202, 175, 16, 16, SRCCOPY);
	if (!ent->b)							// ������ʵ�岻���ӵ�,�����ݽ�ɫ���ͻ�����Ͳ
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
// ����������Ϸ����,�����е��ø��ԵĻ��ƺ�����������
void DrawScene(HDC hdc)
{
	// ������Ϸ��ʾ��Ϣ
	HFONT hf;
	WCHAR str[32];
	long lfHeight;
	lfHeight = -MulDiv(16, GetDeviceCaps(hdc, LOGPIXELSY), 72);
	hf = CreateFont(lfHeight, 0, 0, 0, 0, TRUE, 0, 0, 0, 0, 0, 0, 0, L"Times New Roman");
	HFONT hfOld = (HFONT)SelectObject(hdc, hf);
	if (nLife <= 0)							// ����������Ϊ0,����ʾ��������
	{
		SetTextColor(hdc, RGB(122, 0, 0));
		TextOut(hdc, wndWidth/2-100, wndHeight/2-40,L"Game Over",9);
		SelectObject(hdc, hfOld);
		mciSendString(L"close background.wav", NULL, 0, NULL);
		mciSendString(L"play gameover.wav", NULL, 0, NULL);
		return;
	}
	if (nScore >= 18)						// �����ҵ÷�18,����ʾ��һ��
	{
		SetTextColor(hdc, RGB(122, 0, 0));
		TextOut(hdc, wndWidth / 2 - 100, wndHeight / 2 - 40, L"Next Level", 9);
		SelectObject(hdc, hfOld);
		mciSendString(L"close background.wav", NULL, 0, NULL);
		mciSendString(L"play next.wav", NULL, 0, NULL);
		return;
	}
	// ��ʾ��Ϸͳ����Ϣ
	SetTextColor(hdc, RGB(100, 100, 100));
	wsprintf(str, L"Life:%d Score:%d", nLife, nScore);	
	TextOut(hdc, 0, 0,str,wcslen(str));
	SelectObject(hdc, hfOld);				// �ָ�Ĭ������
	DeleteObject(hf);

	// ���Ƹ�����Ϸʵ��
	Entity* ent = NULL;
	for (int i = 0; i < nEnemy; i++)		// ����
	{
		ent = enemys+i;
		DrawEntity(hdc, ent);
	}
	for (int i = 0; i < nBullet; i++)		// ��ҷ������ڵ�
	{
		ent = bullets+i;
		Drawplayerbullent(hdc, ent);
	}
	for (int i = 0; i < nEnemyBullet; i++)	// ���˷������ڵ�
	{
		ent = enemyBullets+i;
		Drawenemybullent(hdc, ent);
	}
	for (int i = 0; i < nnls; i++)	// ����ʯ
	{
		ent = nls + i;
		Drawstone(hdc, ent);
	}

	Drawplayer(hdc, &player);				// ���
}
//DWORD WINAPI BE(LPVOID DO)//���̵߳��ú���
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
	//�������߳�
	//HANDLE hBE=CreateThread(NULL, 0, BE,(LPVOID)1 , 0, NULL); 
	srand(time(NULL));		// �����������
	Init();					// ��Ϸ��Ϣ��ʼ��

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
	case WM_CREATE:			// ����������,��ʼ����һ����ʱ��
		SetTimer(hWnd,1,timeStep,NULL);
		SetTimer(hWnd, 2, 1000, NULL);	// ���ڸı�����˶�����Ķ�ʱ��
		break;
	case WM_TIMER:			// ��ʱ����Ӧ
		InvalidateRect(hWnd, NULL, TRUE);	// �ô��ڱ�Ϊ��Ч,�Ӷ������ػ���Ϣ
		if (wParam == 1)	// ����Ϸ���и���
		{
			if (nLife > 0)
				Update(timeStep / 10);
		}
		else if (wParam == 2)
			ChangeEnemyDir(enemys);
		break;
	case WM_SIZE:			// ��ȡ���ڵĳߴ�
		wndWidth = LOWORD(lParam);
		wndHeight = HIWORD(lParam);
		break;
	case WM_KEYDOWN:		// ��Ұ��¼��̰���
		{
			InvalidateRect(hWnd, NULL, TRUE);
			switch (wParam)	// ��������������Ϣ������ҿ��Ƶ�̹��״̬
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
			case VK_SPACE:	// ���
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
	case WM_ERASEBKGND:		// ����������,������˸
		break;
	case WM_PAINT:
		{	
			hdc = BeginPaint(hWnd, &ps);
			HDC memHDC = CreateCompatibleDC(hdc);
			RECT rectClient;
			GetClientRect(hWnd, &rectClient);

			//����λͼ
			HBITMAP bmpBuff = CreateCompatibleBitmap(hdc,wndWidth,wndHeight);
			HBITMAP pOldBMP = (HBITMAP)SelectObject(memHDC, bmpBuff);
			PatBlt(memHDC,0,0,wndWidth,wndHeight, BLACKNESS);	// ���ñ���Ϊ��ɫ

			// ���������Ļ���
			DrawScene(memHDC);

			//�����ڴ�HDC���ݵ�ʵ��HDC
			BOOL tt = BitBlt(hdc, rectClient.left, rectClient.top, wndWidth,
				wndHeight, memHDC, rectClient.left, rectClient.top, SRCCOPY);

			//�ڴ����
			SelectObject(memHDC, pOldBMP);
			DeleteObject(bmpBuff);
			DeleteDC(memHDC);

			EndPaint(hWnd, &ps);
			break;
		}
	case WM_DESTROY:
		KillTimer(hWnd,1);		// �����˳�ʱ������ʱ��ɾ��
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