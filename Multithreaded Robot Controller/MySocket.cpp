#include <string>
#include <iostream>
#include "MySocket.h"



//Constructor
MySocket::MySocket(SocketType socType, std::string ip, unsigned int port, ConnectionType conType, unsigned int length) {

	bConnect = false;
	connectionType = conType;
	this->IPAddr = ip;
	this->Port = port;
	this->mySocket = socType;
	WelcomeSocket = INVALID_SOCKET;
	ConnectionSocket = INVALID_SOCKET;

	//allocate memory to Buffer
	if (length > 0) {
		this->MaxSize = length;
	}
	else {
		this->MaxSize = DEFAULT_SIZE;
	}

	if (StartWSA() == true) {
		//allocate memory for buffer
		this->Buffer = new char[this->MaxSize];
	}
	else {
		//Assign it to safe empty state
		Buffer = nullptr;
	}

}



//Deconstructor
MySocket::~MySocket() {
	delete[] this->Buffer;
	closesocket(WelcomeSocket);
	closesocket(ConnectionSocket);
	WSACleanup();
}



//Configures DLLs
bool MySocket::StartWSA() {
	return(WSAStartup(MAKEWORD(2, 2), &this->wsaData) == 0);
}



//Connect TCP
bool MySocket::ConnectTCP() {

	bool connectionSuccessful = false;
	//Just in case checks if connection TYpe is TCP
	if (connectionType == TCP) {
		WelcomeSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		SvrAddr.sin_family = AF_INET;
		SvrAddr.sin_port = htons(Port);
		SvrAddr.sin_addr.s_addr = inet_addr(IPAddr.c_str());

		if (mySocket == CLIENT) {

			if ((connect(this->WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
				bConnect = false;
				closesocket(this->ConnectionSocket);
				connectionSuccessful = false;
				//WSACleanup();
				//std::cin.get();
				//exit(0);
			}
			else {
				bConnect = true;
				connectionSuccessful = true;
			}

		}
		else if (mySocket == SERVER) {
			bool cont = true;
			//Firstly bind
			if ((bind(WelcomeSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
				closesocket(WelcomeSocket);
				connectionSuccessful = false;
				cont = false;
			}

			//Secondly listen
			if (cont == true) {
				if (listen(WelcomeSocket, 1) == SOCKET_ERROR) {
					closesocket(WelcomeSocket);
					connectionSuccessful = false;
					cont = false;
				}
			}

			//Thirdly accept
			if (cont == true) {
				if ((this->ConnectionSocket = accept(this->WelcomeSocket, NULL, NULL)) == SOCKET_ERROR) {
					connectionSuccessful = false;
					closesocket(this->WelcomeSocket);
					//WSACleanup();
					//std::cin.get();
					//exit(0);
					cont = false;
				}
			}

			//Lastly connection successful
			if (cont == true) {
				bConnect = true;
				connectionSuccessful = true;
			}
		}
	}

	return connectionSuccessful;
}


//Disconnects TCP
bool MySocket::DisconnectTCP() {

	bool tcpDisconnected = false;
	//Both need to be set true as sockets usage varies based on SERVER or CLIENT
	bool connection = true;
	bool welcome = true;


	//The closesocket function returns a value of integer type that indicates whether the call was successful. 
	//A value of zero means that the call succeeded. A value of SOCKET_ERROR indicates that the called failed,
	if (connectionType == TCP) {
		if (mySocket == SERVER) {
			connection = (closesocket(ConnectionSocket) != SOCKET_ERROR);
			welcome = (closesocket(this->WelcomeSocket) != SOCKET_ERROR);
		}
		else if (mySocket == CLIENT) {
			welcome = (closesocket(this->WelcomeSocket) != SOCKET_ERROR);
		}

		//Checks if both are true
		if (welcome == true && connection == true) {
			//Disconnected
			bConnect = false;
			tcpDisconnected = true;
		}

	}

	return tcpDisconnected;
}



//Configures the UDP connection sockets for communication
bool MySocket::SetupUDP() {

	bool udpSETUP = false;
	bool cont = true;
	//Just in case checks if connection type is UDP
	if (connectionType == UDP) {
		ConnectionSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

		if (ConnectionSocket != INVALID_SOCKET) {

			if (mySocket == SERVER && cont == true) {
				SvrAddr.sin_family = AF_INET; //Address family type internet
				SvrAddr.sin_port = htons(this->Port); //port (host to network conversion)
				SvrAddr.sin_addr.s_addr = INADDR_ANY; //Anyone cna connect

				if ((bind(this->ConnectionSocket, (struct sockaddr *)&SvrAddr, sizeof(SvrAddr))) == SOCKET_ERROR) {
					closesocket(this->ConnectionSocket);
					udpSETUP = false;
					cont = false;
				}
			}
			if (cont == true) {
				bConnect = true;
				udpSETUP = true;
			}
		}

	}
	return udpSETUP;
}



//Terminate UDP
bool MySocket::TerminateUDP() {
	bool isTerminated = false;
	//ONly terminate if connection Type being used is UDP
	if (connectionType == UDP) {

		//ONly connection Socket uses this
		if (closesocket(ConnectionSocket) != SOCKET_ERROR) {
			bConnect = false;
			isTerminated = true;
		}
	}
	return isTerminated;
}



//Send data from server/client to server/client
int MySocket::SendData(const char* buffer, int size) {

	if (connectionType == TCP) {

		//Whose sending the data
		if (mySocket == CLIENT) {
			return (send(WelcomeSocket, buffer, size, 0));
		}
		else if (mySocket == SERVER) {
			return (send(ConnectionSocket, buffer, size, 0));
		}
	}
	else if (connectionType == UDP) {
		if (mySocket == SERVER) {
			return (sendto(ConnectionSocket, buffer, size, 0, (struct sockaddr*)&RespAddr, sizeof(SvrAddr)));
		}
		else if (mySocket == CLIENT) {

			SvrAddr.sin_family = AF_INET;
			SvrAddr.sin_port = htons(Port);
			//SvrAddr.sin_addr.s_addr = INADDR_ANY;
			SvrAddr.sin_addr.s_addr = inet_addr(IPAddr.c_str());

			return (sendto(ConnectionSocket, buffer, size, 0, (struct sockaddr*)&SvrAddr, sizeof(SvrAddr)));
		}

	}
	return 0;
}


//Used to receive the last block of RAW data stored in the internal MySocket Buffer.
int MySocket::GetData(char* buf) {

	int bufSize = 0;
	//Clear Buffer to safe empty state
	memset(Buffer, 0, MaxSize);

	if (connectionType == TCP) {
		if (mySocket == CLIENT) {
			bufSize = (recv(WelcomeSocket, Buffer, MaxSize, 0));
		}
		else if (mySocket == SERVER) {
			bufSize = (recv(ConnectionSocket, Buffer, MaxSize, 0));
		}
		memcpy(buf, Buffer, bufSize);
	}
	else if (connectionType == UDP) {
		int len = sizeof(this->RespAddr);
		bufSize = (recvfrom(ConnectionSocket, Buffer, MaxSize, 0, (struct sockaddr*)&RespAddr, &len));
		memcpy(buf, Buffer, bufSize);
	}
	return bufSize;
}



//Getter - IP
std::string MySocket::GetIPAddr() {
	return this->IPAddr;
}



//Setter - IP
bool MySocket::SetIPAddr(std::string ip) {
	//ONly assign when a connection HASN'T BEEN MADE
	if (bConnect == false) {
		IPAddr = ip;
		return true;
	}
	return false;
}



//Setter - Port
bool MySocket::SetPortNum(int num) {
	//ONly assign when a connection HASN'T BEEN MADE
	if (bConnect == false) {
		Port = num;
		return true;
	}
	return false;
}



//MySocket - Port
int MySocket::GetPort() {
	return this->Port;
}



//Getter - MySocket
SocketType MySocket::GetType() {
	return this->mySocket;
}



//Setter - MySocket
bool MySocket::SetType(SocketType type) {
	//ONly assign when a connection HASN'T BEEN MADE
	if (bConnect == false) {
		mySocket = type;
		return true;
	}
	return false;
}

