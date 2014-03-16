#include "MainWindow.h"
#include <WindowsX.h>
#include "Macro.h"
#include <algorithm>
#include <process.h>
#include <CommCtrl.h>

// std::min, std::max��min�}�N��, max�}�N���̖��O���Փ˂���
#undef min
#undef max

// Direct2D�̃I�u�W�F�N�g���g�����v�Z���y�ɂ��邽�߂ɉ��Z�q���I�[�o�[���[�h���Ă���
inline D2D1_RECT_F operator+(D2D1_RECT_F r, D2D1_POINT_2F &p)
{
	r.left += p.x;
	r.top += p.y;
	if(r.right != D2D1::FloatMax())
		r.right += p.x;
	if(r.bottom != D2D1::FloatMax())
		r.bottom += p.y;
	return r;
}
inline D2D1_RECT_F operator+(D2D1_POINT_2F &p, D2D1_RECT_F r)
{
	return r + p;
}
inline D2D1_RECT_F operator*(D2D1_RECT_F r, FLOAT f)
{
	r.left *= f;
	r.top *= f;
	if(r.right != D2D1::FloatMax())
		r.right *= f;
	if(r.bottom != D2D1::FloatMax())
		r.bottom *= f;
	return r;
}
inline D2D1_RECT_F operator*(FLOAT f, D2D1_RECT_F r)
{
	return r * f;
}
inline D2D1_POINT_2F operator+(D2D1_POINT_2F p1, D2D1_POINT_2F p2)
{
	p1.x += p2.x;
	p1.y += p2.y;
	return p1;
}
inline D2D1_POINT_2F operator*(D2D1_POINT_2F p, FLOAT f)
{
	p.x *= f;
	p.y *= f;
	return p;
}
inline D2D1_POINT_2F operator*(FLOAT f, D2D1_POINT_2F p)
{
	return p * f;
}


// �E�B���h�E�v���V�[�W��
LRESULT CALLBACK MainWindow::WindowProc_SetData(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto w = reinterpret_cast<MainWindow*>(::GetWindowLongPtrW(hwnd, 0));
	if(msg == WM_NCCREATE){
		auto lpcs = reinterpret_cast<LPCREATESTRUCT>(lParam);
		::SetWindowLongPtrW(hwnd, 0, reinterpret_cast<LONG_PTR>(lpcs->lpCreateParams));
		::SetWindowLongPtrW(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc_Static));
		w = reinterpret_cast<MainWindow*>(lpcs->lpCreateParams);
		w->hwnd = hwnd;
	}
	if(w == NULL)
		return ::DefWindowProcW(hwnd, msg, wParam, lParam);
	return w->WindowProc(hwnd, msg, wParam, lParam);
}

LRESULT CALLBACK MainWindow::WindowProc_Static(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	auto w = reinterpret_cast<MainWindow*>(::GetWindowLongPtrW(hwnd, 0));
	if(w == NULL)
		return ::DefWindowProcW(hwnd, msg, wParam, lParam);
	return w->WindowProc(hwnd, msg, wParam, lParam);
}

void MainWindow::InitRawInput()
{
	RAWINPUTDEVICE device;
	device.usUsagePage = 0x01;
	device.usUsage = 0x02;
	device.dwFlags = RIDEV_DEVNOTIFY;
	device.hwndTarget = hwnd;
	if(::RegisterRawInputDevices(&device,1,sizeof device) == 0)
		return;
}

void MainWindow::InitDeviceHandles()
{
	player[0].h = player[1].h = NULL;
}

LRESULT MainWindow::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch(msg){
	case WM_CREATE:
		if(!OnCreate())
			return -1;
		InitRawInput();
		return 0;
	case WM_CLOSE:
		::DestroyWindow(hwnd);
		return 0;
	case WM_DESTROY:
		OnDestroy();
		::PostQuitMessage(0);
		return 0;
	case WM_SIZE:
		OnSize(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_SETFOCUS:
		OnSetFocus();
		return 0;
	case WM_KILLFOCUS:
		OnKillFocus();
		return 0;
	case WM_LBUTTONDOWN:
		OnLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		return 0;
	case WM_KEYDOWN:
		OnKeyDown(wParam);
		return 0;
	case WM_PAINT:
		OnPaint();
		return 0;
	case WM_ERASEBKGND:
		// �w�i��`�悵�Ȃ���������Ȃ��Ȃ�
		return 0;
	case WM_INPUT_DEVICE_CHANGE:
		InitDeviceHandles();
		return 0;
	case WM_INPUT:
		OnInput(static_cast<UINT>(wParam), reinterpret_cast<HRAWINPUT>(lParam));
		return 0;
	case WM_APP_GRAPHNOTIFY:
		reinterpret_cast<CMediaPlayer*>(lParam)->HandleGraphEvent();
		return 0;
	case WM_APP_GAME_OVER:
		OnGameover();
		return 0;
	}
	return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}

// ������
bool MainWindow::OnCreate()
{
	thid = NULL;
	end = false;

	// Direct2D��DirectWrite��������
	if(FAILED(::D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory))){
		::MessageBoxW(hwnd, L"Direct2D�̏������Ɏ��s���܂���", NULL, MB_OK);
		return false;
	}
	if(FAILED(::DWriteCreateFactory(DWRITE_FACTORY_TYPE_ISOLATED, __uuidof(df), reinterpret_cast<IUnknown**>(&df)))){
		::MessageBoxW(hwnd, L"DirectWrite�̏������Ɏ��s���܂���", NULL, MB_OK);
		return false;
	}

	// �t�H���g��������
	df->CreateTextFormat(
		L"Meiryo",
		NULL,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		30.f,
		L"ja-JP",
		&font);

	// ���ʉ�����ǂݍ���
	const WCHAR *sounds_filenames[] = {
		L"Backmusic1.wav",
		L"Collision1.wav",
		L"Collision1.wav",
		L"Collision2.wav",
		L"Goal.wav",
		L"Login.wav",
		L"Logout.wav"
	};
	for(int i = 0; i < _countof(sounds); ++i){
		try{
			sounds[i] = new CMediaPlayer(hwnd);
		}catch(std::bad_alloc &e){
			::MessageBoxW(hwnd, L"�������̊m�ۂɎ��s���܂���", NULL, MB_OK);
			return false;
		}
		if(FAILED(sounds[i]->Initialize())){
			::MessageBoxW(hwnd, L"CMediaPlayer�̏������Ɏ��s���܂���", NULL, MB_OK);
			return false;
		}
		if(!sounds[i]->SetFileName(sounds_filenames[i])){
			WCHAR msg[500];
			::swprintf(msg, L"'%s'�̓ǂݍ��݂Ɏ��s���܂���", sounds_filenames[i]);
			::MessageBoxW(hwnd, msg, NULL, MB_OK);
			return false;
		}
	}
	sounds[BGM]->SetRepeat(true);

	ResetPack(1);
	player[0].score = 0;
	player[1].score = 0;

	rotate = false;

	return true;
}

// �㏈��
void MainWindow::OnDestroy()
{
	EndThread();
	// �S�ẴI�u�W�F�N�g��j��
	if(target != NULL)
		target->Release();
	if(factory != NULL)
		factory->Release();
	for(int i = 0; i < _countof(sounds); ++i){
		if(sounds[i] != NULL){
			sounds[i]->Shutdown();
			sounds[i]->Release();
		}
	}
	DestroyResource();
	hwnd = NULL;
	::WaitForSingleObject(reinterpret_cast<HANDLE>(thid), 1000);
}

void MainWindow::OnSize(int width, int height)
{
	// �E�B���h�E�̃T�C�Y���ύX���ꂽ��
	scr_size.width = static_cast<FLOAT>(width);
	scr_size.height = static_cast<FLOAT>(height);
	// �K�؂Ȕ{�����v�Z����
	if(scr_size.width / bg_size.width < scr_size.height / bg_size.height){
		scale = scr_size.width / bg_size.width;
		diff.x = 0;
		diff.y = (scr_size.height - bg_size.height * scale) / 2;
	}else{
		scale = scr_size.height / bg_size.height;
		diff.x = (scr_size.width - bg_size.width * scale) / 2;
		diff.y = 0;
	}
	if(target != NULL)
		target->Resize(D2D1::SizeU(width, height));
}

// �N���b�N���ꂽ��
void MainWindow::OnSetFocus()
{
	// �J�[�\�����E�B���h�E�̏�ɂ��鎞�̓J�[�\�����B���ē����Ȃ�����
	hasfocus = true;
	::ShowCursor(FALSE);
	{
		POINT pt;
		::GetCursorPos(&pt);

		RECT rc = {pt.x,pt.y,pt.x,pt.y};
		::ClipCursor(&rc);
	}
}

// ���̑�����O�ɗ�����ESC���������肵����
void MainWindow::OnKillFocus()
{
	hasfocus = false;
	::ShowCursor(TRUE);
	::ClipCursor(NULL);
}

// �N���b�N
void MainWindow::OnLButtonDown(int x, int y)
{
	::SetFocus(hwnd);
}

// �L�[�{�[�h���������Ƃ�
void MainWindow::OnKeyDown(int key)
{
	switch(key){
	case VK_ESCAPE:
		::SetFocus(NULL);
		break;
	case L'R':
		// R�L�[�Ń}���b�g�̓���������ς���
		rotate = !rotate;
		break;
	case L'S':
		// S�L�[�Ńv���C���[�����ւ�
		if(player[1].h != NULL)
			std::swap(player[0].h, player[1].h);
		break;
	case L'C':
		// C�L�[�Ń}�E�X��������
		EndThread();
		player[0].h = player[1].h = NULL;
		sounds[BGM]->Stop();
		sounds[Logout]->Play();
		::InvalidateRect(hwnd, NULL, FALSE);
		break;
	case VK_OEM_PLUS:
		if(::GetKeyState(VK_SHIFT) >= 0)
			break;
		// +�L�[�Ń}���b�g���p�b�N��傫������
		if(::GetKeyState(VK_CONTROL) < 0)	// �R���g���[���L�[��������Ă�����
			data.p_r++;						// �p�b�N��傫������
		else
			data.m_r++;
		::InvalidateRect(hwnd, NULL, FALSE);
		break;
	case VK_OEM_MINUS:
		// -�Ń}���b�g���p�b�N������������
		if(::GetKeyState(VK_CONTROL) < 0)	// �R���g���[���L�[��������Ă�����
			data.p_r--;						// �p�b�N������������
		else
			data.m_r--;
		::InvalidateRect(hwnd, NULL, FALSE);
	}
}

// �}�E�X����������
void MainWindow::OnInput(UINT code, HRAWINPUT hinput)
{
	// Raw Input�̏���

	if(!hasfocus)
		return;

	UINT dwSize;

	GetRawInputData(hinput, RID_INPUT, NULL, &dwSize, 
					sizeof(RAWINPUTHEADER));
	LPBYTE lpb;
	try{
		lpb = new BYTE[dwSize];
	}catch(std::bad_alloc &e){
		return;
	}

	if (GetRawInputData(hinput, RID_INPUT, lpb, &dwSize, sizeof(RAWINPUTHEADER)) != dwSize ){
		delete[] lpb;
		return;
	}

	RAWINPUT* raw = (RAWINPUT*)lpb;
	if(raw->header.dwType == RIM_TYPEMOUSE){
		// �}�E�X�̓��͂��󂯎������
		if(raw->header.hDevice == player[0].h || raw->header.hDevice == player[1].h){
			if(player[1].h != NULL){
				// �����̃}���b�g���}�E�X�Ɗ֘A�t�����Ă���
				// ���������}�E�X���}���b�g�Ɗ֘A�t�����Ă�����
				if((raw->data.mouse.usFlags & 0x1) == MOUSE_MOVE_RELATIVE){
					int idx = raw->header.hDevice == player[1].h? 1: 0;

					::EnterCriticalSection(&cs);

					if(!rotate){
						// �}�E�X�ɃJ�o�[�����Ԃ��Ă��鎞(�}�E�X�������������Ƀv���C���鎞)�̃}���b�g�̏���
						player[idx].pos.x += raw->data.mouse.lLastX / scale;
						player[idx].pos.y += raw->data.mouse.lLastY / scale;
					}else{
						// �}�E�X�ɃJ�o�[�����Ԃ��ĂȂ���(�}�E�X�����������鎞)
						if(idx == 0){
							player[0].pos.x -= raw->data.mouse.lLastY / scale;
							player[0].pos.y += raw->data.mouse.lLastX / scale;
						}else{
							player[1].pos.x += raw->data.mouse.lLastY / scale;
							player[1].pos.y -= raw->data.mouse.lLastX / scale;
						}
					}

					// �}���b�g�̓����𐧌�����
					if(idx == 0){
						if(player[0].pos.x < data.field.left + data.m_r)
							player[0].pos.x = data.field.left + data.m_r;
						else if(player[0].pos.x > (data.field.left + data.field.right) / 2 - data.m_r)
							player[0].pos.x = (data.field.left + data.field.right) / 2 - data.m_r;
					}else{
						if(player[1].pos.x < (data.field.left + data.field.right) / 2 + data.m_r)
							player[1].pos.x = (data.field.left + data.field.right) / 2 + data.m_r;
						else if(player[1].pos.x > data.field.right - data.m_r)
							player[1].pos.x = data.field.right - data.m_r;
					}

					if(player[idx].pos.y < data.field.top + data.m_r)
						player[idx].pos.y = data.field.top + data.m_r;
					else if(player[idx].pos.y > data.field.bottom - data.m_r)
						player[idx].pos.y = data.field.bottom - data.m_r;

					if(idx == 0)
						player[0].pos.x = std::min(player[0].pos.x, (data.field.left + data.field.right) / 2 - data.m_r);
					else
						player[1].pos.x = std::max(player[1].pos.x, (data.field.left + data.field.right) / 2 + data.m_r);


					::LeaveCriticalSection(&cs);
				}
			}
		}else{
			// �����̃}���b�g���}�E�X�Ɗ֘A�t�����Ă��Ȃ���
			if(player[0].h == NULL){
				// 1P�̃}���b�g���֘A�t�����ĂȂ���

				// ���O�C���������̌��ʉ���炷
				sounds[Login]->Play();
				player[0].h = raw->header.hDevice; // �}�E�X�ƃ}���b�g���֘A�t����
				
				// �ʒu��������
				player[0].ppos = player[0].pos = D2D1::Point2F(100.f, 400.f);
				::InvalidateRect(hwnd, NULL, FALSE);
			}else if(player[1].h == NULL){
				// 2P�̃}���b�g

				sounds[Login]->Play();
				player[1].h = raw->header.hDevice;
				player[1].ppos = player[1].pos = D2D1::Point2F(800.f, 400.f);
				if(!BeginThread()){
					::MessageBoxW(hwnd, L"�X���b�h�̍쐬�Ɏ��s���܂���", NULL, MB_OK);
					::DestroyWindow(hwnd);
					return;
				}

				// �����̃}���b�g���}�E�X�Ɗ֘A�t����ꂽ��BGM�𗬂�
				sounds[BGM]->Play();
			}
		}
	}
}

// �`��
void MainWindow::OnPaint()
{
	// �w�i�A�}���b�g�A�p�b�N��`�悷��

	HRESULT hr;
	using namespace D2D1;
	::EnterCriticalSection(&cs);

	if(CreateResource()){
		PAINTSTRUCT ps;
		::BeginPaint(hwnd, &ps);
		::EndPaint(hwnd, &ps);

		target->BeginDraw();

		target->Clear();
		target->DrawBitmap(bg, RectF(0, 0, bg_size.width, bg_size.height) * scale + diff);

		for(int i = 0; i < _countof(data.lines); ++i){
			target->DrawLine(data.lines[i][0] * scale + diff, data.lines[i][1] * scale + diff, wbr, 5.f);
		}

		if(player[0].h != NULL)
			target->FillEllipse(Ellipse(player[0].pos * scale + diff, data.m_r * scale, data.m_r * scale), mbr[0]);
		if(player[1].h != NULL)
			target->FillEllipse(Ellipse(player[1].pos * scale + diff, data.m_r * scale, data.m_r * scale), mbr[1]);
		target->FillEllipse(Ellipse(p_pos * scale + diff, data.p_r * scale, data.p_r * scale), pbr);

		{
			WCHAR str[100];
			::wsprintfW(str, L"%d", player[0].score);
			target->DrawTextW(str, ::wcslen(str), font, RectF((data.field.right - data.field.left)/3, data.field.bottom+12, FloatMax(), FloatMax()) * scale + diff, sbr);
			::wsprintfW(str, L"%d", player[1].score);
			target->DrawTextW(str, ::wcslen(str), font, RectF((data.field.right - data.field.left)*2/3, data.field.bottom+12, FloatMax(), FloatMax()) * scale + diff, sbr);
		}

		if((hr = target->EndDraw()) == D2DERR_RECREATE_TARGET){
			DestroyResource();
			::InvalidateRect(hwnd, NULL, FALSE);
		}
	}

	::LeaveCriticalSection(&cs);
}

bool MainWindow::CreateResource()
{
	// Direct2D�̃I�u�W�F�N�g(�u���V�A�w�i��)��ǂݍ���

	if(target != NULL)
		return true;

	RECT rc;
	::GetClientRect(hwnd, &rc);

	if(FAILED(factory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(rc.right, rc.bottom)),
			&target))){
		::MessageBoxW(hwnd, L"ID2D1HwndRenderTarget�̍쐬�Ɏ��s���܂���", NULL, MB_OK);
		return false;
	}

	if(FAILED(target->CreateSolidColorBrush(data.m_color[0], &mbr[0]))){
		::MessageBoxW(hwnd, L"�u���V�̍쐬�Ɏ��s���܂���", NULL, MB_OK);
		return false;
	}

	if(FAILED(target->CreateSolidColorBrush(data.m_color[1], &mbr[1]))){
		::MessageBoxW(hwnd, L"�u���V�̍쐬�Ɏ��s���܂���", NULL, MB_OK);
		return false;
	}

	if(FAILED(target->CreateSolidColorBrush(data.p_color, &pbr))){
		::MessageBoxW(hwnd, L"�u���V�̍쐬�Ɏ��s���܂���", NULL, MB_OK);
		return false;
	}

	if(FAILED(target->CreateSolidColorBrush(data.w_color, &wbr))){
		::MessageBoxW(hwnd, L"�u���V�̍쐬�Ɏ��s���܂���", NULL, MB_OK);
		return false;
	}

	if(FAILED(target->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &sbr))){
		::MessageBoxW(hwnd, L"�u���V�̍쐬�Ɏ��s���܂���", NULL, MB_OK);
		return false;
	}

	if(!decoder.Decode(L"hockeyfield3_2.png", target, &bg))
		return false;

	return true;
}

void MainWindow::DestroyResource()
{
	// Direct2D�p�̃I�u�W�F�N�g��j������
	SafeRelease(&mbr[0]);
	SafeRelease(&mbr[1]);
	SafeRelease(&pbr);
	SafeRelease(&wbr);
	SafeRelease(&sbr);
	SafeRelease(&bg);
	SafeRelease(&target);
	SafeRelease(&font);
	SafeRelease(&df);
}

void MainWindow::ResetPack(int player)
{
	// �p�b�N�̈ʒu�����Z�b�g����

	if(player == 0){
		p_pos.x = (data.field.right-data.field.left)*3/4+data.field.left;
		p_pos.y = (data.field.bottom-data.field.top)/2+data.field.top;
	}else{
		p_pos.x = (data.field.right-data.field.left)/4+data.field.left;
		p_pos.y = (data.field.bottom-data.field.top)/2+data.field.top;
	}
	p_pos.x += data.p_r;
	p_pos.y += data.p_r;
}

void MainWindow::OnGameover()
{
	// �������������Ƀ_�C�A���O��\������
	TASKDIALOG_BUTTON buttons[] = {
		{1000, L"Retry(&R)"},
		{1001, L"Exit(&E)"}
	};
	TASKDIALOGCONFIG conf = {};
	conf.cbSize = sizeof conf;
	conf.hInstance = hinst;
	conf.hwndParent = hwnd;
	conf.pButtons = buttons;
	conf.cButtons = _countof(buttons);
	conf.pszWindowTitle = L"Game Over";
	conf.pszMainInstruction = player[0].score >= data.setscore? L"1P WIN!": L"2P WIN!";

	EndThread();

	// �_�C�A���O��\��
	int sel;
	::TaskDialogIndirect(&conf, &sel, NULL, NULL);
	if(sel == 1000){
		// Retry(R)
		ResetPack(1);
		player[0].score = 0;
		player[1].score = 0;
		BeginThread();
		if(thid == NULL){
			::MessageBoxW(hwnd, L"�X���b�h�̍쐬�Ɏ��s���܂���\n�Q�[�����I�����܂�", NULL, MB_OK);
			::DestroyWindow(hwnd);
		}
	}else{
		// Exit(E)
		::DestroyWindow(hwnd);
	}
}
