#pragma once

#define FLOAT2LPARAM(fl)        static_cast<LPARAM>(static_cast<int>((fl) * 100000.f))
#define LPARAM2FLOAT(l)         (static_cast<float>(l) / 100000.f)

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

#define WM_APP_SESSION_DUCKED           WM_APP
#define WM_APP_SESSION_UNDUCKED         WM_APP+1
#define WM_APP_GRAPHNOTIFY              WM_APP+2
#define WM_APP_SESSION_VOLUME_CHANGED   WM_APP+3    // wParam = Mute State, lParam = (float) new volume 0.0 - 1.0
#define WM_APP_GAME_OVER                WM_APP+4
