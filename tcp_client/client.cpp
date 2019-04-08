#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <vector>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_IP "1.224.129.109"
#define DEFAULT_PORT "27014"

using namespace std;

struct coor {
	int x;
	int y;
};

typedef vector<coor> vc;

vc arr;
coor c;

char str[8][20] = {
	"@==(^0^)@  ",
	" @=(^0^)@  ",
	"  @(^0^)@  ",
	"  @(^0^)=@ ",
	"  @(^0^)==@",
	"  @(^0^)=@ ",
	"  @(^0^)@  ",
	" @=(^0^)@  "
};
char str_erase[20] = "           ";

void gotoxy(coor c) {
	COORD xy = { (short)c.x, (short)c.y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), xy);
}

bool moving() {
	bool ret = false;
	if (GetAsyncKeyState(VK_LEFT) & 0x8000 && c.x > 1) {
		c.x -= 2;
		ret = true;
	}
	if (GetAsyncKeyState(VK_UP) & 0x8000 && c.y > 0) {
		c.y--;
		ret = true;
	}
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000 && c.x < 108) {
		c.x += 2;
		ret = true;
	}
	if (GetAsyncKeyState(VK_DOWN) & 0x8000 && c.y < 28) {
		c.y++;
		ret = true;
	}
	return ret;
}

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET ConnectSocket = INVALID_SOCKET;
	struct addrinfo *result = NULL, hints;
	char sendbuf[200] = "new connection";
	char recvbuf[200];
	int iResult;

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
		return 1;

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	iResult = getaddrinfo(SERVER_IP, DEFAULT_PORT, &hints, &result);
	if (iResult != 0) {
		WSACleanup();
		return 1;
	}

	ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ConnectSocket == INVALID_SOCKET) {
		WSACleanup();
		return 1;
	}

	iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR || iResult == INVALID_SOCKET) {
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	iResult = send(ConnectSocket, sendbuf, 200, 0);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	iResult = recv(ConnectSocket, recvbuf, 200, 0);
	if (iResult <= 0) {
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	closesocket(ConnectSocket);
	
	int index;
	sscanf(recvbuf, "%d", &index);
	//본격 루프
	if (index == -1) {
		printf("연결한도를 초과!!!");
		system("pause");
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}
	int sequence = 0;
	gotoxy({ 0, 29 });
	printf("종료할땐 esc눌러서!!! 방향키로 상하좌우 이동!!!");
	while (1) {
		coor pre = c;
		if (moving()) {
			ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			if (ConnectSocket == INVALID_SOCKET) {
				WSACleanup();
				return 1;
			}
			iResult = connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
			if (iResult == SOCKET_ERROR || iResult == INVALID_SOCKET) {
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			sprintf(sendbuf, "%d %d %d", index, c.x, c.y);
			iResult = send(ConnectSocket, sendbuf, 200, 0);
			if (iResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				system("pause");
				closesocket(ConnectSocket);
				WSACleanup();
				return 1;
			}
			closesocket(ConnectSocket);
		}

		gotoxy(pre);
		printf("%s", str_erase);
		gotoxy(c);
		printf("%s", str[sequence++]);

		if (sequence == 8)
			sequence = 0;

		if (GetAsyncKeyState(VK_ESCAPE)) {
			ConnectSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
			connect(ConnectSocket, result->ai_addr, (int)result->ai_addrlen);
			sprintf(sendbuf, "%d %d %d", index, -1, -1);
			send(ConnectSocket, sendbuf, 200, 0);
			closesocket(ConnectSocket);
			WSACleanup();
			return 0;
		}

		_sleep(50);
	}

	freeaddrinfo(result);

	iResult = shutdown(ConnectSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		closesocket(ConnectSocket);
		WSACleanup();
		return 1;
	}

	WSACleanup();

	return 0;
}