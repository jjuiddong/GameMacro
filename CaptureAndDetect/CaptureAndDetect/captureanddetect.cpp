
#include "../../../Common/Common/common.h"
#include "../../../Common/CamCommon/camcommon.h"
#include "../../../Common/Network/network.h"

// server packet
// 1 : capture and detect
// 2 : read match script
// 3 : set excute label
// 4 : set capture delay milliseconds
//

using namespace std;
using namespace cv;


struct sPacket
{
	int protocol;
	int result;
	int elpasedTime1;
	int elpasedTime2;
	char str[64];
};


void main(int argc, char *argv[])
{
	// server bind port
	if (argc < 2)
	{
		cout << "argument <server bind port>" << endl;
		return;
	}

	const int bindPort = atoi(argv[1]);

	dbg::RemoveErrLog();
	dbg::RemoveLog();

	network::cTCPServer server;
	if (server.Init(bindPort))
	{
		cout << "success server bind port=" << bindPort << endl;
	}
	else
	{
		cout << "fail server bind" << endl;
		return;
	}

	cvproc::imagematch::cMatchScript matchScript;
	const string matchScriptFileName = "dirt3_trackname.txt";
	if (matchScript.Read(matchScriptFileName))
	{
		cout << "success read match script" << endl;
	}
	else
	{
		cout << "match script read error" << endl;
		return;
	}

	cvproc::cScreenCapture capture;
	capture.Init();
	string executeLabel = "detect_trackname_roi";

	cout << "default match script = " << matchScriptFileName << endl;
	cout << "default execute label = " << executeLabel << endl;

	while (1)
	{
		network::sSockBuffer packet;
		if (server.m_recvQueue.Front(packet))
		{
			if (packet.readLen >= 4)
			{
				const int protocol = *(int*)packet.buffer;
				if (protocol == 1)
				{
					cout << "capture and detect" << endl;
					const int t0 = timeGetTime();
					Mat img = capture.ScreenCapture(true);
					if (img.data)
					{
						Rect roi = (img.cols >= 5760)? Rect(1920, 0, 1920, 1080) : Rect(0,0,img.cols, img.rows);
						cvtColor(img(roi), img, CV_BGR2GRAY);
						const int t1 = timeGetTime();
						matchScript.ExecuteEx(executeLabel, "", img, true);
						const int t2 = timeGetTime();
						cout << "executeex = " << matchScript.m_threadArg.resultStr.c_str() << endl;
						cout << "    - elapsed time = " << t2 - t1 << endl;

						sPacket data;
						ZeroMemory(&data, sizeof(data));
						data.protocol = 1;
						data.result = 1;
						data.elpasedTime1 = t2 - t1;
						data.elpasedTime2 = t2 - t0;
						strcpy_s(data.str, matchScript.m_threadArg.resultStr.c_str());
						server.m_sendQueue.Push(packet.sock, (BYTE*)&data, sizeof(data));
					}
					else
					{
						cout << "error capture" << endl;

						sPacket data;
						ZeroMemory(&data, sizeof(data));
						data.protocol = 1;
						data.result = 0;
						server.m_sendQueue.Push(packet.sock, (BYTE*)&data, sizeof(data));
					}
				}
				else if (protocol == 2) // 2 : read match script
				{
					char *ptr = (char*)(packet.buffer + sizeof(protocol));
					string fileName = ptr;
					cout << "read match script = " << fileName << endl;
					if (matchScript.Read(fileName))
					{
						cout << "success read match script " << endl;

						sPacket data;
						ZeroMemory(&data, sizeof(data));
						data.protocol = 2;
						data.result = 1;
						strcpy_s(data.str, fileName.c_str());
						server.m_sendQueue.Push(packet.sock, (BYTE*)&data, sizeof(data));
					}
					else
					{
						cout << "fail read match script " << endl;

						sPacket data;
						ZeroMemory(&data, sizeof(data));
						data.protocol = 2;
						data.result = 0;
						strcpy_s(data.str, fileName.c_str());
						server.m_sendQueue.Push(packet.sock, (BYTE*)&data, sizeof(data));
					}
				}
				else if (protocol == 3) // 3 : set excute label
				{
					char *ptr = (char*)(packet.buffer + sizeof(protocol));
					executeLabel = ptr;
					cout << "set execute label = " << executeLabel << endl;

					sPacket data;
					ZeroMemory(&data, sizeof(data));
					data.protocol = 3;
					data.result = 1;
					strcpy_s(data.str, executeLabel.c_str());
					server.m_sendQueue.Push(packet.sock, (BYTE*)&data, sizeof(data));
				}
				else if (protocol == 4) // 4 : set capture screen delay milliseconds
				{
					const int delayMilliseconds = *(int*)(packet.buffer + sizeof(protocol));
					cout << "set delay milliseconds = " << delayMilliseconds << endl;
					capture.m_prtScrSleepTime = delayMilliseconds;

					sPacket data;
					ZeroMemory(&data, sizeof(data));
					data.protocol = 4;
					data.result = 1;
					server.m_sendQueue.Push(packet.sock, (BYTE*)&data, sizeof(data));
				}
			}
			server.m_recvQueue.Pop();
		}
	}
}
