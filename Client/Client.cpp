#pragma comment (lib, "ws2_32.lib")
#pragma warning (disable:4996)

//#include <winsock.h>
#include <windows.h> 
#include "iostream"
#include <string>
#include <fstream>
#include <sstream>

#include <vector>
#include "help_winsock_function.h"

const int WINSOCK_VERSION = 0x0202;

struct udpPacket {
	int id;
	std::string data;
	bool isVerificate = false;
	//udpPacket();
	udpPacket(int id, std::string str) {
		this->id = id;
		int lengthPacket = str.size();	
		
		std::stringstream str_stream; 
		str_stream << id<< " "<< lengthPacket<<" "<<str;
		data = str_stream.str();		
	}
};

int main(int argc, char* argv[])
{
	
	char* server_ip_str = argv[1];
	char*  server_port_str = argv[2];
	u_short server_port = (u_short)strtoul(server_port_str, NULL,10);

	char* udp_server_port_str = argv[3];
	u_short udp_server_port = (u_short)strtoul(udp_server_port_str, NULL, 10);
	char* fileName_str = argv[4];
	int time_interval= strtoul(argv[5], NULL, 10);

	//initialzation WinSock
	WSADATA  wsaData;
	if (WSAStartup(WINSOCK_VERSION, &wsaData))
	{
		std::cout << "Error initialzation WinSock  " << WSAGetLastError();
		return -1;
	}


	//creation socket   
	sockaddr_in dest_addr;
	SOCKET tcp_socket = createSocketAndConnect(SOCK_STREAM, server_ip_str, server_port_str, dest_addr);
	if (tcp_socket == INVALID_SOCKET)
	{
		WSACleanup();
		closesocket(tcp_socket);
		std::cout << "Error Socket " << WSAGetLastError();
		std::cin.get();
		return INVALID_SOCKET;
	}
	
	//send port and file Name 
	int length_fileName = strlen(fileName_str);
	char buff[1024];
	std::stringstream str_stream; str_stream <<udp_server_port<< " "<<length_fileName<<" "<< fileName_str;
	send(tcp_socket, str_stream.str().c_str(), str_stream.str().length(), 0);

	SOCKET udp_socket;
	if ((udp_socket = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		WSACleanup();
		closesocket(tcp_socket);
		closesocket(tcp_socket);
		std::cout << "Error Socket " << WSAGetLastError();
		std::cin.get();
		return INVALID_SOCKET;
	}

	// get sockaddr_in for bind udp socket
	 sockaddr_in  server_addr_udp;
	int my_addr_size = sizeof(server_addr_udp);
	getsockname(tcp_socket, (struct sockaddr*) & server_addr_udp, &my_addr_size);
	server_addr_udp.sin_port = udp_server_port;

	if (bind(udp_socket, (sockaddr*)&server_addr_udp,
		sizeof(server_addr_udp)))
	{
		std::cout << "bind error: " << WSAGetLastError() << "\n";
		WSACleanup();
		closesocket(udp_socket);
		closesocket(tcp_socket);
		std::cin.get();
		return INVALID_SOCKET;
	}

	//get udp port from server for send udp packet
	recv(tcp_socket, buff, 1024, 0);
	int port= (u_short)atoi(&buff[0]);
	
	sockaddr_in destAdress_for_udp;
	destAdress_for_udp.sin_family = AF_INET;
	destAdress_for_udp.sin_port = port;
	destAdress_for_udp.sin_addr.s_addr = inet_addr(server_ip_str);
		

	std::ifstream infile;
	infile.open(fileName_str, std::ifstream::in);
	if (!infile.is_open()) {
		std::cout << " file do not open ";
		WSACleanup();
		closesocket(udp_socket);
		closesocket(tcp_socket);
		return -1;
	}

	//Get tata from file and send on Server
	std::vector<udpPacket> vectorPackets;
	std::string str;       
	int i_numberPacket=0;
	while (std::getline(infile, str)) 
	{	
		udpPacket uPacket(i_numberPacket, str);
		vectorPackets.push_back(uPacket);
		sendto(udp_socket, uPacket.data.c_str(), uPacket.data.size(),0,
			(sockaddr*)&destAdress_for_udp, sizeof(destAdress_for_udp));			
		i_numberPacket++;
		
	}
	infile.close();

	//udp packet delivery check 
	bool flg = true;
	while (flg) {
		Sleep(time_interval);

		char buf[1024]; int i = sizeof(buf);
		recv(tcp_socket, buf, sizeof(buf), 0);
		std::stringstream str_stream; str_stream << buf;
		int val;
		while (str_stream >> val) {
			vectorPackets[val].isVerificate = true;
		}
		flg = false;
		for (auto uPacket : vectorPackets) {
			if (!uPacket.isVerificate) {
				sendto(udp_socket, uPacket.data.c_str(), uPacket.data.size(), 0,
					(sockaddr*)&destAdress_for_udp, sizeof(destAdress_for_udp));
				flg = true;
			}
		}
	}

	send(tcp_socket, buff, 10, 0);
	std::cout << " the file is saved on the server ";
	closesocket(udp_socket);
	closesocket(tcp_socket);
	WSACleanup();
	std::cin.get();
	return 1;
}

