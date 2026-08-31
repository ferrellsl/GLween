/*
 *	GLween Screensaver -- Windows screensaver entry point.
 *
 *	The rendering itself (Casket, Pumpkin, Ghosts, particles, lightning --
 *	see GLWnd.cpp and friends) is Jim Strong's original October 2000 NeHe
 *	Halloween-contest demo, "GLween" / "Xersist's Happy Halloween".
 *
 *	This file replaces the original tutorial-style WinMain (a single
 *	window, a "Fullscreen?" prompt, and an F1 toggle) with a proper .scr
 *	entry point:
 *
 *		/s          Run the screensaver -- one borderless topmost window
 *		            per monitor, exits on the first real mouse move,
 *		            click, or key press.
 *		/p <HWND>   Render a live preview into the small monitor thumbnail
 *		            in the Screen Saver settings dialog.
 *		/c [<HWND>] Show the (very short) configuration dialog.
 *		(no args)   Run in a single ordinary window -- convenient for
 *		            testing from the debugger; Windows itself never
 *		            invokes a screensaver this way.
 */

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

#include <gl\gl.h>
#include <gl\glu.h>

#include "GLWnd.h"

//////////////////////////////////////////////////////////////////////
// Types & Globals
//////////////////////////////////////////////////////////////////////

enum RunMode { MODE_SAVER, MODE_CONFIG, MODE_PREVIEW, MODE_WINDOWED };

struct MonitorGL
{
	HWND	hWnd;
	HDC		hDC;
	HGLRC	hRC;
	GLWnd	*scene;
	RECT	rect;
};

static const int	MAX_GLWEEN_MONITORS = 16;
static MonitorGL	g_mon[MAX_GLWEEN_MONITORS];
static int			g_monCount = 0;

static HINSTANCE	g_hInstance = NULL;
static const char	*WND_CLASS_NAME = "GLweenSaverWnd";

// Set only while an actual /s screensaver session is running -- gates
// whether mouse/keyboard activity should end the process. Off for the
// preview, config, and windowed-test modes.
static bool			g_exitOnInput = false;
static bool			g_haveInitialMouse = false;
static POINT		g_initialMouse;
static const int	MOUSE_MOVE_THRESHOLD = 10;		// Pixels Of Real Movement Before Exiting

static bool			g_cursorHidden = false;

// Set by RunWindowed()/RunPreview() (the two single-window modes) so
// WM_SIZE can live-resize the GL viewport as the user drags the window.
// Left NULL for the multi-monitor saver, which has no single "the" scene.
static GLWnd		*g_singleScene = NULL;

//////////////////////////////////////////////////////////////////////
// DPI Awareness
//
// Without this, Windows treats the app as DPI-unaware and virtualizes
// it: GetClientRect()/CreateWindow() etc. all hand back LOGICAL (96-DPI)
// coordinates while the window's actual on-screen pixel surface is
// larger on a scaled display. Since glViewport()/gluPerspective() get
// sized from that too-small logical rect, the rendered scene only fills
// a fraction of the real framebuffer -- e.g. exactly the upper-left
// quarter at 200% scaling. That's most obvious on the small preview
// thumbnail, but would affect every window this app creates.
//
// Per-Monitor-V2 (Windows 10 1703+) Is Loaded Dynamically So This Still
// Builds And Runs Fine On Older Windows -- Falls Back To The Simpler,
// Older System-DPI-Aware API There, Which Still Fixes The Bug (Just
// Without Live-Adjusting If Different Monitors Use Different Scaling).
//////////////////////////////////////////////////////////////////////

static void EnableDpiAwareness(void)
{
	HMODULE hUser32 = LoadLibrary("user32.dll");
	if (hUser32)
	{
		typedef BOOL (WINAPI *SetProcessDpiAwarenessContextFn)(HANDLE);
		SetProcessDpiAwarenessContextFn pSetCtx =
			(SetProcessDpiAwarenessContextFn)GetProcAddress(hUser32, "SetProcessDpiAwarenessContext");
		if (pSetCtx)
		{
			// DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2, By Its Documented
			// Numeric Value -- Spelled Out Rather Than Named So This Doesn't
			// Depend On A Newer Windows SDK Actually Declaring The Symbol.
			HANDLE perMonitorV2 = (HANDLE)(-4);
			if (pSetCtx(perMonitorV2))
			{
				FreeLibrary(hUser32);
				return;												// Best Case -- Per-Monitor Awareness Set
			}
		}
		FreeLibrary(hUser32);
	}

	SetProcessDPIAware();											// Available Since Vista -- Simple Fallback
}

//////////////////////////////////////////////////////////////////////
// Command Line Parsing
//
// Windows invokes a screensaver with a leading '/' switch (occasionally
// '-'): /s to run, /c[:<owner hwnd>] to configure, /p <preview hwnd> to
// draw the settings-dialog thumbnail. The HWND for /p and /c has shown
// up both as a separate token and glued directly onto the switch across
// different Windows versions, so this accepts either.
//////////////////////////////////////////////////////////////////////

static RunMode ParseCommandLine(LPSTR lpCmdLine, HWND *outHwnd)
{
	*outHwnd = NULL;

	char *p = lpCmdLine;
	while (*p==' ' || *p=='\t')
		p++;

	if (*p=='\0')
		return MODE_WINDOWED;					// No switch -- dev/test convenience

	if (*p=='/' || *p=='-')
		p++;

	char c = *p;
	if (c)
		p++;

	switch (c)
	{
	case 's': case 'S':
		return MODE_SAVER;

	case 'p': case 'P':
		while (*p==':' || *p==' ' || *p=='\t')
			p++;
		*outHwnd = (HWND)(ULONG_PTR)atol(p);
		return MODE_PREVIEW;

	case 'c': case 'C':
	case 'a': case 'A':					// Legacy "change password" switch --
											// not applicable on modern Windows
											// (the OS handles the lock screen
											// itself), treated the same as /c.
		while (*p==':' || *p==' ' || *p=='\t')
			p++;
		if (*p)
			*outHwnd = (HWND)(ULONG_PTR)atol(p);
		return MODE_CONFIG;
	}

	return MODE_WINDOWED;
}

//////////////////////////////////////////////////////////////////////
// Shared Window Procedure
//
// Used by every window this app creates (one per monitor in /s mode,
// one child window in /p mode, one normal window in windowed mode).
// g_exitOnInput controls whether activity actually ends the process --
// it's only true during a real /s run.
//////////////////////////////////////////////////////////////////////

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_MOUSEMOVE:
		{
			// Windows can post a synthetic first WM_MOUSEMOVE right as a
			// fullscreen window is created, which shouldn't itself count
			// as "the user moved the mouse". Record it as a baseline
			// instead, then only exit once the cursor has moved a real
			// distance away from that baseline.
			POINT pt;
			GetCursorPos(&pt);
			if (!g_haveInitialMouse)
			{
				g_initialMouse = pt;
				g_haveInitialMouse = true;
			}
			else if (g_exitOnInput)
			{
				int dx = pt.x - g_initialMouse.x;
				int dy = pt.y - g_initialMouse.y;
				if (dx*dx + dy*dy > MOUSE_MOVE_THRESHOLD*MOUSE_MOVE_THRESHOLD)
					PostQuitMessage(0);
			}
			return 0;
		}

		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_KEYDOWN:
			if (g_exitOnInput)
				PostQuitMessage(0);
			return 0;

		case WM_SYSCOMMAND:
			switch (wParam & 0xFFF0)
			{
				case SC_SCREENSAVE:				// Don't let Windows try to
				case SC_MONITORPOWER:				// launch another screensaver
					return 0;						// or blank the monitor on us
			}
			break;

		case WM_SIZE:
			if (g_singleScene)
				g_singleScene->Resize(LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//////////////////////////////////////////////////////////////////////
// GL Context Setup/Teardown
//////////////////////////////////////////////////////////////////////

static BOOL SetupGLContext(HWND hWnd, HDC *pHDC, HGLRC *pHRC)
{
	static PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32,
		0, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		16,
		0,
		0,
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};

	HDC hDC = GetDC(hWnd);
	if (!hDC)
		return FALSE;

	int pf = ChoosePixelFormat(hDC, &pfd);
	if (!pf || !SetPixelFormat(hDC, pf, &pfd))
	{
		ReleaseDC(hWnd, hDC);
		return FALSE;
	}

	HGLRC hRC = wglCreateContext(hDC);
	if (!hRC || !wglMakeCurrent(hDC, hRC))
	{
		if (hRC)
			wglDeleteContext(hRC);
		ReleaseDC(hWnd, hDC);
		return FALSE;
	}

	*pHDC = hDC;
	*pHRC = hRC;
	return TRUE;
}

static void DestroyGLContext(HWND hWnd, HDC hDC, HGLRC hRC)
{
	wglMakeCurrent(NULL, NULL);
	if (hRC)
		wglDeleteContext(hRC);
	if (hDC)
		ReleaseDC(hWnd, hDC);
}

//////////////////////////////////////////////////////////////////////
// Frame Pacing
//
// The animation in GLWnd/Casket/Pumpkin advances by a fixed amount
// each time Render() is called -- there's no delta-time anywhere, so
// the apparent speed of the scene is entirely a function of how often
// the loop below calls Render(). With no vsync and nothing throttling
// it, a modern GPU calls it far more often per second than anything
// available when this was written, so the whole scene visibly speeds
// up. Capping to a fixed rate restores the original pace regardless
// of how fast the video card can actually push frames.
//
// Tweak TARGET_FPS if it still looks too fast/slow on your hardware.
//////////////////////////////////////////////////////////////////////

static const DWORD	TARGET_FPS = 30;
static const DWORD	FRAME_INTERVAL_MS = 1000 / TARGET_FPS;

static void PaceFrame(DWORD *lastTick)
{
	DWORD now = GetTickCount();
	DWORD elapsed = now - *lastTick;
	if (elapsed < FRAME_INTERVAL_MS)
		Sleep(FRAME_INTERVAL_MS - elapsed);
	*lastTick = GetTickCount();
}

//////////////////////////////////////////////////////////////////////
// /s -- Run As The Screensaver (Every Monitor)
//////////////////////////////////////////////////////////////////////

static BOOL CALLBACK MonitorEnumProc(HMONITOR hMon, HDC hdcMon, LPRECT rect, LPARAM lParam)
{
	if (g_monCount < MAX_GLWEEN_MONITORS)
	{
		g_mon[g_monCount].rect = *rect;
		g_monCount++;
	}
	return TRUE;
}

static BOOL CreateMonitorWindow(int index)
{
	const RECT &rect = g_mon[index].rect;

	HWND hWnd = CreateWindowEx(
		WS_EX_APPWINDOW | WS_EX_TOPMOST,
		WND_CLASS_NAME, "GLween Screensaver",
		WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
		rect.left, rect.top, rect.right-rect.left, rect.bottom-rect.top,
		NULL, NULL, g_hInstance, NULL);
	if (!hWnd)
		return FALSE;

	HDC hDC; HGLRC hRC;
	if (!SetupGLContext(hWnd, &hDC, &hRC))
	{
		DestroyWindow(hWnd);
		return FALSE;
	}

	GLWnd *scene = new GLWnd();
	scene->m_playAudio = (index == 0);				// Only one monitor plays thunder
	scene->Resize(rect.right-rect.left, rect.bottom-rect.top);

	if (!scene->Init())
	{
		delete scene;
		DestroyGLContext(hWnd, hDC, hRC);
		DestroyWindow(hWnd);
		return FALSE;
	}

	g_mon[index].hWnd = hWnd;
	g_mon[index].hDC = hDC;
	g_mon[index].hRC = hRC;
	g_mon[index].scene = scene;

	ShowWindow(hWnd, SW_SHOW);
	return TRUE;
}

static int RunSaver(void)
{
	g_monCount = 0;
	EnumDisplayMonitors(NULL, NULL, MonitorEnumProc, 0);

	if (g_monCount == 0)							// Extremely unlikely fallback
	{
		g_mon[0].rect.left = 0;
		g_mon[0].rect.top = 0;
		g_mon[0].rect.right = GetSystemMetrics(SM_CXSCREEN);
		g_mon[0].rect.bottom = GetSystemMetrics(SM_CYSCREEN);
		g_monCount = 1;
	}

	int created = 0;
	for (int i=0; i<g_monCount; i++)
	{
		g_mon[i].hWnd = NULL;
		g_mon[i].hDC = NULL;
		g_mon[i].hRC = NULL;
		g_mon[i].scene = NULL;
		if (CreateMonitorWindow(i))
			created++;
	}

	MSG msg;
	msg.wParam = 0;

	if (created > 0)
	{
		SetForegroundWindow(g_mon[0].hWnd);
		SetFocus(g_mon[0].hWnd);
		ShowCursor(FALSE);
		g_cursorHidden = true;
		g_exitOnInput = true;

		DWORD lastFrameTick = GetTickCount();

		BOOL done = FALSE;
		while (!done)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					done = TRUE;
				else
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			else
			{
				for (int i=0; i<g_monCount; i++)
				{
					if (!g_mon[i].scene)
						continue;
					wglMakeCurrent(g_mon[i].hDC, g_mon[i].hRC);
					g_mon[i].scene->Render();
					SwapBuffers(g_mon[i].hDC);
				}
				PaceFrame(&lastFrameTick);
			}
		}
	}

	if (g_cursorHidden)
	{
		ShowCursor(TRUE);
		g_cursorHidden = false;
	}

	for (int i=0; i<g_monCount; i++)
	{
		if (g_mon[i].scene)
		{
			delete g_mon[i].scene;
			g_mon[i].scene = NULL;
		}
		if (g_mon[i].hWnd)
		{
			DestroyGLContext(g_mon[i].hWnd, g_mon[i].hDC, g_mon[i].hRC);
			DestroyWindow(g_mon[i].hWnd);
		}
	}

	return (int)msg.wParam;
}

//////////////////////////////////////////////////////////////////////
// /p <HWND> -- Live Preview For The Screen Saver Settings Dialog
//////////////////////////////////////////////////////////////////////

static int RunPreview(HWND hwndParent)
{
	if (!hwndParent || !IsWindow(hwndParent))
		return 0;

	// The Settings Dialog Can Hand Us The Preview Control's HWND Before
	// It's Actually Finished Laying Itself Out, So GetClientRect() Here
	// Can Genuinely Come Back 0x0 For A Moment. Rather Than Giving Up
	// (Which Just Leaves The Thumbnail Permanently Black), Fall Back To
	// A Placeholder Size Now And Keep Picking Up The Real Size Once It's
	// Available, In The Render Loop Below.
	RECT rc;
	GetClientRect(hwndParent, &rc);
	int width  = (rc.right  > 0) ? rc.right  : 1;
	int height = (rc.bottom > 0) ? rc.bottom : 1;

	HWND hWnd = CreateWindowEx(0, WND_CLASS_NAME, "", WS_CHILD | WS_VISIBLE,
		0, 0, width, height, hwndParent, NULL, g_hInstance, NULL);
	if (!hWnd)
		return 0;

	HDC hDC; HGLRC hRC;
	if (!SetupGLContext(hWnd, &hDC, &hRC))
	{
		DestroyWindow(hWnd);
		return 0;
	}

	GLWnd *scene = new GLWnd();
	scene->m_playAudio = false;					// Never play sound for the tiny thumbnail
	scene->Resize(width, height);

	g_exitOnInput = false;

	if (scene->Init())
	{
		g_singleScene = scene;

		MSG msg;
		BOOL done = FALSE;
		while (!done)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					done = TRUE;
				else
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			else if (!IsWindow(hwndParent))		// The settings dialog was closed
			{
				done = TRUE;
			}
			else
			{
				// Keep Checking The Parent's Real Client Size -- It Can
				// Start Out At 0x0 (Or Simply Change) While The Dialog
				// Is Still Settling, So Pick It Up As Soon As It's Real.
				RECT curRc;
				GetClientRect(hwndParent, &curRc);
				if (curRc.right > 0 && curRc.bottom > 0 &&
					(curRc.right != width || curRc.bottom != height))
				{
					width = curRc.right;
					height = curRc.bottom;
					SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_NOZORDER | SWP_NOMOVE);
					scene->Resize(width, height);
				}

				// Just a thumbnail -- no need to run full speed while it
				// sits in the background of the settings dialog.
				wglMakeCurrent(hDC, hRC);
				scene->Render();
				SwapBuffers(hDC);
				Sleep(66);
			}
		}

		g_singleScene = NULL;
	}

	delete scene;
	DestroyGLContext(hWnd, hDC, hRC);
	DestroyWindow(hWnd);
	return 0;
}

//////////////////////////////////////////////////////////////////////
// /c [<HWND>] -- Configuration Dialog
//
// There's nothing to configure -- the original demo has no adjustable
// settings -- so this just satisfies the convention that /c shows
// *something*.
//////////////////////////////////////////////////////////////////////

static int RunConfig(HWND hwndOwner)
{
	MessageBox(hwndOwner,
		"GLween Screensaver\n\n"
		"Original OpenGL demo by Jim Strong, written for the NeHe Productions\n"
		"2000 Halloween contest (\"Xersist's Happy Halloween\").\n\n"
		"No configurable options.",
		"GLween Screensaver",
		MB_OK | MB_ICONINFORMATION);
	return 0;
}

//////////////////////////////////////////////////////////////////////
// (No Args) -- Ordinary Window, For Testing From The Debugger
//
// Windows itself never launches a screensaver this way -- it always
// passes /s, /c, or /p -- so this path exists purely so F5 in Visual
// Studio does something useful without fiddling with debug arguments.
//////////////////////////////////////////////////////////////////////

static int RunWindowed(void)
{
	RECT rect = {0, 0, 800, 600};
	DWORD style = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	DWORD exStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	AdjustWindowRectEx(&rect, style, FALSE, exStyle);

	HWND hWnd = CreateWindowEx(exStyle, WND_CLASS_NAME,
		"GLween Screensaver (Test Window -- Esc To Quit)",
		style,
		CW_USEDEFAULT, CW_USEDEFAULT, rect.right-rect.left, rect.bottom-rect.top,
		NULL, NULL, g_hInstance, NULL);
	if (!hWnd)
		return 0;

	HDC hDC; HGLRC hRC;
	if (!SetupGLContext(hWnd, &hDC, &hRC))
	{
		DestroyWindow(hWnd);
		return 0;
	}

	GLWnd *scene = new GLWnd();
	scene->m_playAudio = true;
	scene->Resize(800, 600);

	g_exitOnInput = false;

	MSG msg;
	msg.wParam = 0;

	if (scene->Init())
	{
		g_singleScene = scene;

		ShowWindow(hWnd, SW_SHOW);
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);

		DWORD lastFrameTick = GetTickCount();

		BOOL done = FALSE;
		while (!done)
		{
			if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
			{
				if (msg.message == WM_QUIT)
					done = TRUE;
				else
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			}
			else if (GetAsyncKeyState(VK_ESCAPE) & 0x8000)
			{
				done = TRUE;
			}
			else
			{
				wglMakeCurrent(hDC, hRC);
				scene->Render();
				SwapBuffers(hDC);
				PaceFrame(&lastFrameTick);
			}
		}

		g_singleScene = NULL;
	}

	delete scene;
	DestroyGLContext(hWnd, hDC, hRC);
	DestroyWindow(hWnd);
	return (int)msg.wParam;
}

//////////////////////////////////////////////////////////////////////
// Entry Point
//////////////////////////////////////////////////////////////////////

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	EnableDpiAwareness();											// Before Any Window Is Created

	g_hInstance = hInstance;

	WNDCLASS wc;
	ZeroMemory(&wc, sizeof(wc));
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc		= WndProc;
	wc.hInstance		= hInstance;
	wc.hIcon			= LoadIcon(NULL, IDI_WINLOGO);
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= NULL;
	wc.lpszClassName	= WND_CLASS_NAME;

	if (!RegisterClass(&wc))
		return 0;

	HWND hwndParam = NULL;
	RunMode mode = ParseCommandLine(lpCmdLine, &hwndParam);

	int result = 0;
	switch (mode)
	{
		case MODE_SAVER:    result = RunSaver(); break;
		case MODE_PREVIEW:  result = RunPreview(hwndParam); break;
		case MODE_CONFIG:   result = RunConfig(hwndParam); break;
		case MODE_WINDOWED: result = RunWindowed(); break;
	}

	UnregisterClass(WND_CLASS_NAME, hInstance);
	return result;
}
