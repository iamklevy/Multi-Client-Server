#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#pragma comment (lib, "ws2_32.lib")


using namespace std;


void main() {


    //initalize winsock

	WSADATA wsData;
	WORD version = MAKEWORD(2, 2);


	int wsOK = WSAStartup(version, &wsData);   //INCL_WINSOCK_API_PROTOTYPES 
	if (wsOK != 0)
	{
		cerr << "can't initialize winsock! quitting!" << endl;
		return;
	}


	//create a socket  -> socket is  an endpoint (number) just to be attached with an ip & port

	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);

	if (listening == INVALID_SOCKET)
	{
		cerr << "can't create socket!" << endl;
		return;
	}

	//bind an ip address and port to a socket

	sockaddr_in hint;                             //The SOCKADDR_IN structure specifies a transport address and port for the AF_INET address family.
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);                //The htons function converts a u_short from host to TCP/IP network byte order (which is big-endian) -> port can be changed
	hint.sin_addr.S_un.S_addr = INADDR_ANY;     // "INADDR_ANY"  is just Definition of bits in internet address integers. ->   could also use inet_pton ....


	bind(listening, (sockaddr*)&hint, sizeof(hint));


	//tell winsock the socket is for listening 

	listen(listening, SOMAXCONN);   //"SOMAXCONN" is a Maximum queue length specifiable by listen.

	fd_set master;

	FD_ZERO(&master);

	FD_SET(listening, &master);

	while (true)
	{
		fd_set copy = master;

		int SoketCount = select(0, &copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < SoketCount; i++)
		{
			SOCKET sock = copy.fd_array[i];

			if (sock == listening)
			{
				//accept a new connection
				SOCKET client = accept(listening, nullptr, nullptr);
				//add the new connection to the list of connected clients
				FD_SET(client, &master);
				//send welcome message to the connected clients 
				string welcomeMSG = "welcome to the chat server!\r\n";
				send(client, welcomeMSG.c_str(), welcomeMSG.size() + 1, 0);

			}
			else
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				//Receive MSG
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0) {
					//drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					//send message to other clients, and definiately not the lestening socket

					for (int i = 0; i < master.fd_count; i++) {
						SOCKET outsock = master.fd_array[i];

						if (outsock == listening)
						{
							continue;
						}
						ostringstream ss;

						if (outsock != sock) {
							ss << "sock #" << sock << ": " << buf << "\r\n";
						}
						else {

							ss << "ME :" << buf << "\r\n";
						}
						string strout = ss.str();
						send(outsock, strout.c_str(), strout.size() + 1, 0);
					}



				}


			}

		}

	}

	FD_CLR(listening, &master);
	closesocket(listening);


	//cleanup winsock
	WSACleanup();

	system("pause");

}




