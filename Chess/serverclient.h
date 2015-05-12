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
	static void Init();//初始化
	static bool CreateSocket(SOCKET& sock,int type = SOCK_STREAM);//创建socket
	static bool ConnectToServer(SOCKET& sock, string ip, string port);//向服务器发出连接请求
	static bool CloseSocket(SOCKET& sock);//关闭套接字

	static bool BindSocket(SOCKET &sock, string ip, unsigned short port);//将socket绑定到一个IP地址和一个端口上
	static bool SetListen(SOCKET &sock, int backLog);//将socket设置为监听模式等待client连接请求

	static void* CommunicateWithServer(void *args);//和服务器交流
	static void* CommunicateWithClient(void *args);//和客户端交流

	static void* SendPieceMoveMsg(void *args);//传送棋子移动信息
	static void* RecvPieceMoveMsg(void *args);//接收棋子移动信息

	static void CreateThreads();//创建服务器和客户端两个线程
	static void WaitThreads();//等待服务器和客户端两个线程

	static void Close();//关闭

private:
	static bool LoadWSA();//加载socket库
	static bool CloseWSA();//关闭加载的套接字库
	static void PrintServerIPPort();//打印出服务器的相关信息
	static void PrintPieceMoveMsg(const Message &msg, TransmitType transmitType);//打印出棋子移动信息
	static bool GetLocalIP(string &ip);//获取本地IP
	static bool GetPublicIP(string &ip);//获取外网IP

	static SOCKET clientSocket;
	static SOCKET serverSocket;

	static SOCKADDR_IN addrClient;//client的地址信息
	static SOCKADDR_IN addrServer;//server的地址信息
	static SOCKET acceptedSock;//accept返回的用于和client通信的socket
	static int sockaddrSize;//sockaddr结构体的大小

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