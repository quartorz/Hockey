#include <Windows.h>
#include "MainWindow.h"

HINSTANCE hinst;


#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "strmiids")
#pragma comment(lib, "rpcrt4")
#pragma comment(lib, "d2d1")
#pragma comment(lib, "dwrite")
#pragma comment(lib, "WindowsCodecs")
#pragma comment(lib, "Comctl32")


// from DuckingMediaPlayerSample of Windows SDK v7.1
bool IsWin7OrLater()
{
    bool bWin7OrLater = true;

    OSVERSIONINFO ver = {};
    ver.dwOSVersionInfoSize = sizeof(ver);

    if (GetVersionEx(&ver))
    {
        bWin7OrLater = (ver.dwMajorVersion > 6) ||
                       ((ver.dwMajorVersion == 6) && (ver.dwMinorVersion >= 1));
    }

    return bWin7OrLater;
}

int APIENTRY wWinMain(HINSTANCE hInst, HINSTANCE, LPWSTR, int)
{
	if(!IsWin7OrLater()){
		::MessageBoxW(NULL, L"���̃v���O������Windows 7�ȍ~�ł������s�ł��܂���", NULL, MB_OK);
		return 0;
	}

	// �������珉����
	hinst = hInst;
	if(!MainWindow::Register()){
		::MessageBoxW(NULL, L"�E�B���h�E�N���X�̓o�^�Ɏ��s���܂���", NULL, MB_OK);
		return 0;
	}

	::CoInitialize(NULL);

	MSG msg;

	{
		MainWindow mw;
		if(!mw.Create()){
			::MessageBoxW(NULL, L"�E�B���h�E�̍쐬�Ɏ��s���܂���", NULL, MB_OK);
			::CoUninitialize();
			return 0;
		}
		// �����܂ŏ�����

		BOOL bRet;
		while( (bRet = GetMessage(&msg, NULL, 0, 0)) && (bRet != -1)){
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	::CoUninitialize();
	return msg.wParam;
}
