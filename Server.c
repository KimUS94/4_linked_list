////��ó: http://remocon33.tistory.com/465
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
HANDLE hMutex;//���ؽ�

//unsigned WINAPI HandleClient(SOCKADDR_IN clientAddr, void* arg);//������ �Լ�
unsigned WINAPI HandleClient(void* arg);//������ �Լ�
void SendMsg(SOCKET clientSock, char* msg, int len);//�޽��� ������ �Լ�(Ŭ���̾�Ʈ���� ���� ��� ������)
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

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) //������ ������ ����ϰڴٴ� ����� �ü���� ����
		ErrorHandling("WSAStartup() error!"); 

	hMutex = CreateMutex(NULL, FALSE, NULL);//�ϳ��� ���ؽ��� �����Ѵ�.
	serverSock = socket(PF_INET, SOCK_STREAM, 0); //�ϳ��� ������ �����Ѵ�.

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(atoi(port));

	if (bind(serverSock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) //������ ������ ��ġ�Ѵ�.
		ErrorHandling("bind() error");
	if (listen(serverSock, 5) == SOCKET_ERROR)//������ �غ���¿� �д�.
		ErrorHandling("listen() error");

	printf("listening...\n");

	while (1) {
		clientAddrSize = sizeof(clientAddr);
		clientSock = accept(serverSock, (SOCKADDR*)&clientAddr, &clientAddrSize);//�������� ���޵� Ŭ���̾�Ʈ ������ clientSock�� ����
		hThread = (HANDLE)_beginthreadex(NULL, 0, HandleClient, (void*)&clientSock, 0, NULL);//HandleClient ������ ����, clientSock�� �Ű������� ����
		printf("Connected Client IP : %s\n", inet_ntoa(clientAddr.sin_addr));
	}
	closesocket(serverSock);//������ ������ ����.
	WSACleanup();//������ ������ �����ϰڴٴ� ����� �ü���� ����
	return 0;
}

#pragma endregion 2012244048 ��溸

#pragma region
//unsigned WINAPI HandleClient(SOCKADDR_IN clientAddr, void* arg)
unsigned WINAPI HandleClient(void* arg)
{
	SOCKET clientSock = *((SOCKET*)arg); //�Ű������ι��� Ŭ���̾�Ʈ ������ ����
//	SOCKADDR_IN clientAddr = *((SOCKET*)arg); //�Ű������ι��� Ŭ���̾�Ʈ ������ ����
	//inet_ntoa(clientAddr.sin_addr);
	File *temp = NULL;
	char fileName[20] = { NULL };
	char ctemp[BUF_SIZE] = { NULL };
	char msg[BUF_SIZE] = { NULL };
	int strLen = 0, i, j = 1, k = 0, flag = 0;

	//[ return value ] 0 : ����
	//[ return value ] n > 0 : ���� ���� ������
	//[ return value ] n = -1 : error ( ���� ���� ��, ���Ͽ��� )
	while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0)  //Ŭ���̾�Ʈ�κ��� �޽����� ���������� ��ٸ���.
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
			
			WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
			current->next = newFile;
			current = current->next;
			ReleaseMutex(hMutex);//���ؽ� ����
			
			//Ŭ���̾�Ʈ�� ���� ������ �޽����� �����ڸ� ���� �����Ͽ� �ڽ��� ����ü�� �����Ѵ�.
			//Ŭ���̾�Ʈ�� ���� ������ �޽����� ����, ����, ������ �����Ѵ�.
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
			closesocket(clientSock);//������ �����Ѵ�.
			return 0;
		}
		if (msg[0] == 'd')
		{
			// �ٿ�ε� ��û �� �ڽ��� ����Ʈ�� ���������� ��� ���Ͽ� Ŭ���̾�Ʈ�� �����ش�.
			//printf("%d Client file download request \n");
			temp = head;
			strcpy(msg, "");
			if (temp->next == NULL)
			{
				strcpy(ctemp, "[Empty file list..]");
				sprintf(msg, "t\n%s", ctemp);
				SendMsg(clientSock, msg, strLen);
				closesocket(clientSock);//������ �����Ѵ�.
				return 0;
			}
			else
			{
				WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
				for (temp = head->next;temp != NULL;temp = temp->next)
				{
					sprintf(autor_title, "[%s]<%s>", temp->autor, temp->title);
					sprintf(autor_title2, "%s\n%s", autor_title2, autor_title);
				}
				ReleaseMutex(hMutex);//���ؽ� ����
				
				sprintf(msg, "t %s", autor_title2);  // �ڽ��� ������ �ִ� ���ϸ���� 
				SendMsg(clientSock, msg, strLen);    // ���� ������ ��� ���� ���ڿ��� Ŭ���̾�Ʈ���� ����

				while ((strLen = recv(clientSock, msg, sizeof(msg), 0)) != 0) // Ŭ���̾�Ʈ�� �����̸��� �Է��ϱ� ��ٸ���.
				{
					if (msg[0] == 'r')
					{
						for (j = 1; j < strLen;j++)
						{
							fileName[k] = msg[j];
							k++;
						}
						fileName[k] = '\0'; // Ŭ���̾�Ʈ�� ���� �����̸��� ������

						WaitForSingleObject(hMutex, INFINITE);//���ؽ� ����
						// ���� ��Ͽ��� �ش� ������ ã�� ����
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
						ReleaseMutex(hMutex);//���ؽ� ����
						if (flag == 0)
							strcpy(msg, "f[Not found file..]");
						SendMsg(clientSock, msg, strLen);

						closesocket(clientSock);//������ �����Ѵ�.
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
#pragma endregion ���Ǽ� ��溸

#pragma region
void SendMsg(SOCKET clientSock, char* msg, int len)
{
	//�ٿ�ε� ��û ����
	if (msg[0] == 't')
	{
		send(clientSock, msg, strlen(msg), 0);
		return 0;
	}

	//�ٿ�ε� �� ���ϸ� ����
	if (msg[0] == 'f')
	{
		send(clientSock, msg, strlen(msg), 0);
		return 0;
	}
}

#pragma endregion 2012244048���Ǽ�