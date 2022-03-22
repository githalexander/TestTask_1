#pragma comment (lib,"ws2_32.lib")
#pragma warning (disable:4996)
#define _CRTDBG_MAP_ALLOC

#include <winsock.h>
#include "process.h"
#include <iostream>
#include "help_winsock_function.h"
#include <sstream>
#include <fstream>
#include <filesystem>

#include <stdlib.h>
#include <crtdbg.h>

const int WINSOCK_VERSION = 0x0202;
const int BACKLOG = 0x200;

#include <stdlib.h>
#include <vector>


struct informStructForClient {
	SOCKET _socket;
	std::string ipAdress_str;
	std::string str_folder_path;
	sockaddr_in client_adress;
};


unsigned __stdcall  clientThread(void* args)
{
	
	informStructForClient infStruct = *((informStructForClient*)args);
	SOCKET tcp_socket = infStruct._socket;

	char buffer[1024];

	//get file name and udp port from client
	if (recv(tcp_socket, buffer, 1024, 0) == SOCKET_ERROR)
	{
		WSACleanup();
		std::cout << "Error recv " << WSAGetLastError();
		closesocket(tcp_socket);
	}
	std::stringstream str_stream; str_stream << buffer;
	u_short len, udp_client_port; std::string fileName;
	str_stream >> udp_client_port >> len>> fileName;
	fileName=fileName.substr(0,len);



	SOCKET udp_socket = createNonBlockSocket(SOCK_DGRAM);
	struct sockaddr_in  my_udp_adress; int my_addr_udp_socket_size= sizeof(my_udp_adress);
	
	//bind udp socket and adress
	my_udp_adress.sin_family = AF_INET;
	my_udp_adress.sin_addr.S_un.S_addr = inet_addr(infStruct.ipAdress_str.c_str());
	my_udp_adress.sin_port = htons(0); ;
	bind(udp_socket, (sockaddr*)&my_udp_adress, sizeof(my_udp_adress));
	getsockname(udp_socket, (sockaddr*)&my_udp_adress,&my_addr_udp_socket_size);

	
	//send udp adress for recv message frjm client 
	std::stringstream mesStream_udpAdress; mesStream_udpAdress << my_udp_adress.sin_port;
	send(tcp_socket, mesStream_udpAdress.str().c_str(), mesStream_udpAdress.str().size(), 0);

	//tcp socket non block
	unsigned long ulMode = 1;
	ioctlsocket(tcp_socket, FIONBIO, (unsigned long*)&ulMode);

	
	bool flg_is_end_recvData = false;
	std::vector<std::string> vectorDataLine;
	sockaddr_in rec_dest_addr; int rec_dest_addr_size=sizeof(rec_dest_addr);
	while (!flg_is_end_recvData) {
		
		if (recvfrom(udp_socket, &buffer[0], sizeof(buffer), 0,
			(sockaddr*)&rec_dest_addr, &rec_dest_addr_size) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				std::cout << "error : " << WSAGetLastError() << "\n";
				flg_is_end_recvData = true;
			}
		}
		else
		{	
			//check that recv mess from udp port client
			if (rec_dest_addr.sin_port!= udp_client_port) {
				continue;
			}
			int id, len; std::string str;
			std::stringstream str_stream; str_stream << buffer;

			str_stream >> id >> len; 
			std::getline(str_stream, str);
			
			if ((id +1) > vectorDataLine.size()) { vectorDataLine.resize(id + 1); }
			vectorDataLine[id] = str.substr(1, len);
			
			//verification udp packet
			str_stream = std::stringstream();
			str_stream << id << " ";
			send(tcp_socket, str_stream.str().c_str(), str_stream.str().length(), 0);
			
		}

		////end of data confirmation
		if (recv(tcp_socket, buffer, 1024, 0) == SOCKET_ERROR) {
			if (WSAGetLastError() != WSAEWOULDBLOCK)
			{
				std::cout << "error : " << WSAGetLastError() << "\n";
				flg_is_end_recvData = true;
			}

		}
		else {
			flg_is_end_recvData = true;
		}
	}

	//Save file
	auto result = std::filesystem::create_directory(infStruct.str_folder_path);
	infStruct.str_folder_path.append("//");
	infStruct.str_folder_path.append(fileName);
	
	std::ofstream outfile(infStruct.str_folder_path);
	for (auto line : vectorDataLine) {
		outfile << line  <<"\n";
	}
	outfile.close();
	std::cout << "file saved: " << infStruct.str_folder_path << std::endl;
	closesocket(udp_socket);
	closesocket(tcp_socket);
	return 0;
}

int main(int argc, char* argv[])
{
    char* server_ip_str = argv[1];
	char* tcp_port_str = argv[2];


	WSADATA  wsaData;
	if (WSAStartup(WINSOCK_VERSION, &wsaData))
	{
		std::cout << "Error initialzation WinSock  " << WSAGetLastError();
		std::cin.get();
		return -1;
	}

	
	//creation socket 
	SOCKET server_socket;
	if ((server_socket = createSocketAndBindWithAdress(SOCK_STREAM, server_ip_str, tcp_port_str)) == INVALID_SOCKET){
		std::cout<<"server not start";
		return -1;
	};
	

	//waiting
	if (listen(server_socket, BACKLOG))
	{
		closesocket(server_socket);
		WSACleanup();
		std::cout << "Error listen " << WSAGetLastError();
		std::cin.get();
		return -1;
	}

	std::cout << "server start" << std::endl;
	
	//structura for send in _beginthreadex parameters
	informStructForClient infStruct;
	infStruct.ipAdress_str = std::string(server_ip_str);
	infStruct.str_folder_path = std::string(argv[3]);
	int client_addr_size = sizeof(infStruct.client_adress);

	while ((infStruct._socket = accept(server_socket, (sockaddr*)
		&infStruct.client_adress, &client_addr_size)))
	{
		//new client
		_beginthreadex(0, 2000, clientThread, (void*)&infStruct, 0, 0);
	}
	
	std::cin.get();
	return 0;
}
