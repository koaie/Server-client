#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <vector>
#include <cctype>
#include <algorithm>
#include <boost\container_hash\hash.hpp>
#include <stdlib.h>
#include <ctime>
#include <fstream>
#include <stdexcept> 


#pragma comment (lib, "ws2_32.lib")

void GetTime(std::string* Timeptr)
{
	char Buf[4096];
	struct tm TTime;
	std::time_t RawTime = std::time(0);
	localtime_s(&TTime, &RawTime);
	strftime(Buf, sizeof(Buf), "%d-%m-%Y %I:%M:%S", &TTime);
	std::string Time(Buf);
	*Timeptr = Time;
}

std::string ToString(size_t x)
{
	std::stringstream y;
	y << x;
	return y.str();
}

void SetError(std::string x , std::string y , std::string* z , std::string Error)
{
	if (x == y)
	{
		*z = Error;
	}
}

bool IsEmpty(std::ifstream& File)
{
	return File.peek() == std::ifstream::traits_type::eof();
}


void Encrypt(std::string str , std::string* Hashedstr)
{
	std::stringstream y;
	boost::hash<std::string> strhash;
	std::size_t SizetHashedStr = strhash(str);
	y << SizetHashedStr;
	*Hashedstr = y.str();
}

void SendMsg(SOCKET clientSocket, std::string temp , std::ostream& Log)
{
	std::string Time;
	std::string* Timeptr = &Time;
	GetTime(Timeptr);
	std::cout << Time << ":\tServer: " << temp << std::endl;
	send(clientSocket, temp.c_str(), temp.size() + 1, 0);
	Log << Time << ":\tServer: " << temp << std::endl;
}

void main()
{
	std::string Name, HardwareID , Version = "version:0.1.1", Time , Database = "Users.db" , LogLocation = "log.log";
	std::string* Timeptr = &Time;

	std::ofstream Log;
	Log.open(LogLocation, std::ofstream::app);

	std::ifstream EmptyCheck(Database);

	if (IsEmpty(EmptyCheck))
	{
		std::ofstream File;
		File.open(Database, std::ofstream::app);
		File << "\t Hardware ID \t\t Username \t\t Password\n";
		File.close();
	}

	WSADATA WsData;
	WORD ver = MAKEWORD(2, 2);

	int WsOk = WSAStartup(ver, &WsData);
	if (WsOk != 0)
	{
		std::cerr << "Can't Initialize winsock! Quitting" << std::endl;
		return;
	}
	
	SOCKET Listening = socket(AF_INET, SOCK_STREAM, 0);
	if (Listening == INVALID_SOCKET)
	{
		std::cerr << "Can't create a socket! Quitting" << std::endl;
		return;
	}

	sockaddr_in Hint;
	Hint.sin_family = AF_INET;
	Hint.sin_port = htons(8080);
	Hint.sin_addr.S_un.S_addr = INADDR_ANY;
	
	bind(Listening, (sockaddr*)&Hint, sizeof(Hint));

	listen(Listening, SOMAXCONN);

	fd_set Master;
	FD_ZERO(&Master);

 
	FD_SET(Listening, &Master);

	char Buf[4096];

	while (true)
	{
		Sleep(1);
		fd_set Copy = Master;

		int SocketCount = select(0, &Copy, nullptr, nullptr, nullptr);

		for (int i = 0; i < SocketCount; i++)
		{
			SOCKET Sock = Copy.fd_array[i];

			if (Sock == Listening)
			{
				SOCKET Client = accept(Listening, nullptr, nullptr);

				FD_SET(Client, &Master);

			}
			else 
			{
				ZeroMemory(Buf, 4096);
				int BytesReceived = recv(Sock, Buf, 4096, 0);

				GetTime(Timeptr);
				if (std::string(Buf).find("live:") == std::string::npos && std::string(Buf).find("hwid:") == std::string::npos && std::string(Buf).find("name:") == std::string::npos && std::string(Buf).find("version:") == std::string::npos)
				{
					Log << Time << ":\t(" << Name << ") Client: " << std::string(Buf) << std::endl;
					std::cout << Time << ":\t(" << Name << ") Client: " << Buf << std::endl;
				}

	           if (std::string(Buf).find("error:") != std::string::npos)
	           {

				std::string ErrorCode = std::string(Buf).erase(0, 6), Error;
				std::string* Errorptr = &Error;

		           if (std::string(Buf) == "000")
		           {
		                SendMsg(Sock, "You have been banned" , Log);
		            	closesocket(Sock);
			            FD_CLR(Sock, &Master);
		           }
	            SetError(ErrorCode, "001", Errorptr, "Cannot send empty Username or password");
				SetError(ErrorCode, "002", Errorptr, "Invaild use of a command");
		        SendMsg(Sock, Error , Log);
	           }
	           
			   else if (std::string(Buf).find("name:") != std::string::npos)
			   {
				   send(Sock, std::string("Received name").c_str(), std::string("Received name").size() + 1, 0);
				   Name = std::string(Buf);
				   Name.erase(0, 5);
			   }

			   else if (std::string(Buf).find("-ping") != std::string::npos)
			   {
				   SendMsg(Sock, "Pong", Log);
			   }

			   else if (std::string(Buf).find("live:") != std::string::npos)
			   {
				   send(Sock, std::string("live").c_str(), std::string("live").size() + 1, 0);
			   }

			   else if (std::string(Buf).find("hwid:") != std::string::npos)
			   {
				   send(Sock, std::string("Received hwid").c_str(), std::string("Received hwid").size() + 1, 0);
				   HardwareID = std::string(Buf);
				   HardwareID.erase(0, 5);
			   }
				
				else if (std::string(Buf).find("version:") != std::string::npos)
				{
					if (std::string(Buf) == Version)
					{
						send(Sock, std::string("Received version").c_str(), std::string("Received version").size() + 1, 0);
						GetTime(Timeptr);
						std::cout << "-------------------------------------------------------------------------\nNew connection: " << Name << "\nTime: " << Time << "\tHWID: " << HardwareID << "\nClient " << Buf << "\tServer " << Version << "\n-------------------------------------------------------------------------\n\n";
						Log  << "-------------------------------------------------------------------------\nNew connection: " << Name << "\nTime: " << Time << "\tHWID: " << HardwareID << "\nClient " << Buf << "\tServer " << Version << "\n-------------------------------------------------------------------------\n\n";
						Name = "";
					}
					else
					{
						SendMsg(Sock, "Update required, Logging you off...\n" , Log);
						closesocket(Sock);
						FD_CLR(Sock, &Master);
					}
				}

				else if (std::string(Buf).find("-login") != std::string::npos)
				{
					std::string Login, HashedUsername, HashedPassword, Password, Username , ServerUsername, ServerPassword , ServerHardwareID , LineOutput;
					std::string* ptrHashedUsername = &HashedUsername;
					std::string* ptrHashedPassword = &HashedPassword;

					std::ifstream UserCheck(Database);

					Login = std::string(Buf);
					Login.erase(0, 6);

					size_t SemiColon = Login.find_first_of(':');
					Username = Login.substr(0, SemiColon),
					Password = Login.substr(SemiColon + 1);

					Encrypt(Username, ptrHashedUsername);
					Encrypt(Password, ptrHashedPassword);

					int Line = 0;
					while (std::getline(UserCheck, LineOutput))
					{
						++Line;
						if (LineOutput.find(HashedUsername) != std::string::npos)
						{
							int pos = LineOutput.find_first_of(":");
							ServerHardwareID = LineOutput.substr(0,pos);
							std::string Split = LineOutput.substr(pos + 1);
							pos = Split.find_first_of(":");
							ServerUsername = Split.substr(0, pos), ServerPassword = Split.substr(pos + 1);
						}
					}



						if (ServerUsername == HashedUsername && ServerPassword == HashedPassword)
						{							
							if (ServerHardwareID == HardwareID)
							{
								SendMsg(Sock, "Login successfully", Log);
								ServerUsername = "";
								ServerPassword = "";
								HashedUsername = "";
								HashedPassword = "";
								HardwareID = "";
							}
							else
							{
								SendMsg(Sock, "This account isn't yours!", Log);
								ServerUsername = "";
								ServerPassword = "";
								HashedUsername = "";
								HashedPassword = "";
								HardwareID = "";
							}
						}
						else
						{
							SendMsg(Sock, "Username or password are incorrect", Log);
							ServerUsername = "";
							ServerPassword = "";
							HashedUsername = "";
							HashedPassword = "";
							HardwareID = "";
						}
				}

				else if (std::string(Buf).find("-register") != std::string::npos)
				{
						std::string Register, HashedUsername, HashedPassword, Password, Username, LineOutput;
						std::string* ptrHashedUsername = &HashedUsername;
						std::string* ptrHashedPassword = &HashedPassword;
						bool PassCheck = false , Exists = false , Registerd = false;

						std::ifstream UserCheck(Database);

						Register = std::string(Buf);
						Register.erase(0, 9);

						size_t SemiColon = Register.find_first_of(':');
						Username = Register.substr(0, SemiColon),
							Password = Register.substr(SemiColon + 1);

						Encrypt(Username, ptrHashedUsername);
						Encrypt(Password, ptrHashedPassword);

						int Line = 0;
						while (std::getline(UserCheck, LineOutput))
						{
							++Line;
							if (LineOutput.find(HardwareID) != std::string::npos)
							{
								Registerd = true;
							}

							else if (LineOutput.find(HashedUsername) != std::string::npos)
							{
								Exists = true;
							}

							else
							{
								if (HashedPassword != "" && HashedUsername != "")
								{
								PassCheck = true;
								}
							}

						}
						
						if (Registerd == true)
						{
							SendMsg(Sock, "You already registerd", Log);
							PassCheck = false;
							HashedUsername = "";
							HashedPassword = "";
							HardwareID = "";
						}

						else if (Exists == true)
						{
							SendMsg(Sock, "User already exists", Log);
							PassCheck = false;
							HashedUsername = "";
							HashedPassword = "";
							HardwareID = "";
						}

						 else if (PassCheck == true)
						{
							SendMsg(Sock, "Account successfuly created", Log);
							Sleep(100);
							std::ofstream Register;
							Register.open(Database, std::ofstream::app);
							Register << std::endl << HardwareID << ":" << HashedUsername << ":" << HashedPassword;
							HashedUsername = "";
							HashedPassword = "";
							HardwareID = "";
							Register.close();
						}
				}

				else if (std::string(Buf) == "-version")
				{
					std::string str = Version;
					str.erase(0, 8);
					SendMsg(Sock, str , Log);
				}

				else if (std::string(Buf) == "-time")
				{
                    GetTime(Timeptr);
					SendMsg(Sock, Time , Log);
				}


				else if (std::string(Buf) == "-disconnect")
				{
					SendMsg(Sock, Name + " disconnected" , Log);
					Name = "";
					closesocket(Sock);
					FD_CLR(Sock, &Master);
				}
			    
				else if (std::string(Buf) == "-help")
				{
					std::string Help = "-ping | send a request to server\n-version | to see the current version\n-username <Username> | input Username\n-password <password> | input password\n-login | login after inputing username and password\n-register | register after inputing username and password\n-time | get server's time\n-disconnect | disconnect from the server\n-exit | disconnect from the server and exit the program";
					SendMsg(Sock, Help , Log);
				}

				else
				{
					std::string Temp = "Invalid command , type -help for help";
					SendMsg(Sock, Temp , Log);
				}


			if (BytesReceived <= 0 || BytesReceived == SOCKET_ERROR)
			{
				SendMsg(Sock, Name + " disconnected", Log);
				Name = "";
				closesocket(Sock);
				FD_CLR(Sock, &Master);
			}
			else
			{
					for (int i = 0; i < Master.fd_count; i++)
					{
						SOCKET OutSock = Master.fd_array[i];
						if (OutSock != Listening && OutSock != Sock)
						{
							std::ostringstream ss;
							ss << "SOCKET #" << Sock << ": " << Buf << "\r\n";
							std::string StrOut = ss.str();

							send(OutSock, StrOut.c_str(), StrOut.size() + 1, 0);
						}
					}
				}
			}
		}
	}

	FD_CLR(Listening, &Master);
	closesocket(Listening);

	WSACleanup();
	Log.close();
	std::cin.get();
}