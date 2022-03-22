#pragma once
#include <winsock.h>
#include <iostream>


SOCKET createSocketAndConnect(int typeSocket, char* server_ip_str, char* port, sockaddr_in& dest_addr)
{
	//Create socket
	SOCKET _socket;
	if ((_socket = socket(AF_INET, typeSocket, 0)) == INVALID_SOCKET)
	{
		WSACleanup();
		std::cout << "Error Socket " << WSAGetLastError();
		return -1;
	}

	//Address
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = (u_short)atoi(port);

	if (inet_addr(server_ip_str) != INADDR_NONE)
	{
		dest_addr.sin_addr.s_addr = inet_addr(server_ip_str);
	}
	else
	{
		std::cout << "Invalid address " << server_ip_str << " " << WSAGetLastError();
		closesocket(_socket);
		WSACleanup();
		std::cin.get();
		return INVALID_SOCKET;
	}

	if (connect(_socket, (sockaddr*)&dest_addr, sizeof(dest_addr)))
	{
		std::cout << "Connect error " << WSAGetLastError();
		closesocket(_socket);
		WSACleanup();
		std::cin.get();
		return INVALID_SOCKET;
	}

	return _socket;
}


SOCKET createSocketAndBindWithAdress(int typeSocket, char* server_ip_str, char* port)
{

	SOCKET _socket;

	//Create socket 
	if ((_socket = socket(AF_INET, typeSocket, 0)) == INVALID_SOCKET)
	{
		WSACleanup();
		std::cout << "Error Socket " << WSAGetLastError();
		return -1;
	}

	//Address
	sockaddr_in dest_addr;
	dest_addr.sin_family = AF_INET;
	dest_addr.sin_port = (u_short)atoi(port);

	if (inet_addr(server_ip_str) != INADDR_NONE)
	{
		dest_addr.sin_addr.s_addr = inet_addr(server_ip_str);
	}
	else
	{
		std::cout << "Invalid address " << server_ip_str << " " << WSAGetLastError();
		closesocket(_socket);
		WSACleanup();
		std::cin.get();
		return INVALID_SOCKET;
	}

	//binding a socket to a local address 
	if (bind(_socket, (sockaddr*)&dest_addr, sizeof(dest_addr)))
	{
		closesocket(_socket);
		WSACleanup();
		std::cout << "Error bind " << WSAGetLastError();
		std::cin.get();
		return INVALID_SOCKET;
	}

	return _socket;
}

SOCKET createNonBlockSocket(int typeSocket)
{
	SOCKET _socket = socket(AF_INET, typeSocket, 0);
	if (_socket == INVALID_SOCKET)
	{
		std::cout << "ioctlsocket: " << WSAGetLastError() << "\n";
		WSACleanup();
		return -1;
	}

	unsigned long ulMode = 1;
	if (ioctlsocket(_socket, FIONBIO, (unsigned long*)&ulMode) != NO_ERROR)
	{
		std::cout << "ioctlsocket: " << WSAGetLastError() << "\n";
		closesocket(_socket);
		WSACleanup();
		std::cin.get();
		return INVALID_SOCKET;
	}
	return _socket;
}