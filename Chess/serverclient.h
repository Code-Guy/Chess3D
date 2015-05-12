#ifndef _SERVERCLIENT_H
#define  _SERVERCLIENT_H

#include <Winsock2.h>
#include <pthread.h>
#include "packet.h"
#include "player.h"

#define DEFAULT_PORT 12345
#define DEFAULT_IP "127.0.0.1"
#define DEFAULT_BACKLOG 10

enum Role
{
	Client, Server
};

enum TransmitType
{
	Send, Recv
};

class ServerClient
{
public:
	static void Init();//��ʼ��
	static bool CreateSocket(SOCKET& sock,int type = SOCK_STREAM);//����socket
	static bool ConnectToServer(SOCKET& sock, string ip, string port);//�������������������
	static bool CloseSocket(SOCKET& sock);//�ر��׽���

	static bool BindSocket(SOCKET &sock, string ip, unsigned short port);//��socket�󶨵�һ��IP��ַ��һ���˿���
	static bool SetListen(SOCKET &sock, int backLog);//��socket����Ϊ����ģʽ�ȴ�client��������

	static void* CommunicateWithServer(void *args);//�ͷ���������
	static void* CommunicateWithClient(void *args);//�Ϳͻ��˽���

	static void* SendPieceMoveMsg(void *args);//���������ƶ���Ϣ
	static void* RecvPieceMoveMsg(void *args);//���������ƶ���Ϣ

	static void CreateThreads();//�����������Ϳͻ��������߳�
	static void WaitThreads();//�ȴ��������Ϳͻ��������߳�

	static void Close();//�ر�

private:
	static bool LoadWSA();//����socket��
	static bool CloseWSA();//�رռ��ص��׽��ֿ�
	static void PrintServerIPPort();//��ӡ���������������Ϣ
	static void PrintPieceMoveMsg(const Message &msg, TransmitType transmitType);//��ӡ�������ƶ���Ϣ
	static bool GetLocalIP(string &ip);//��ȡ����IP
	static bool GetPublicIP(string &ip);//��ȡ����IP

	static SOCKET clientSocket;
	static SOCKET serverSocket;

	static SOCKADDR_IN addrClient;//client�ĵ�ַ��Ϣ
	static SOCKADDR_IN addrServer;//server�ĵ�ַ��Ϣ
	static SOCKET acceptedSock;//accept���ص����ں�clientͨ�ŵ�socket
	static int sockaddrSize;//sockaddr�ṹ��Ĵ�С

	static bool hasConnectedToServer;
	static bool hasConnectedWithClient;

	static string serverIP;
	static int serverPort;

	static string localIP;
	static string globalIP;

	static char hostName[DEFAULT_BUFFER_SIZE];

	static pthread_t clientTid;
	static pthread_t serverTid;

	static Ally *ally;
	static Rival *rival;
};
#endif //_SERVERCLIENT_H