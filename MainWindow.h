#pragma once

// 背景画像の大きさは939x675で固定

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

	// 画像ファイルを読み込むためのオブジェクト
	Decoder decoder;

	// Direct2Dのファクトリ(オブジェクトを生成するためのオブジェクト)
	ID2D1Factory *factory;

	// ウィンドウハンドル
	HWND hwnd;

	// Direct2Dで描画するのに必要なオブジェクト
	ID2D1HwndRenderTarget *target;

	// 背景画像
	ID2D1Bitmap *bg;

	// 単色のブラシ(マレット、パック、壁、スコアを描くためのもの)
	ID2D1SolidColorBrush *mbr[2], *pbr, *wbr, *sbr;

	// DirectWriteのファクトリ
	IDWriteFactory *df;
	// フォント
	IDWriteTextFormat *font;

	// 音楽を読み込んで保持するためのオブジェクト
	enum Sound{
		BGM,
		Col1_1, // 1Pとパックが当たった時の音
		Col1_2, // 2Pとパック
		Col2,   // パックと壁
		Goal,
		Login,
		Logout,
		Sound_Max
	};
	CMediaPlayer *sounds[Sound_Max];


	bool hasfocus;

	// マレットの動かし方をどうするか保持しておく
	bool rotate;

	// プレイヤーの情報を保存しておく
	struct Player{
		HANDLE h;
		D2D1_POINT_2F pos, ppos;
		int score;
	}player[2];

	// パックの位置
	D2D1_POINT_2F p_pos;

	// ゲーム中に変化しないデータ(色、壁の位置、マレットとパックの大きさ)を保持するための構造体
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
	
	// 背景画像→画面の倍率
	FLOAT scale;

	D2D1_POINT_2F diff;


	// 背景と画面の大きさを保持する
	struct Size{
		FLOAT width, height;
		Size(FLOAT w, FLOAT h):width(w), height(h)
		{
		}
	}bg_size, scr_size;

	// パックの移動を処理するためのスレッド
	static unsigned int __stdcall Run(void *p);
	// スレッドのID(thread id → thid)
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
	// ウィンドウクラスを登録する
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
	// ウィンドウを作る
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
	// 初期化
	bool OnCreate();
	// 後処理
	void OnDestroy();
	// サイズが変わった時
	void OnSize(int width, int height);
	// クリックされた時
	void OnSetFocus();
	// 他の窓が手前に来たりESCを押したりした時
	void OnKillFocus();
	void OnLButtonDown(int x, int y);
	// キーボードを押した時
	void OnKeyDown(int key);
	// 描画
	void OnPaint();
	// マウスが動いた時
	void OnInput(UINT code, HRAWINPUT hinput);
	// 勝負がついた時
	void OnGameover();

	void ResetPack(int player);
};
