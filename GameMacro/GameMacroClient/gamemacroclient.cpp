
#include "../../../Common/Common/common.h"
#include "../../../Common/CamCommon/camcommon.h"
#include "../../../Common/Network/network.h"
#include <conio.h>


#pragma comment (lib, "winmm.lib")

using namespace std;
using namespace cvproc;
using namespace cvproc::imagematch;


struct sPacket
{
	int protocol;
	char str[256];
	int result;
};


void main(int argc, char *argv[])
{
	if (argc != 3)
	{
		cout << "commandline <ip> <port>" << endl;
		getchar();
		return;
	}

	const string ip = argv[1];
	const int port = atoi(argv[2]);

	network::cTCPClient client;
	if (client.Init(ip, port))
	{
		cout << "Success Connect Server " << ip << " " << port << endl;
	}
	else
	{
		cout << "Fail Connect Server " << ip << " " << port << endl;
		getchar();
		return;
	}

	cout << "-----------------------------------------" << endl;
	cout << "0 : exit" << endl;
	cout << "1 : move scene " << endl;
	cout << "2 : read script " << endl;
	cout << "3 : cancel " << endl;
	cout << "4 : move scene command " << endl;
	cout << "5 : write capture file " << endl;
	cout << "-----------------------------------------" << endl;

	while (client.IsConnect())
	{

		while (!_kbhit())
		{
			Sleep(100);

			network::sSockBuffer data;
			if (client.m_recvQueue.Front(data))
			{
				cout << "recv >>" << endl;
				sPacket *packet = (sPacket*)data.buffer;
				switch (packet->protocol)
				{
				case 1:
					cout << "    result = " << packet->result << endl;
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
					break;
				}
				client.m_recvQueue.Pop();

				//break; // out of network recv process
			}
		}

		char c;
		cin >> c;
		switch (c)
		{
		case '0': client.Close(); break;

		case '1':
		{
			sPacket packet;
			packet.protocol = 1;

			cout << "move scene" << endl;
			cout << "input scene name << " << endl;
			cin >> packet.str;

			client.Send((BYTE*)&packet, sizeof(packet));
		}
		break;

		case '2':
		{
			sPacket packet;
			packet.protocol = 2;
			client.Send((BYTE*)&packet, sizeof(packet));
		}
		break;

		case '3':
		{
			sPacket packet;
			packet.protocol = 3;
			client.Send((BYTE*)&packet, sizeof(packet));
		}
		break;

		case '4':
		{
			cout << "move scene" << endl;
			cout << "input command file << " << endl;
			string fileName;
			cin >> fileName;

			string cmds;
			ifstream ifs(fileName);
			if (ifs.is_open())
			{
				//stringstream ss;
				string line;
				while (getline(ifs, line))
				{
					trim(line);
					if (line.empty())
						continue;
					if (line[0] == '#')
						continue;

					if (!cmds.empty())
						cmds += ";";
					cmds += line;
				}
			}

			cout << cmds << endl;

 			sPacket packet;
 			packet.protocol = 1;
			strcpy(packet.str, cmds.c_str());
			client.Send((BYTE*)&packet, sizeof(packet));
		}
		break;

		case '5':
			sPacket packet;
			packet.protocol = 4;
			client.Send((BYTE*)&packet, sizeof(packet));
			break;
		}

		Sleep(100);
	}	

}
