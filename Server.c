////출처: http://remocon33.tistory.com/465
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32");
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <process.h>

#define BUF_SIZE 2000
#define MAX_CLNT 256
#define AUTOR_TITLE_SIZE 20

typedef struct file
{
	char autor[AUTOR_TITLE_SIZE];
	char title[AUTOR_TITLE_SIZE];
	char file[BUF_SIZE];
	struct File* next;
}File;

File *head = NULL, *current = NULL;
HANDLE hMutex;//뮤텍스

//unsigned WINAPI HandleClient(SOCKADDR_IN clientAddr, void* arg);//쓰레드 함수
unsigned WINAPI HandleClient(void* arg);//쓰레드 함수
void SendMsg(SOCKET clientSock, char* msg, int len);//메시지 보내는 함수(클라이언트에게 파일 목록 보내줌)
void ErrorHandling(char* msg);


#pragma region
int main() {
	WSADATA wsaData;
	SOCKET serverSock, clientSock;
	SOCKADDR_IN serverAddr, clientAddr;
	int clientAddrSize;
	HANDLE hThread;
	char port[100];

	head = (File*)malloc(sizeof(File));
	memset(head->autor, '\0', AUTOR_TITLE_SIZE);
	memset(head->title, '\0', AUTOR_TITLE_SIZE);
	memset(head->file, '\0', BUF_SIZE);
	head->next = NULL;
	current = head;

	printf("Input port number : ");
	gets(port);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //윈도우 소켓을 사용하겠다는 사실을 운영체제에 전달
		ErrorHandling("WSAStartup() error!"); 

	hMutex = CreateMutex(NULL, FALSE, NULL);//하나의 뮤텍스를 생성한다.
	serverSock = socket(PF_INET, SOCK_STREAM, 0); //하나의 소켓을 생성한다.

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(atoi(port));

	if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) //생성한 소켓을 배치한다.
		ErrorHandling("bind() error");
	if (listen(serverSock, 5) == SOCKET_ERROR)//소켓을 준비상태에 둔다.
		ErrorHandling("listen() error");

	printf("listening...\n");

	while (1) {
		clientAddrSize = sizeof(clientAddr);
		clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize);//서버에게 전달된 클라이언트 소켓을 clientSock에 전달
		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)&clientSock, 0, NULL);//HandleClient 쓰레드 실행, clientSock을 매개변수로 전달
		printf("Connected Client IP : %s\n", inet_ntoa(clientAddr.sin_addr));
	}
	closesocket(serverSock);//생성한 소켓을 끈다.
	WSACleanup();//윈도우 소켓을 종료하겠다는 사실을 운영체제에 전달
	return 0;
}

#pragma endregion 2012244048 김경보

#pragma region
//unsigned WINAPI HandleClient(SOCKADDR_IN clientAddr, void* arg)
unsigned WINAPI HandleClient(void* arg)
{
	SOCKET clientSock = *((SOCKET*)arg); //매개변수로받은 클라이언트 소켓을 전달
//	SOCKADDR_IN clientAddr = *((SOCKET*)arg); //매개변수로받은 클라이언트 소켓을 전달
	//inet_ntoa(clientAddr.sin_addr);
	File *temp = NULL;
	char fileName[20] = { NULL };
	char ctemp[BUF_SIZE] = { NULL };
	char msg[BUF_SIZE] = { NULL };
	int strLen = 0, i, j = 1, k = 0, flag = 0;

	//[ return value ] 0 : 닫힘
	//[ return value ] n > 0 : 받은 버퍼 사이즈
	//[ return value ] n = -1 : error ( 받지 않을 때, 소켓오류 )
	while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0)  //클라이언트로부터 메시지를 받을때까지 기다린다.
	{
		char autor_title[BUF_SIZE] = { NULL };
		char autor_title2[BUF_SIZE] = { NULL };
		if (msg[0] == 'u')
		{
			File *newFile;
			newFile = (File*)malloc(sizeof(File));
			memset(newFile->autor, '\0', AUTOR_TITLE_SIZE);
			memset(newFile->title, '\0', AUTOR_TITLE_SIZE);
			memset(newFile->file, '\0', BUF_SIZE);
			newFile->next = NULL;
			
			WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
			current->next = newFile;
			current = current->next;
			ReleaseMutex(hMutex);//뮤텍스 중지
			
			//클라이언트로 부터 수신한 메시지를 구분자를 토대로 분해하여 자신의 구조체에 저장한다.
			//클라이언트로 부터 수신한 메시지는 저자, 제목, 내용을 포함한다.
			while (msg[j] != ']')
			{
				if (msg[j] == '[');
				else {
					newFile->autor[k] = msg[j];
					k++;
				}
				j++;
			}
			newFile->autor[k] = '\0';
			k = 0; j++;
			while (msg[j] != '>')
			{
				if (msg[j] == '<' || msg[j] == ']');
				else
				{
					newFile->title[k] = msg[j];
					k++;
				}
				j++;
			}
			k = 0; j++;
			while (msg[j] != '\0')
			{
				if (msg[j] == '>');
				else
				{
					newFile->file[k] = msg[j];
					k++;
				}
				j++;
			}
			newFile->file[j] = '\0';
			closesocket(clientSock);//소켓을 종료한다.
			return 0;
		}
		if (msg[0] == 'd')
		{
			// 다운로드 요청 시 자신의 리스트에 파일제목을 모두 더하여 클라이언트로 보내준다.
			//printf("%d Client file download request \n");
			temp = head;
			strcpy(msg, "");
			if (temp->next == NULL)
			{
				strcpy(ctemp, "[Empty file list..]");
				sprintf(msg, "t\n%s", ctemp);
				SendMsg(clientSock, msg, strLen);
				closesocket(clientSock);//소켓을 종료한다.
				return 0;
			}
			else
			{
				WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
				for (temp = head->next;temp != NULL;temp = temp->next)
				{
					sprintf(autor_title, "[%s]<%s>", temp->autor, temp->title);
					sprintf(autor_title2, "%s\n%s", autor_title2, autor_title);
				}
				ReleaseMutex(hMutex);//뮤텍스 중지
				
				sprintf(msg, "t %s", autor_title2);  // 자신이 가지고 있는 파일목록의 
				SendMsg(clientSock, msg, strLen);    // 파일 제목을 모두 더한 문자열을 클라이언트에게 전송

				while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) // 클라이언트가 파일이름을 입력하길 기다린다.
				{
					if (msg[0] == 'r')
					{
						for (j = 1; j < strLen;j++)
						{
							fileName[k] = msg[j];
							k++;
						}
						fileName[k] = '\0'; // 클라이언트가 보낸 파일이름을 저장함

						WaitForSingleObject(hMutex, INFINITE);//뮤텍스 실행
						// 파일 목록에서 해당 파일을 찾아 전송
						for (temp = head; temp != NULL; temp = temp->next)
						{
							if (strcmp(temp->title, fileName) == 0)
							{
								flag = 1;
								strcpy(fileName, temp->file);
								sprintf(msg, "f\n%s", fileName);
								break;
							}
						}
						ReleaseMutex(hMutex);//뮤텍스 중지
						if (flag == 0)
							strcpy(msg, "f[Not found file..]");
						SendMsg(clientSock, msg, strLen);

						closesocket(clientSock);//소켓을 종료한다.
						return 0;
					}
				}
			}
		}
	}
}
void ErrorHandling(char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(1);
}
#pragma endregion 김의섭 김경보

#pragma region
void SendMsg(SOCKET clientSock, char* msg, int len)
{
	//다운로드 요청 수신
	if (msg[0] == 't')
	{
		send(clientSock, msg, strlen(msg), 0);
		return 0;
	}

	//다운로드 할 파일명 수신
	if (msg[0] == 'f')
	{
		send(clientSock, msg, strlen(msg), 0);
		return 0;
	}
}

#pragma endregion 2012244048김의섭