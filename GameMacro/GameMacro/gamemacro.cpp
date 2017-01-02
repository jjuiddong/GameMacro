//
// 2016-06-13, jjuiddong
// 영상을 인식해서, 게임을 자동으로 실행되게 한다.
//
// macro_config 내용
// 
// bindport = 8000
// matchscript_scene = scriptname
// matchscript_track = scriptname
// menu_script = scriptname
// 
//
// network protocol
//
//	Goto Menu
//		protocol = 1
//		string = menu name
//
// Read Config
//		protocol = 2
//		string = filename
//
//
//
// Exit
//		protocol = 100
//
//
#include "../../../Common/Common/common.h"
#include "../../../Common/CamCommon/camcommon.h"
#include "../../../Common/Network/network.h"
#include <shlwapi.h>


#pragma comment (lib, "winmm.lib")

using namespace std;
using namespace cvproc;
using namespace cvproc::imagematch;
using namespace cv;

struct sPacket
{
	int protocol;
	char str[256];
	int result;
};

bool KeyDown(const int vkey);
void PrintVirtualKey(const int vkey, const int tab = 0, const string &str = ""); //tab=0, str=""
void NetworkProc();

cFlowControl g_flowControl;
cMatchScript2 trackMatchScript;
cvproc::cFrapsCapture frapsCap;
network::cTCPServer svr;
int g_nextKey = 0;
string sceneMatchScriptFileName;
string trackMatchScriptFileName;
string menuScriptName;
bool isLoop = true;
bool g_isDbgCapture = false;
#define CAPTURE_KEY  (VK_F9)


void main(int argc, char *argv[])
{
	if (argc != 2)
	{
		cout << "commandline <config filename>" << endl;
		getchar();
		return;
	}

	dbg::RemoveErrLog();
	dbg::RemoveLog();
	CreateDirectoryA("detect", NULL);

	const string fileName = argv[1];

	cout << "Read Config File << " << fileName << endl << endl;
	cConfig config;
	if (!config.Read(fileName))
	{
		cout << "Error!! read config file = " << fileName << endl;
		return;
	}

	const int bindPort = config.GetInt("bindport", 10000);
	sceneMatchScriptFileName = config.GetString("matchscript_scene");
	trackMatchScriptFileName = config.GetString("matchscript_track");
	menuScriptName = config.GetString("menu_script");

	if (trackMatchScript.Read(trackMatchScriptFileName))
	{
		cout << "Success Read track match script " << trackMatchScriptFileName << endl;
	}
	else
	{
		cout << "Fail Read track match script " << trackMatchScriptFileName << endl;
		return;
	}

	if (svr.Init(bindPort))
	{
		cout << "success bind server port = " << bindPort << endl;
	}
	else
	{
		cout << "Faile bind server port = " << bindPort << endl;
		return;
	}

	if (g_flowControl.Init(sceneMatchScriptFileName, menuScriptName, CAPTURE_KEY))
	{
		cout << "Success Init Scene Traverse << " << sceneMatchScriptFileName  << " << " << menuScriptName << endl;
	}
	else
	{
		cout << "Fail Init Scene Traverse << " << sceneMatchScriptFileName << " << " << menuScriptName << endl;
		return;
	}
	g_flowControl.m_isLog = true;

	frapsCap.Init("C:/Fraps/Screenshots/", CAPTURE_KEY);
	frapsCap.m_keyDownDelay = 30;

	dbg::Log("Start GameMacro \n");

	bool capture = false;
	Mat img;
	int oldT = timeGetTime();
	while (isLoop)
	{
		const int curT = timeGetTime();
		const int elapseT = curT - oldT;
		if (elapseT < 10)
			continue;
		oldT = curT;
		const float deltaSeconds = elapseT * 0.001f;

		NetworkProc();

		int vkey = 0;
		g_flowControl.Update(deltaSeconds, img, vkey);
		if (img.data)
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
				imwrite(fileName.c_str(), img);
			}

			img = Mat();
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
			capture = true;
			frapsCap.ScreenShot();
			break;

		default:
			KeyDown(vkey);
			break;
		}

		if (capture && frapsCap.IsUpdateScreenShot())
		{
			capture = false;
			if (frapsCap.m_img.data)
			{
				img = frapsCap.m_img;
				frapsCap.m_img = Mat();
			}
			else
			{
				capture = true;
				frapsCap.ScreenShot();
			}
		}

		Sleep(10);
	}
}


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
			dbg::Log(g_isDbgCapture? "Debug Capture On \n" : "Debug Capture Off \n");
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

	dbg::Log("%s\n", ss.str().c_str());
}
