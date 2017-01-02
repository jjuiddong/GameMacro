
#include "../../../Common/Common/common.h"
#include "../../../Common/CamCommon/camcommon.h"
#include "../../../Common/Network/network.h"
#include <conio.h>

// server packet
// 1 : capture and detect
// 2 : read match script
// 3 : set execute label
//

using namespace std;
using namespace cv;

struct sPacket
{
	int protocol;
	int result;
	int elapsedTime1;
	int elapsedTime2;
	char str[64];
};


void main(int argc, char *argv[])
{
	// connect server
	// commandline argument
	// <ip> <port>
	cout << "1" << endl;

	if (argc != 3)
	{
		cout << "commandline <ip> <port>" << endl;
		getchar();
		return;
	}

	cout << "2" << endl;

	const string ip = argv[1];
	const int port = atoi(argv[2]);

	network::cTCPClient client;
	if (client.Init(ip, port))
	{
		cout << "success connect" << endl;
	}
	else
	{
		cout << "fail connect server" << endl;
		getchar();
		return;
	}

	cout << "3" << endl;

	cout << "-----------------------------------------" << endl;
	cout << "0 : exit" << endl;
	cout << "1 : capture and detect " << endl;
	cout << "2 : read image match script " << endl;
	cout << "3 : set execute label " << endl;
	cout << "-----------------------------------------" << endl;


	while (1)
	{

		while (!_kbhit())
		{
			network::sSockBuffer data;
			if (client.m_recvQueue.Front(data))
			{
				cout << "recv >>" << endl;
				sPacket *packet = (sPacket*)data.buffer;
				switch (packet->protocol)
				{
				case 1:
					cout << "    result = " << packet->result << endl;
					cout << "    elapsedTime1 = " << packet->elapsedTime1 << endl;
					cout << "    elapsedTime2 = " << packet->elapsedTime2 << endl;
					cout << "    str = " << packet->str << endl;
					break;
				case 2:
					cout << "    result = " << packet->result << endl;
					cout << "    str = " << packet->str << endl;
					break;
				case 3:
					cout << "    result = " << packet->result << endl;
					cout << "    str = " << packet->str << endl;
					break;
				case 4:
					cout << "    result = " << packet->result << endl;
					break;
				}
				client.m_recvQueue.Pop();

				//break; // out of network recv process
			}
		}


		char c;
		cin >> c;
		if (c == '1')
		{
			cout << "capture and detect" << endl;
			int data = 1;
			client.Send((BYTE*)&data, sizeof(data));
		}
		else if (c == '2')
		{
			cout << "input image match script name " << endl;
			string fileName;
			cin >> fileName;

			char buff[128];
			*(int*)buff = 2;
			strcpy(buff+sizeof(int), fileName.c_str());
			client.Send((BYTE*)buff, sizeof(buff));
		}
		else if (c == '3')
		{
			cout << "input execute label name " << endl;
			string labelName;
			cin >> labelName;

			char buff[128];
			*(int*)buff = 3;
			strcpy(buff + sizeof(int), labelName.c_str());
			client.Send((BYTE*)buff, sizeof(buff));
		}
		else if (c == '4')
		{
			cout << "input capture delay milliseconds " << endl;
			int delayMilliseconds;
			cin >> delayMilliseconds;

			char buff[128];
			*(int*)buff = 4;
			*(int*)(buff+sizeof(int)) = delayMilliseconds;
			
			client.Send((BYTE*)buff, sizeof(buff));
		}
		else if (c == '?')
		{
			cout << "-----------------------------------------" << endl;
			cout << "0 : exit" << endl;
			cout << "1 : capture and detect " << endl;
			cout << "2 : read image match script " << endl;
			cout << "3 : set execute label " << endl;
			cout << "-----------------------------------------" << endl;
		}
		else if (c == '0')
		{
			break;
		}


	}



}
