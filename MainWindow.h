#pragma once

// �w�i�摜�̑傫����939x675�ŌŒ�

#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include "MediaPlayer.h"
#include"Decoder.h"
#include <process.h>

extern HINSTANCE hinst;

class MainWindow{
	static LRESULT CALLBACK WindowProc_SetData(HWND, UINT, WPARAM, LPARAM);
	static LRESULT CALLBACK WindowProc_Static(HWND, UINT, WPARAM, LPARAM);

	// �摜�t�@�C����ǂݍ��ނ��߂̃I�u�W�F�N�g
	Decoder decoder;

	// Direct2D�̃t�@�N�g��(�I�u�W�F�N�g�𐶐����邽�߂̃I�u�W�F�N�g)
	ID2D1Factory *factory;

	// �E�B���h�E�n���h��
	HWND hwnd;

	// Direct2D�ŕ`�悷��̂ɕK�v�ȃI�u�W�F�N�g
	ID2D1HwndRenderTarget *target;

	// �w�i�摜
	ID2D1Bitmap *bg;

	// �P�F�̃u���V(�}���b�g�A�p�b�N�A�ǁA�X�R�A��`�����߂̂���)
	ID2D1SolidColorBrush *mbr[2], *pbr, *wbr, *sbr;

	// DirectWrite�̃t�@�N�g��
	IDWriteFactory *df;
	// �t�H���g
	IDWriteTextFormat *font;

	// ���y��ǂݍ���ŕێ����邽�߂̃I�u�W�F�N�g
	enum Sound{
		BGM,
		Col1_1, // 1P�ƃp�b�N�������������̉�
		Col1_2, // 2P�ƃp�b�N
		Col2,   // �p�b�N�ƕ�
		Goal,
		Login,
		Logout,
		Sound_Max
	};
	CMediaPlayer *sounds[Sound_Max];


	bool hasfocus;

	// �}���b�g�̓����������ǂ����邩�ێ����Ă���
	bool rotate;

	// �v���C���[�̏���ۑ����Ă���
	struct Player{
		HANDLE h;
		D2D1_POINT_2F pos, ppos;
		int score;
	}player[2];

	// �p�b�N�̈ʒu
	D2D1_POINT_2F p_pos;

	// �Q�[�����ɕω����Ȃ��f�[�^(�F�A�ǂ̈ʒu�A�}���b�g�ƃp�b�N�̑傫��)��ێ����邽�߂̍\����
	struct Data{
		FLOAT m_r, p_r;
		D2D1_COLOR_F m_color[2], p_color, w_color;
		D2D1_RECT_F field;
		D2D1_POINT_2F lines[6][2];
		int setscore;

		Data():
			m_r(::GetPrivateProfileIntW(L"Size", L"mallet", 28, L".\\config.ini")),
			p_r(::GetPrivateProfileIntW(L"Size", L"pack", 15, L".\\config.ini")),
			setscore(3)
		{
			m_color[0] = D2D1::ColorF(D2D1::ColorF::Red);
			m_color[1] = D2D1::ColorF(D2D1::ColorF::Blue);
			p_color = D2D1::ColorF(D2D1::ColorF::Yellow);
			w_color = D2D1::ColorF(D2D1::ColorF::White);
			field = D2D1::RectF(40, 46, 879, 629);
			D2D1_POINT_2F l[6][2] = {
				{D2D1::Point2F(field.left, field.top), D2D1::Point2F(field.right, field.top)},
				{D2D1::Point2F(field.left, field.top), D2D1::Point2F(field.left, (field.bottom - field.top) * 2 / 7 + field.top)},
				{D2D1::Point2F(field.left, field.bottom), D2D1::Point2F(field.right, field.bottom)},
				{D2D1::Point2F(field.right, field.top), D2D1::Point2F(field.right, (field.bottom - field.top) * 2 / 7 + field.top)},
				{D2D1::Point2F(field.left, (field.bottom - field.top) * 5 / 7 + field.top), D2D1::Point2F(field.left, field.bottom)},
				{D2D1::Point2F(field.right, (field.bottom - field.top) * 5 / 7 + field.top), D2D1::Point2F(field.right, field.bottom)},
			};
			::memcpy(lines, l, sizeof l);
		}
	}data;
	
	// �w�i�摜����ʂ̔{��
	FLOAT scale;

	D2D1_POINT_2F diff;


	// �w�i�Ɖ�ʂ̑傫����ێ�����
	struct Size{
		FLOAT width, height;
		Size(FLOAT w, FLOAT h):width(w), height(h)
		{
		}
	}bg_size, scr_size;

	// �p�b�N�̈ړ����������邽�߂̃X���b�h
	static unsigned int __stdcall Run(void *p);
	// �X���b�h��ID(thread id �� thid)
	uintptr_t thid;
	bool end;
	CRITICAL_SECTION cs;

	bool BeginThread()
	{
		if(thid != NULL)
			return true;
		thid = ::_beginthreadex(NULL, 0, Run, this, 0, NULL);
		return thid != NULL;
	}
	void EndThread()
	{
		end = true;
		::WaitForSingleObject(reinterpret_cast<HANDLE>(thid), INFINITE);
		end = false;
		thid = NULL;
	}

public:
	MainWindow(): factory(NULL), hwnd(NULL), target(NULL), bg(NULL), mbr(), pbr(NULL), wbr(NULL), sbr(NULL), sounds(), bg_size(939, 675),
					scr_size(::GetSystemMetrics(SM_CXSCREEN), ::GetSystemMetrics(SM_CYSCREEN))
	{
		::InitializeCriticalSection(&cs);
	}
	~MainWindow()
	{
		if(hwnd != NULL)
			::DestroyWindow(hwnd);
		::DeleteCriticalSection(&cs);
	}
	// �E�B���h�E�N���X��o�^����
	static bool Register(void)
	{
		WNDCLASSEXW wcex;
		wcex.cbSize			= sizeof(WNDCLASSEX);
		wcex.style			= CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc	= WindowProc_SetData;
		wcex.cbClsExtra		= 0;
		wcex.cbWndExtra		= sizeof(void*);
		wcex.hInstance		= hinst;
		wcex.hIcon			= NULL; 
		wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
		wcex.hbrBackground	= NULL;
		wcex.lpszMenuName	= NULL;
		wcex.lpszClassName	= L"Hockey_MainWindow";
		wcex.hIconSm		= NULL;

		return ::RegisterClassExW(&wcex) != NULL;
	}
	// �E�B���h�E�����
	bool Create()
	{
		hasfocus = false;
		HWND hwnd = ::CreateWindowExW(
			0,
			L"Hockey_MainWindow",
			L"Hockey",
			WS_POPUP | WS_BORDER,
			0,
			0,
			scr_size.width,
			scr_size.height,
			NULL,
			NULL,
			hinst,
			this);
		if(hwnd == NULL)
			return false;

		::ShowWindow(hwnd, SW_SHOW);
		return true;
	}

	bool CreateResource();
	void DestroyResource();

	void InitRawInput();
	void InitDeviceHandles();

	LRESULT WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	// ������
	bool OnCreate();
	// �㏈��
	void OnDestroy();
	// �T�C�Y���ς������
	void OnSize(int width, int height);
	// �N���b�N���ꂽ��
	void OnSetFocus();
	// ���̑�����O�ɗ�����ESC���������肵����
	void OnKillFocus();
	void OnLButtonDown(int x, int y);
	// �L�[�{�[�h����������
	void OnKeyDown(int key);
	// �`��
	void OnPaint();
	// �}�E�X����������
	void OnInput(UINT code, HRAWINPUT hinput);
	// ������������
	void OnGameover();

	void ResetPack(int player);
};
