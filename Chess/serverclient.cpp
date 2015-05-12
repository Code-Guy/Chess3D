#include "serverclient.h"
#include <stdio.h>
#include <iostream>
#include <WS2tcpip.h>

#define bzero(a, b) memset(a, 0, b)

SOCKET ServerClient::clientSocket;
SOCKET ServerClient::serverSocket;

SOCKADDR_IN ServerClient::addrClient;//client�ĵ�ַ��Ϣ
SOCKADDR_IN ServerClient::addrServer;//server�ĵ�ַ��Ϣ
SOCKET ServerClient::acceptedSock;//accept���ص����ں�clientͨ�ŵ�socket
int ServerClient::sockaddrSize;//sockaddr�ṹ��Ĵ�С

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

void ServerClient::Init()//��ʼ��
{
	hasConnectedToServer = false;
	hasConnectedWithClient = false;

	sockaddrSize = sizeof(struct sockaddr);//sockaddr�ṹ��Ĵ�С

	LoadWSA();//����socket��

	//�����������Ϳͻ��˵�socket
	CreateSocket(clientSocket);
	CreateSocket(serverSocket);

	//��ȡ����IP
	GetLocalIP(localIP);
	//��ȡ����IP
	GetPublicIP(globalIP);

	printf("Global IP: %s\n", globalIP.c_str());
	printf("Local IP: %s\n", localIP.c_str());

	//���÷������İ󶨺ͼ���
	BindSocket(serverSocket, localIP, 0);
	SetListen(serverSocket, DEFAULT_BACKLOG);

	//��ӡ���������������Ϣ
	PrintServerIPPort();

	//��ȡAlly��Rival instance
	ally = Ally::GetInstance();
	rival = Rival::GetInstance();

	ally->SetName(hostName);
}

bool ServerClient::CreateSocket(SOCKET& sock,int type)//����socket
{
	sock = socket(AF_INET, type, 0);//����һ��tcp�׽���
	if(sock == INVALID_SOCKET)
		return false;
	return true;
}

bool ServerClient::ConnectToServer(SOCKET& sock, string ip, string port)//�������������������
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

bool ServerClient::CloseSocket(SOCKET& sock)//�ر��׽���
{
	if(shutdown(sock, SD_BOTH) == SOCKET_ERROR)
		return false;
	return true;
}

bool ServerClient::BindSocket(SOCKET &sock, string ip, unsigned short port)//��socket�󶨵�һ��IP��ַ��һ���˿���
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

bool ServerClient::SetListen(SOCKET &sock, int backLog)//��socket����Ϊ����ģʽ�ȴ�client��������
{
	if(listen(sock, backLog) == SOCKET_ERROR)
		return false;
	return true;
}

void* ServerClient::CommunicateWithServer(void *args)//�ͷ���������
{
	//1.�ͷ�������������
	printf("Please input the server's IP and Port:\n");

	while(true)
	{
		if (hasConnectedWithClient)//����Ϳͻ������ϵĻ����������˷������Ľ�ɫ��������������߼���
		{
			return NULL;
		}
		char key[DEFAULT_BUFFER_SIZE] = {0};
		//��ȡ��������
		cin.getline(key, sizeof(key));

		vector<string> ret;
		Split(key, ret, " ");

		if (!hasConnectedToServer && ret.size() == 2)//conn
		{
			if (ConnectToServer(clientSocket, ret[0], ret[1]))
			{
				printf("Connect to server successfully!\n");
				hasConnectedToServer = true;

				//�ȷ���һ�����ֺ��ְ� ����������ֺ���
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

	//���ݿͻ��ˣ��������߳�
	pthread_t sendTid, recvTid;
	
	int sendRet = pthread_create(&sendTid, NULL, SendPieceMoveMsg, (void *)&clientSocket);
	int recvRet = pthread_create(&recvTid, NULL, RecvPieceMoveMsg, (void *)&clientSocket);

	pthread_join(sendTid, NULL);
	pthread_join(recvTid, NULL);

	return NULL;
}

void* ServerClient::CommunicateWithClient(void *args)//�Ϳͻ��˽���
{
	//1.�����ͻ���
	while(true)
	{
		if (hasConnectedToServer)//����ͷ��������ϵĻ����������˿ͻ��˵Ľ�ɫ��������������߼���
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
	
	//���ݷ��������������߳�
	pthread_t sendTid, recvTid;

	int sendRet = pthread_create(&sendTid, NULL, SendPieceMoveMsg, (void *)&acceptedSock);
	int recvRet = pthread_create(&recvTid, NULL, RecvPieceMoveMsg, (void *)&acceptedSock);

	pthread_join(sendTid, NULL);
	pthread_join(recvTid, NULL);

	return NULL;
}

void* ServerClient::SendPieceMoveMsg(void *args)//���������ƶ���Ϣ
{
	SOCKET sock = *(SOCKET *)args;//��ԭ��ԭ����socket

	while(true)
	{
		//2.������������Ally�����ƶ���Ϣ
		Message sendMsg = ally->GetAllyPieceMoveMsg();
		if (sendMsg.prevPos == INVALID_POS && sendMsg.targetPos == INVALID_POS)//invalid pos 
		{
			continue;
		}
		else
		{
			whichTurn = 1;//�����Է���
			myTimer = 0;//���ü�ʱ��

			//��ȡ����Ally��һ��Ҫ�ƶ������ӵ��ƶ�λ����Ϣ��ǰһ��λ��+��һ��λ�ã�
			//���͸�������
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

void* ServerClient::RecvPieceMoveMsg(void *args)//���������ƶ���Ϣ
{
	SOCKET sock = *(SOCKET *)args;//��ԭ��ԭ����socket

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
			whichTurn = 0;//�ҷ�����
			myTimer = 0;//���ü�ʱ��

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

void ServerClient::CreateThreads()//�����������Ϳͻ��������߳�
{
	int clientRet = pthread_create(&clientTid, NULL, CommunicateWithServer, NULL);
	int serverRet = pthread_create(&serverTid, NULL, CommunicateWithClient, NULL);
}

void ServerClient::WaitThreads()//�ȴ��������Ϳͻ��������߳�
{
	pthread_join(clientTid, NULL);
	pthread_join(serverTid, NULL);

	pthread_exit(NULL);
}

bool ServerClient::LoadWSA()//����socket��
{
	WSADATA wsaData;

	if(WSAStartup(MAKEWORD(2, 2), &wsaData) == 0)
		return true;
	return false;
}

bool ServerClient::CloseWSA()//�رռ��ص��׽��ֿ�
{
	if(WSACleanup() == 0)
		return true;
	return false;
}

void ServerClient::PrintServerIPPort()//��ӡ���������������Ϣ
{
	getsockname(serverSocket, (struct sockaddr *)&addrServer, &sockaddrSize);

	serverIP = inet_ntoa(addrServer.sin_addr);
	serverPort = ntohs(addrServer.sin_port);

	printf("\n---------Network Stuff--------\n");
	printf("Bind IP: %s, Bind Port: %d\n", serverIP.c_str(), serverPort);
}

void ServerClient::PrintPieceMoveMsg(const Message &msg, TransmitType transmitType)//��ӡ�������ƶ���Ϣ
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

bool ServerClient::GetLocalIP(string &ip)//��ȡ����IP
{
	char ipBuffer[256];
	//1.��ʼ��wsa 
	//2.��ȡ������  

	int ret = gethostname(hostName,sizeof(hostName));  
	if (ret == SOCKET_ERROR)  
	{  
		return false;  
	}  
	//3.��ȡ����ip  
	HOSTENT* host = gethostbyname(hostName);  
	if (host == NULL)  
	{  
		return false;  
	}  
	//4.ת��Ϊchar*����������  
	strcpy(ipBuffer,inet_ntoa(*(in_addr*)*host->h_addr_list));  

	ip = Trim(ipBuffer);
	return true;  
}

bool ServerClient::GetPublicIP(string &ip)//��ȡ����IP
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

void ServerClient::Close()//�ر�
{
	CloseSocket(clientSocket);
	CloseSocket(serverSocket);
	CloseSocket(acceptedSock);
	CloseWSA();
}