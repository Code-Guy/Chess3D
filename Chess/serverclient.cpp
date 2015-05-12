#include "serverclient.h"
#include <stdio.h>
#include <iostream>
#include <WS2tcpip.h>

#define bzero(a, b) memset(a, 0, b)

SOCKET ServerClient::clientSocket;
SOCKET ServerClient::serverSocket;

SOCKADDR_IN ServerClient::addrClient;//client的地址信息
SOCKADDR_IN ServerClient::addrServer;//server的地址信息
SOCKET ServerClient::acceptedSock;//accept返回的用于和client通信的socket
int ServerClient::sockaddrSize;//sockaddr结构体的大小

bool ServerClient::hasConnectedToServer;
bool ServerClient::hasConnectedWithClient;

string ServerClient::serverIP;
int ServerClient::serverPort;

string ServerClient::localIP;
string ServerClient::globalIP;

char ServerClient::hostName[DEFAULT_BUFFER_SIZE];

pthread_t ServerClient::clientTid;
pthread_t ServerClient::serverTid;

Ally *ServerClient::ally = NULL;
Rival *ServerClient::rival = NULL;

void ServerClient::Init()//初始化
{
	hasConnectedToServer = false;
	hasConnectedWithClient = false;

	sockaddrSize = sizeof(struct sockaddr);//sockaddr结构体的大小

	LoadWSA();//加载socket库

	//创建服务器和客户端的socket
	CreateSocket(clientSocket);
	CreateSocket(serverSocket);

	//获取本地IP
	GetLocalIP(localIP);
	//获取外网IP
	GetPublicIP(globalIP);

	printf("Global IP: %s\n", globalIP.c_str());
	printf("Local IP: %s\n", localIP.c_str());

	//设置服务器的绑定和监听
	BindSocket(serverSocket, localIP, 0);
	SetListen(serverSocket, DEFAULT_BACKLOG);

	//打印出服务器的相关信息
	PrintServerIPPort();

	//获取Ally和Rival instance
	ally = Ally::GetInstance();
	rival = Rival::GetInstance();

	ally->SetName(hostName);
}

bool ServerClient::CreateSocket(SOCKET& sock,int type)//创建socket
{
	sock = socket(AF_INET, type, 0);//创建一个tcp套接字
	if(sock == INVALID_SOCKET)
		return false;
	return true;
}

bool ServerClient::ConnectToServer(SOCKET& sock, string ip, string port)//向服务器发出连接请求
{
	SOCKADDR_IN address;
	address.sin_family = AF_INET;

	address.sin_port = htons((short)atoi(port.c_str()));
	address.sin_addr.S_un.S_addr = inet_addr(ip.c_str());

	memset(address.sin_zero, 0, sizeof(address.sin_zero));

	if(connect(sock, (const struct sockaddr *)&address, sizeof(struct sockaddr))
		== SOCKET_ERROR)
		return false;
	return true;
}

bool ServerClient::CloseSocket(SOCKET& sock)//关闭套接字
{
	if(shutdown(sock, SD_BOTH) == SOCKET_ERROR)
		return false;
	return true;
}

bool ServerClient::BindSocket(SOCKET &sock, string ip, unsigned short port)//将socket绑定到一个IP地址和一个端口上
{
	SOCKADDR_IN address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = inet_addr(ip.c_str());
	//address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);
	memset(address.sin_zero, 0, sizeof(address.sin_zero));

	if(bind(sock, (const struct sockaddr *)&address, sizeof(struct sockaddr)) == SOCKET_ERROR)
		return false;
	return true;
}

bool ServerClient::SetListen(SOCKET &sock, int backLog)//将socket设置为监听模式等待client连接请求
{
	if(listen(sock, backLog) == SOCKET_ERROR)
		return false;
	return true;
}

void* ServerClient::CommunicateWithServer(void *args)//和服务器交流
{
	//1.和服务器建立连接
	printf("Please input the server's IP and Port:\n");

	while(true)
	{
		if (hasConnectedWithClient)//如果和客户端连上的话，即扮演了服务器的角色，就无需下面的逻辑了
		{
			return NULL;
		}
		char key[DEFAULT_BUFFER_SIZE] = {0};
		//获取键盘输入
		cin.getline(key, sizeof(key));

		vector<string> ret;
		Split(key, ret, " ");

		if (!hasConnectedToServer && ret.size() == 2)//conn
		{
			if (ConnectToServer(clientSocket, ret[0], ret[1]))
			{
				printf("Connect to server successfully!\n");
				hasConnectedToServer = true;

				//先发送一个先手后手包 随机生成先手后手
				ally->RandomOffsensive();
				rival->SetOffsensive(!ally->GetOffsensive());

				whichTurn = ally->GetOffsensive();

				Message msg;
				msg.isOpen = true;
				msg.isRivalOffsensive = ally->GetOffsensive() == 0 ? 1 : 0;
				strcpy(msg.hostName, hostName);

				int ret = send(clientSocket, (char *)&msg, sizeof(msg), NULL);

				if (ret == SOCKET_ERROR)
				{
					printf("Server exit!\n");
					break;
				}
				break;
			}
			else
			{
				printf("Connect to server failed!\n");
			}
		}
	}

	//扮演客户端，开两个线程
	pthread_t sendTid, recvTid;
	
	int sendRet = pthread_create(&sendTid, NULL, SendPieceMoveMsg, (void *)&clientSocket);
	int recvRet = pthread_create(&recvTid, NULL, RecvPieceMoveMsg, (void *)&clientSocket);

	pthread_join(sendTid, NULL);
	pthread_join(recvTid, NULL);

	return NULL;
}

void* ServerClient::CommunicateWithClient(void *args)//和客户端交流
{
	//1.监听客户端
	while(true)
	{
		if (hasConnectedToServer)//如果和服务器连上的话，即扮演了客户端的角色，就无需下面的逻辑了
		{
			return NULL;
		}
		if (!hasConnectedWithClient)
		{
			acceptedSock = accept(serverSocket, (struct sockaddr *)&addrClient, &sockaddrSize);

			if(acceptedSock == INVALID_SOCKET)
			{
				//cout << "accept failed" << endl;
				continue;
			}
			else
			{
				cout << "Client from " << inet_ntoa(addrClient.sin_addr) << ":"
					<< ntohs(addrClient.sin_port) << endl;
				hasConnectedWithClient = true;

				Message msg;
				msg.isOpen = true;
				msg.isRivalOffsensive = -1;
				strcpy(msg.hostName, hostName);

				int ret = send(acceptedSock, (char *)&msg, sizeof(msg), NULL);
				if (ret == SOCKET_ERROR)
				{
					printf("Server exit!\n");
					break;
				}

				break;
			}
		}
	}
	
	//扮演服务器，开两个线程
	pthread_t sendTid, recvTid;

	int sendRet = pthread_create(&sendTid, NULL, SendPieceMoveMsg, (void *)&acceptedSock);
	int recvRet = pthread_create(&recvTid, NULL, RecvPieceMoveMsg, (void *)&acceptedSock);

	pthread_join(sendTid, NULL);
	pthread_join(recvTid, NULL);

	return NULL;
}

void* ServerClient::SendPieceMoveMsg(void *args)//传送棋子移动信息
{
	SOCKET sock = *(SOCKET *)args;//还原成原来的socket

	while(true)
	{
		//2.给服务器发送Ally棋子移动信息
		Message sendMsg = ally->GetAllyPieceMoveMsg();
		if (sendMsg.prevPos == INVALID_POS && sendMsg.targetPos == INVALID_POS)//invalid pos 
		{
			continue;
		}
		else
		{
			whichTurn = 1;//交给对方下
			myTimer = 0;//重置计时器

			//获取到了Ally下一步要移动的棋子的移动位置信息（前一个位置+下一个位置）
			//发送给服务器
			int ret = send(sock, (char *)&sendMsg, sizeof(sendMsg), NULL);

			if (ret == SOCKET_ERROR)
			{
				printf("Server exit!\n");
				break;
			}
			else
			{
				//PrintPieceMoveMsg(sendMsg, Send);
			}
		}
	}

	return NULL;
}

void* ServerClient::RecvPieceMoveMsg(void *args)//接收棋子移动信息
{
	SOCKET sock = *(SOCKET *)args;//还原成原来的socket

	while(true)
	{
		Message recvMsg;

		int ret = recv(sock, (char *)&recvMsg, sizeof(recvMsg), NULL);
		if (ret == SOCKET_ERROR)
		{
			printf("Client exit!\n");
			break;
		}
		else
		{
			whichTurn = 0;//我方下棋
			myTimer = 0;//重置计时器

			if (recvMsg.isOpen)
			{
				if (recvMsg.isRivalOffsensive != -1)
				{
					ally->SetOffsensive(recvMsg.isRivalOffsensive);
					rival->SetOffsensive(!ally->GetOffsensive());

					whichTurn = ally->GetOffsensive();
				}
				
				rival->SetName(recvMsg.hostName);
			}
			else
			{
				//PrintPieceMoveMsg(recvMsg, Recv);
				rival->DriveByMsg(recvMsg);
			}
		}
	}

	return NULL;
}

void ServerClient::CreateThreads()//创建服务器和客户端两个线程
{
	int clientRet = pthread_create(&clientTid, NULL, CommunicateWithServer, NULL);
	int serverRet = pthread_create(&serverTid, NULL, CommunicateWithClient, NULL);
}

void ServerClient::WaitThreads()//等待服务器和客户端两个线程
{
	pthread_join(clientTid, NULL);
	pthread_join(serverTid, NULL);

	pthread_exit(NULL);
}

bool ServerClient::LoadWSA()//加载socket库
{
	WSADATA wsaData;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)
		return true;
	return false;
}

bool ServerClient::CloseWSA()//关闭加载的套接字库
{
	if(WSACleanup() == 0)
		return true;
	return false;
}

void ServerClient::PrintServerIPPort()//打印出服务器的相关信息
{
	getsockname(serverSocket, (struct sockaddr *)&addrServer, &sockaddrSize);

	serverIP = inet_ntoa(addrServer.sin_addr);
	serverPort = ntohs(addrServer.sin_port);

	printf("\n---------Network Stuff--------\n");
	printf("Bind IP: %s, Bind Port: %d\n", serverIP.c_str(), serverPort);
}

void ServerClient::PrintPieceMoveMsg(const Message &msg, TransmitType transmitType)//打印出棋子移动信息
{
	if (transmitType == Send)
	{
		printf("Send piece move message:{(%d, %d), (%d, %d)}\n", msg.prevPos.x, msg.prevPos.y, msg.targetPos.x, msg.targetPos.y);
	}
	else if(transmitType == Recv)
	{
		printf("Receive piece move message:{(%d, %d), (%d, %d)}\n", msg.prevPos.x, msg.prevPos.y, msg.targetPos.x, msg.targetPos.y);
	}
}

bool ServerClient::GetLocalIP(string &ip)//获取本地IP
{
	char ipBuffer[256];
	//1.初始化wsa 
	//2.获取主机名  

	int ret = gethostname(hostName,sizeof(hostName));  
	if (ret == SOCKET_ERROR)  
	{  
		return false;  
	}  
	//3.获取主机ip  
	HOSTENT* host = gethostbyname(hostName);  
	if (host == NULL)  
	{  
		return false;  
	}  
	//4.转化为char*并拷贝返回  
	strcpy(ipBuffer,inet_ntoa(*(in_addr*)*host->h_addr_list));  

	ip = Trim(ipBuffer);
	return true;  
}

bool ServerClient::GetPublicIP(string &ip)//获取外网IP
{
	int sock;  
	char **pptr = NULL;  
	struct sockaddr_in destAddr;  
	struct hostent    *ptr = NULL;  
	char destIP[128];  

	sock = socket(AF_INET,SOCK_STREAM,0);  
	if( -1 == sock ){  
		perror("creat socket failed");  
		return false;  
	}  
	bzero((void *)&destAddr, sizeof(destAddr));  
	destAddr.sin_family = AF_INET;  
	destAddr.sin_port = htons(80);  
	ptr = gethostbyname("www.ip138.com");  
	if(NULL == ptr){  
		perror("gethostbyname error");  
		return false;  
	}  
	for(pptr=ptr->h_addr_list ; NULL != *pptr ; ++pptr){  
		inet_ntop(ptr->h_addrtype,*pptr,destIP,sizeof(destIP));  
		ip = destIP;  
		return true;  
	}  
	return true;
}

void ServerClient::Close()//关闭
{
	CloseSocket(clientSocket);
	CloseSocket(serverSocket);
	CloseSocket(acceptedSock);
	CloseWSA();
}