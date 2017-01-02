#include "stdafx.h"
#include "Flow.h"


using namespace cvproc;
using namespace cvproc::imagematch;
using namespace cv;


const int g_Width = 300;
const int g_Height = 300;
HINSTANCE hInst;
HWND g_hListBox = NULL;

cFlowControl g_flowControl;
cvproc::cFrapsCapture frapsCap;
network::cTCPServer svr;
int g_nextKey = 0;
string sceneMatchScriptFileName;
string trackMatchScriptFileName;
string menuScriptName;
bool isLoop = true;
bool g_isDbgCapture = true;
bool g_isCapture = false;
Mat g_img;
bool g_isLoop = true;

#define CAPTURE_KEY  (VK_F9)

std::thread g_initFlowControlThread;
int g_stateInitFlowControl = 0;// 0=init, 1=success, 2=fail, 3=complete

enum STATE {
	INIT,
	START,
	ERR,
};
STATE g_state = INIT;

struct sPacket
{
	int protocol;
	char str[256];
	int result;
};

struct sPacket2
{
	char str[256];
};


BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK  WndProc(HWND, UINT, WPARAM, LPARAM);
void MainLoop();
void Log(const char* fmt, ...);
bool InitFlow(const string &configFileName);
bool KeyDown(const int vkey);
void PrintVirtualKey(const int vkey, const int tab = 0, const string &str = ""); //tab=0, str=""
void NetworkProc();
void NetworkProc2();


int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    if (!InitInstance (hInstance, nCmdShow))
        return FALSE;

	InitFlow("macro_config.txt");

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_FLOW));

	MSG msg;
	while (g_isLoop)
    {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		switch (g_stateInitFlowControl)
		{
		case 1: 
			g_state = START;
			g_stateInitFlowControl = 3;
			Log("Start Flow");
			break;

		case 2: 
			g_state = ERR;
			g_stateInitFlowControl = 3;
			Log("Stop");
			break;

		default:
			break;
		}

		MainLoop();
		Sleep(1);
    }

	if (g_initFlowControlThread.joinable())
		g_initFlowControlThread.join(); // 쓰레드 종료
	cMatchProcessor::Release();

    return (int) msg.wParam;
}


BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance;

   const int screenW = GetSystemMetrics(SM_CXSCREEN);
   const int screenH = GetSystemMetrics(SM_CYSCREEN);

   WNDCLASSEXW wcex;
   wcex.cbSize = sizeof(WNDCLASSEX);
   wcex.style = CS_HREDRAW | CS_VREDRAW;
   wcex.lpfnWndProc = WndProc;
   wcex.cbClsExtra = 0;
   wcex.cbWndExtra = 0;
   wcex.hInstance = hInstance;
   wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_FLOW));
   wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
   wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
   wcex.lpszMenuName = NULL;
   wcex.lpszClassName = L"FLOW";
   wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
   RegisterClassExW(&wcex);

   HWND hWnd = CreateWindowW(L"FLOW", L"Flow", WS_OVERLAPPEDWINDOW,
	   screenW/2 - g_Width/2, screenH/2 - g_Height/2, g_Width, g_Height, 
	   nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
      return FALSE;

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

enum {
	ID_LISTBOX  = 100,
};

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_CREATE:
		{
			g_hListBox = CreateWindow(L"listbox", NULL, WS_CHILD | WS_VISIBLE // | WS_BORDER 
				| LBS_NOTIFY, 0, 0, g_Width, g_Height, hWnd, (HMENU)ID_LISTBOX, hInst, NULL);
		}
		break;

   case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;

   case WM_SIZE:
	   {
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			MoveWindow(g_hListBox, 0, 0, width, height, TRUE);
	   }
	   break;

   case WM_CLOSE:
	   g_isLoop = false;
	   break;

    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void Log(const char* fmt, ...)
{
	char textString[256] = { '\0' };
	va_list args;
	va_start(args, fmt);
	vsnprintf_s(textString, sizeof(textString), _TRUNCATE, fmt, args);
	va_end(args);

// 	m_logList.AddString(str2wstr(textString).c_str());
// 	m_logList.SetCurSel(m_logList.GetCount() - 1);
	SendMessage(g_hListBox, LB_ADDSTRING, 0, (LPARAM)str2wstr(textString).c_str());
}


void InitFlowControl()
{
	if (g_flowControl.Init(sceneMatchScriptFileName, menuScriptName, CAPTURE_KEY))
	{
		Log("Success Init Flow Control << %s << %s ", sceneMatchScriptFileName.c_str(), menuScriptName.c_str());
		g_stateInitFlowControl = 1;
	}
	else
	{
		Log("Fail Init Flow Control << %s << %s ", sceneMatchScriptFileName.c_str(), menuScriptName.c_str());
		g_stateInitFlowControl = 2;
		return;
	}
	g_flowControl.m_isLog = true;
}


bool InitFlow(const string &configFileName)
{
	dbg::RemoveErrLog();
	dbg::RemoveLog();
	CreateDirectoryA("detect", NULL);

	Log("Read Config File << %s", configFileName.c_str());
	cConfig config;
	if (!config.Read(configFileName))
	{
		Log("Error!! read config file ");
		return false;
	}

	const int bindPort = config.GetInt("bindport", 10000);
	sceneMatchScriptFileName = config.GetString("matchscript_scene");
	trackMatchScriptFileName = config.GetString("matchscript_track");
	menuScriptName = config.GetString("menu_script");

	if (svr.Init(bindPort))
	{
		Log("success bind server port = %d ", bindPort);
	}
	else
	{
		Log("Fail bind server port = %d", bindPort);
		return false;
	}

	frapsCap.Init("C:/Fraps/Screenshots/", CAPTURE_KEY);
	frapsCap.m_keyDownDelay = 30;

	// FlowControl 은 초기화가 오래걸리기 때문에 쓰레드로 처리한다.
	Log("Init Flow Control Wait... ");
	g_initFlowControlThread = std::thread(InitFlowControl);

	return true;
}


// string protocol
// -gotomenu; ~~~
// - readconfig; ~~
// - stop; ~~~
// - capture;
// - quit;
void NetworkProc2()
{
	network::sSockBuffer packet;
	if (!svr.m_recvQueue.Front(packet))
		return;

	dbg::Log("recv packet >> \n");

	const sPacket2 *data = (sPacket2*)packet.buffer;
	vector<string> strs;
	common::tokenizer(data->str, "<<", "", strs);
	if (strs.empty())
		goto exit;

	const char *prt[] = { "gotomenu", "readconfig", "stop", "capture", "quit" };
	const int prtSize = sizeof(prt) / sizeof(char*);
	int protocol = -1;
	for (int i = 0; i < prtSize; ++i)
	{
		if (strs[0] == prt[i])
		{
			protocol = i;
			break;
		}
	}
	if (protocol == -1)
		goto exit;

	switch (protocol)
	{
	case 0: //	Goto Menu
		if (strs.size() <= 1)
			goto exit;

		g_flowControl.CommandStr(strs[1]);
		dbg::Log("Goto Scene << %s \n", data->str);
		break;

	case 1: // Read Config
		dbg::Log("Read Config \n");

		if (g_flowControl.Init(sceneMatchScriptFileName, menuScriptName, CAPTURE_KEY))
		{
			dbg::Log("Success Init Scene Traverse << %s << %s \n", sceneMatchScriptFileName.c_str(), menuScriptName.c_str());
		}
		else
		{
			dbg::Log("Fail Init Scene Traverse << %s << %s \n", sceneMatchScriptFileName.c_str(), menuScriptName.c_str());
		}
		break;

	case 2: // stop
		dbg::Log("Stop \n");
		g_flowControl.Cancel();
		break;

	case 3: // debug capture
		g_isDbgCapture = !g_isDbgCapture;
		dbg::Log(g_isDbgCapture ? "Debug Capture On \n" : "Debug Capture Off \n");
		break;

	case 4:
		isLoop = false;
		break;
	}

exit:
	svr.m_recvQueue.Pop();
}


// binary protocol
void NetworkProc()
{
	network::sSockBuffer packet;
	if (svr.m_recvQueue.Front(packet))
	{
		dbg::Log("recv packet >> \n");

		const sPacket  *data = (sPacket*)packet.buffer;
		switch (data->protocol)
		{
		case 1: //	Goto Menu
			g_flowControl.CommandStr(data->str);
			dbg::Log("Goto Scene << %s \n", data->str);
			break;

		case 2: // Read Config
			dbg::Log("Read Config \n");

			if (g_flowControl.Init(sceneMatchScriptFileName, menuScriptName, CAPTURE_KEY))
			{
				dbg::Log("Success Init Scene Traverse << %s << %s \n", sceneMatchScriptFileName.c_str(), menuScriptName.c_str());
			}
			else
			{
				dbg::Log("Fail Init Scene Traverse << %s << %s \n", sceneMatchScriptFileName.c_str(), menuScriptName.c_str());
			}
			break;

		case 3: // stop
			dbg::Log("Stop \n");
			g_flowControl.Cancel();
			break;

		case 4: // debug capture
			g_isDbgCapture = !g_isDbgCapture;
			dbg::Log(g_isDbgCapture ? "Debug Capture On \n" : "Debug Capture Off \n");
			break;

		case 100:
			isLoop = false;
			break;
		}

		svr.m_recvQueue.Pop();
	}
}


bool KeyDown(const int vkey)
{
	if (vkey <= 0)
		return false;

	PrintVirtualKey(vkey, 0, "keyDown >>");

	INPUT input;
	input.type = INPUT_KEYBOARD;
	input.ki.wScan = MapVirtualKey((WORD)vkey, MAPVK_VK_TO_VSC);
	input.ki.time = 0;
	input.ki.dwExtraInfo = 0;
	input.ki.wVk = (WORD)vkey;
	input.ki.dwFlags = KEYEVENTF_SCANCODE;
	SendInput(1, &input, sizeof(INPUT));

	Sleep(30);

	input.ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_SCANCODE;
	SendInput(1, &input, sizeof(INPUT));

	Sleep(30);

	return true;
}


void PrintVirtualKey(const int vkey, const int tab, const string &str) //tab=0, str=""
{
	std::stringstream ss;
	ss << str;

	for (int i = 0; i < tab; ++i)
		ss << "    ";

	switch (vkey)
	{
	case VK_SNAPSHOT: ss << "vkey = VK_SNAPSHOT"; break;
	case VK_UP: ss << "vkey = VK_UP"; break;
	case VK_DOWN: ss << "vkey = VK_DOWN"; break;
	case VK_LEFT: ss << "vkey = VK_LEFT"; break;
	case VK_RIGHT: ss << "vkey = VK_RIGHT"; break;
	case VK_F11: ss << "vkey = VK_11"; break;
	case VK_RETURN: ss << "vkey = VK_RETURN"; break;
	case VK_ESCAPE: ss << "vkey = VK_ESCAPE"; break;
	default:
		ss << vkey;
		break;
	}

	//Log("%s ", ss.str().c_str());
}


void MainLoop() 
{
	static int oldT = timeGetTime();
	const int curT = timeGetTime();
	const int elapseT = curT - oldT;
	if (elapseT < 10)
		return;
	oldT = curT;
	const float deltaSeconds = elapseT * 0.001f;

	RET(g_state != START);

	NetworkProc2();

	int vkey = 0;
	g_flowControl.Update(deltaSeconds, g_img, vkey);
	if (g_img.data)
	{
		if (g_isDbgCapture) // write file
		{
			static int imgCnt = 1;
			string fileName;
			do
			{
				std::stringstream ss;
				ss << "detect/img_capture" << imgCnt++ << ".jpg";
				fileName = ss.str();
			} while (PathFileExistsA(fileName.c_str()));
			imwrite(fileName.c_str(), g_img);
		}

		g_img = Mat();
	}


	static cFlowControl::STATE oldState = cFlowControl::STATE::ERR;
	if (oldState != g_flowControl.m_state)
	{
		dbg::Log("FlowControl State = %s\n", g_flowControl.GetStateString(g_flowControl.m_state).c_str());
		oldState = g_flowControl.m_state;
	}


	switch (vkey)
	{
	case CAPTURE_KEY:
		dbg::Log("screen capture \n");
		g_isCapture = true;
		frapsCap.ScreenShot();
		break;

	default:
		KeyDown(vkey);
		break;
	}

	if (g_isCapture && frapsCap.IsUpdateScreenShot())
	{
		g_isCapture = false;
		if (frapsCap.m_img.data)
		{
			g_img = frapsCap.m_img;
			frapsCap.m_img = Mat();
		}
		else
		{
			g_isCapture = true;
			frapsCap.ScreenShot();
		}
	}
}
