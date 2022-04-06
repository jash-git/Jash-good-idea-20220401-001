// VC2015CppMultiSocketServer.cpp : 定義主控台應用程式的進入點。
// https://stackoverflow.com/questions/56623685/simple-non-blocking-multi-threaded-tcp-server

//https://shengyu7697.github.io/std-queue/

#include "stdafx.h"
#include <stdio.h>
#include <iostream>
#include <string>
#include <thread>
#include <winsock2.h>
#include <ws2tcpip.h>

#include <mutex>
#include <queue>

using namespace std;

#pragma comment (lib, "ws2_32.lib")

std::mutex g_mutex;

void pause()
{
	printf("Press Enter key to continue..");
	fgetc(stdin);
}

void clientSocketHandler(SOCKET clientSocket, std::string client_ip)
{
	char buf[4096];

	std::thread::id thread_id = std::this_thread::get_id();
	std::cout << thread_id << " - " << client_ip << ": connected" << std::endl;

	while (true)
	{

		ZeroMemory(buf, 4096);

		int bytesReceived = recv(clientSocket, buf, 4096, 0);

		if (bytesReceived == 0)
		{

			std::cout << thread_id << " - " << client_ip << ": disconnected" << std::endl;

			break;

		}

		if (bytesReceived > 0)
		{

			std::cout << thread_id << " - " << client_ip << ": " << std::string(buf, 0, bytesReceived) << std::endl;
			g_mutex.lock();
			//send(clientSocket, buf, bytesReceived + 1, 0);
			g_mutex.unlock();
		}

	}

	std::cout << thread_id << " - " << client_ip << ": closing client socket & exiting thread..." << std::endl;

	closesocket(clientSocket);

}

void waitForConnections(SOCKET serverSocket)
{

	sockaddr_in hint;

	hint.sin_family = AF_INET;
	hint.sin_port = htons(1337);
	hint.sin_addr.S_un.S_addr = INADDR_ANY;

	bind(serverSocket, (sockaddr*)&hint, sizeof(hint));
	listen(serverSocket, SOMAXCONN);
	cout << "TCP Server Listen(port:1337) ..." << endl;

	while (true) {

		sockaddr_in client;

		int clientSize = sizeof(client);

		SOCKET clientSocket = accept(serverSocket, (sockaddr*)&client, &clientSize);

		if (clientSocket != INVALID_SOCKET)
		{

			char host[NI_MAXHOST];      // Client's remote name

			ZeroMemory(host, NI_MAXHOST); // same as memset(host, 0, NI_MAXHOST);

			std::string client_ip = inet_ntop(AF_INET, &client.sin_addr, host, NI_MAXHOST);
			std::thread t(clientSocketHandler, clientSocket, client_ip);

			t.detach();

		}

		Sleep(100);

	}

}

int main()
{
	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);

	if (wsOk != 0)
	{
		std::cerr << "Can't Initialize winsock! Quitting..." << std::endl;

		return 1;
	}

	// Create a socket
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (serverSocket == INVALID_SOCKET)
	{
		WSACleanup();
		std::cerr << "Can't create a socket! Quitting..." << std::endl;

		return 1;
	}

	// Start listening for connections
	waitForConnections(serverSocket);

	// Cleanup winsock
	WSACleanup();

	pause();
    return 0;
}

