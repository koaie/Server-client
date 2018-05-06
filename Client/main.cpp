#include <iostream>
#include <string>
#include <WS2tcpip.h>
#include <windows.h>
#include <boost\container_hash\hash.hpp>
#include <sstream>
#include <Lmcons.h>
#pragma comment(lib, "ws2_32.lib")


void Ram(int* ramsizepointer)
{
	MEMORYSTATUSEX StateEX;
	StateEX.dwLength = sizeof(StateEX);
	GlobalMemoryStatusEx(&StateEX);
	*ramsizepointer = (float)StateEX.ullTotalPhys / (1024 * 1024 * 1024);
}


std::string ToString(size_t x)
{
	std::stringstream y;
	y << x;
	return y.str();
}


void main()
{
	char UserChar[UNLEN + 1];
	DWORD UserLen = UNLEN + 1;
	GetUserName(UserChar, &UserLen);
	std::string User = "name:" + std::string(UserChar);
	DWORD HardDriveIDtemp;
	GetVolumeInformation(NULL , NULL , MAX_PATH + 1, &HardDriveIDtemp,NULL,NULL,NULL, MAX_PATH + 1);
	std::string HardDriveID = std::to_string(HardDriveIDtemp);

	std::string HardwareID , Cores, CpuType, CpuMask;
	SYSTEM_INFO SystemInfo;
	GetSystemInfo(&SystemInfo);

	int IntRamSize;

	Cores = std::to_string(SystemInfo.dwNumberOfProcessors);
	CpuType = std::to_string(SystemInfo.dwProcessorType);
	CpuMask = std::to_string(SystemInfo.dwActiveProcessorMask);

	int* Ramptr = &IntRamSize;
	Ram(Ramptr);
	CPUID cpuID(0);

	std::string RamSize = std::to_string(IntRamSize);

	std::string Vendor;
	Vendor += std::string((const char *)&cpuID.EBX(), 4);
	Vendor += std::string((const char *)&cpuID.EDX(), 4);
	Vendor += std::string((const char *)&cpuID.ECX(), 4);
	
	HardwareID = HardDriveID + RamSize + Cores + CpuType + CpuMask + Vendor;

	std::string Version = "version:0.1.1";
	std::string IpAddress = "79.154.1.225";	
	int Port = 8080;
											
	WSAData Data;
	WORD Ver = MAKEWORD(2, 2);
	int WsResult = WSAStartup(Ver, &Data);
	if (WsResult != 0)
	{
		std::cerr << "Can't start Winsock, Err #" << WsResult << std::endl;
		return;
	}

	SOCKET Sock = socket(AF_INET, SOCK_STREAM, 0);
	if (Sock == INVALID_SOCKET)
	{
		std::cerr << "Can't create socket" << std::endl;
				WSACleanup();
		std::cin.get();
		exit(1);
		return;
	}

	sockaddr_in Hint;
	Hint.sin_family = AF_INET;
	Hint.sin_port = htons(Port);
	inet_pton(AF_INET, IpAddress.c_str(), &Hint.sin_addr);

	int ConnResult = connect(Sock, (sockaddr*)&Hint, sizeof(Hint));
	if (ConnResult == SOCKET_ERROR)
	{
		std::cerr << "Can't connect to server" << std::endl;
		closesocket(Sock);
		WSACleanup();
		std::cin.get();
		exit(1);
		return;
	}

	char Buf[4096];
	ZeroMemory(Buf, 4096);
	std::string UserInput ,Username , Password;

	std::string* sptr = &HardwareID;
	boost::hash<std::string> SSHash;
	std::size_t HashedHardwareID = SSHash(*sptr);
	*sptr = "hwid:" + ToString(HashedHardwareID);

	send(Sock, User.c_str(), User.size() + 1, 0);
	recv(Sock, Buf, 4096, 0);
	send(Sock, HardwareID.c_str(), HardwareID.size() + 1, 0);
	recv(Sock, Buf, 4096, 0);
	send(Sock, Version.c_str(), Version.size() + 1, 0);
	recv(Sock, Buf, 4096, 0);

	UserInput = "";
	system("cls");
	std::cout << "Connected successfully to " << IpAddress << "\ton port " << Port << "\n";
	while (1)
	{
		ZeroMemory(Buf, 4096);
		UserInput = "";
		Sleep(1);
		while(UserInput.empty())
		{
			ZeroMemory(Buf, 4096);
			send(Sock, std::string("live:").c_str(), std::string("live:").size() + 1, 0);
			recv(Sock, Buf, 4096, 0);
			std::cout << "\r";
			if (std::string(Buf).empty())
			{
				system("cls");
				std::cerr << "Connection lost \n";
				closesocket(Sock);
				WSACleanup();
				std::cout << "Press any key to continue...\n";
				std::cin.get();
				exit(1);
			}
			std::cout << "> ";
			getline(std::cin, UserInput);

				if (UserInput.empty())
				{
					std::cout << "Invalid command , type -help for help\n";
				}
		}

			if (UserInput.find("-username") != std::string::npos)
			{
				if (UserInput != "-username")
				{
					Username = UserInput;
					Username.erase(0, 8);
					std::string* ptr = &Username;
					boost::hash<std::string> SHash;
					std::size_t HashedUsername = SHash(*ptr);
					*ptr = ToString(HashedUsername);
					UserInput = "";
				}
				else
				{
					std::cout << "Invalid use of a command , type -help for help\n";
					UserInput = "";
				}
			}
			else if (UserInput.find("-password") != std::string::npos)
			{
				if (UserInput != "-password")
				{
					Password = UserInput;
					Password.erase(0, 8);
					std::string* ptr = &Password;
					boost::hash<std::string> SHash;
					std::size_t HashedPassword = SHash(*ptr);
					*ptr = ToString(HashedPassword);
					UserInput = "";
				}
				else
				{
					std::cout << "Invalid use of a command , type -help for help\n";
					UserInput = "";
				}
			}


			else if (UserInput.find("-login") != std::string::npos)
			{
				if (!Username.empty() && !Password.empty())
				{
					if (UserInput == "-login")
					{
						send(Sock, HardwareID.c_str(), HardwareID.size() + 1, 0);
						recv(Sock, Buf, 4096, 0);
						std::cout << "\r";
						UserInput = "-login" + Username + ":" + Password;
						Username = "";
						Password = "";
					}
					else
					{
						UserInput = "error:002";
					}
				}
				else
				{
					UserInput = "error:001";
				}
			}

			else if (UserInput.find("-register") != std::string::npos)
			{
				if (!Username.empty() && !Password.empty())
				{
					if (UserInput == "-register")
					{
						send(Sock, HardwareID.c_str(), HardwareID.size() + 1, 0);
						recv(Sock, Buf, 4096, 0);
						std::cout << "\r";
						UserInput = "-register" + Username + ":" + Password;
						Username = "";
						Password = "";
					}
					else
					{
						UserInput = "error:002";
					}
				}
				else
				{
					UserInput = "error:001";
				}
			}

			else if (UserInput.find("-exit") != std::string::npos)
			{
				UserInput = "-disconnect";
				send(Sock, User.c_str(), User.size() + 1, 0);
				recv(Sock, Buf, 4096, 0);
				std::cout << "\r";
				send(Sock, UserInput.c_str(), UserInput.size() + 1, 0);
				recv(Sock, Buf, 4096, 0);
				UserInput = "";
				closesocket(Sock);
				WSACleanup();
				exit(1);
			}

			else if (UserInput.find("-ping") != std::string::npos)
			{
				UserInput = "-ping";
			}

		if (UserInput.size() > 0)	
		{
			send(Sock, User.c_str(), User.size() + 1, 0);
			recv(Sock, Buf, 4096, 0);
			send(Sock, HardwareID.c_str(), HardwareID.size() + 1, 0);
			recv(Sock, Buf, 4096, 0);
			ZeroMemory(Buf, 4096);
			
			int SendResult = send(Sock, UserInput.c_str(), UserInput.size() + 1, 0);
			UserInput = "";
			if (SendResult != SOCKET_ERROR)
			{
				ZeroMemory(Buf, 4096);
				int BytesReceived = recv(Sock, Buf, 4096, 0);
				if (BytesReceived > 0)
				{
					std::cout << std::string(Buf, 0, BytesReceived) << std::endl;
				}
			}

			if (ConnResult == SOCKET_ERROR)
			{
				UserInput = "";
				std::cerr << "Can't connect to server, Err #" << WSAGetLastError() << std::endl;
				closesocket(Sock);
				WSACleanup();
				std::cin.get();
				exit(1);
			}
		}
		UserInput = "";
	}

	closesocket(Sock);
	WSACleanup();
}
