#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <Windows.h>
#include <process.h>
#pragma warning(disable:4996)
#pragma comment(lib,"ws2_32");

#define BUF_SIZE 2000
#define NAME_SIZE 20

char name[NAME_SIZE] = "[DEFAULT]";
char file[NAME_SIZE] = "[DEFAULT]";
char msg[BUF_SIZE] = { NULL };

typedef struct node
{
	int data;
	int empt;
	struct node *pre;
	struct node *up;
	struct node *down;
	struct node *next;
}NODE;

int listFlag = 0;
int fileNameFlag = 0;

NODE* insert(NODE *head, NODE *tail, NODE *current, int data);  // 노드 삽입 함수       _ 김의섭
NODE* Delete(NODE * head, NODE *tail, NODE *current, int data); // 노드 삭제 함수       _ 김의섭
void Exit(NODE *head, NODE *tail);                              // 메모리 해제 함수     _ 김의섭

NODE* Move(NODE *head, NODE *tail, NODE *current, int data);   // 커서 이동 함수        _ 김경보
NODE* FileIO(NODE* head, NODE* tail, int data);				   // 파일 입출력 함수      _ 김경보
void FileRenewal(char *fname);                                 // 파일 재배치 함수      _ 김경보
void setCusor(NODE *head, NODE* current);                      // 커서 출력 함수        _ 김경보
void ViewAll(NODE *head, NODE *tail);                          // 노드 리스트 출력 함수 _ 김경보


NODE* FileUpload(NODE *head, NODE* tail);						// 업로드 요청 함수
unsigned UpSendMsg(SOCKET sock);								// 업로드 요청 메세지 전송함수

NODE* FileDownload(NODE *head, NODE* tail);						// 다운로드 요청 함수
unsigned DownSendMsg(SOCKET sock);								// 다운로드 요청 메세지 전송함수
unsigned WINAPI DownRecvMsg(void* arg);							// 다운로드 요청 메세지 수신함수


NODE *head = NULL, *tail = NULL, *current = NULL;

int main()
{
	int i;
	head = (NODE*)malloc(sizeof(NODE));
	tail = (NODE*)malloc(sizeof(NODE));
	head->data = NULL; head->up = NULL; head->down = NULL; head->pre = NULL;
	tail->data = NULL; tail->up = NULL; tail->down = NULL; tail->next = NULL;
	head->next = tail;	tail->pre = head;
	current = head;
	ViewAll(head, tail);

	while (1)
	{
		i = getch();
		system("cls");
		if (i == 224)
		{
			i = getch();
			/*
			[end]	224 79		[home]	224	71		[Del]	224 83
			[↑]	224 72
			[←] 224 75      [↓]	224 80		224 7711 [→]
			*/
			if (i == 72 || 80 || 75 || 77 || 79 || 71)
				current = Move(head, tail, current, i);

			else if (i == 83)
				Delete(head, tail, current, i);
		}
		else if (i == 0)
		{
			i = getch();
			if (i == 0) break;
			else if (i == 63)
			{
				Exit(head, tail);
				return 0;
			}
			else FileIO(head, tail, i);
		}
		else if (i == 8) // BackSpace
			current = Delete(head, tail, current, i);

		else current = insert(head, tail, current, i);

		ViewAll(head, tail);
		setCusor(head, current);
	}
	Exit(head, tail);
	return 0;
}

#pragma region
void ViewAll(NODE *head, NODE *tail)
{
	NODE *temp = (NODE*)malloc(sizeof(NODE));
	printf("[F1]FileOpen   [F2]FileSave   [F3]FileUproad   [F4}Filedownload   [F5]Exit\n");
	printf("--------------------------------------------------------------------------\n");
	for (temp = head; temp != tail; temp = temp->next)
	{
		if (temp->data == NULL);
		else printf("%c", temp->data);
	}
	// 반복적으로 함수를 호출하며 head ~ tail 까지 출력
}

void setCusor(NODE *head, NODE* current)
{
	int x = 0, y = 2;
	NODE *temp = head;
	while (temp != current)
	{
		if (temp->data == 10) // 10 = enter 키값 즉 enter입력 시 커서 y축으로 증가(아래로 내려감)
		{
			x = 0;
			y++;
		}
		else x++;
		temp = temp->next;
	}
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos); // 함수 사용 http://kkikkodev.tistory.com/26 참고
																	// while문 내에서 리스트에 저장된 값에 따라 x, y 좌료를 증감 시키고 해당하는 x,y 좌표로 커서를 이동 시켜주는 함수
}

NODE* FileIO(NODE* head, NODE* tail, int data)
{
	char fname[50], ch;
	FILE *fp_in, *fp_out;
	NODE* current = head;

	// F1 입력 시
	if (data == 59)
	{
		printf("Plese enter the File name_");
		scanf("%s", fname);
		system("cls");
		FileRenewal(fname); // 파일을 재배치 함. ex) 읽어들인 파일의 각 줄마다 글자 수를 체크 하여 80글자가 넘어 가면 개행문자를 더해 파일을 재 구성
		fp_in = fopen(fname, "r");
		if (fp_in == NULL) { // 예외처리
			printf("File Error\n");
			return head;
		}
		while (!feof(fp_in)) // 파일을 한글자 씩 읽어 리스트에 추가
		{
			fscanf(fp_in, "%c", &ch);
			current = insert(head, tail, current, ch);
		}
		fclose(fp_in);
		return head;
	}
	//F2 입력 시
	if (data == 60)
	{
		printf("Plese enter the SaveFile name_");
		scanf("%s", fname);
		system("cls");
		fp_out = fopen(fname, "w");
		if (fp_out == NULL) { //예외처리
			printf("File Error\n");
			return head;
		}
		while (current != tail) //current(head) ~ tail까지 리스트에 달린 노드를 파일에 출력
		{
			if (current->data == NULL);
			else fprintf(fp_out, "%c", current->data);
			current = current->next;
		}
		fclose(fp_out);
		return tail->pre;
	}
	//F3 입력 시
	if (data == 61)
		FileUpload(head, tail);
	//F4 입력 시
	if (data == 62)
		FileDownload(head, tail);
}

void FileRenewal(char *fname)
{
	FILE *fp_in, *fp_out;
	char str[5000];
	char ch;
	int i = 0, count = 0;
	fp_in = fopen(fname, "r");

	if (fp_in == NULL) { // 예외처리
		printf("File Error\n");
		return;
	}
	while ((ch = fgetc(fp_in)) != EOF) // 파일의 끝까지 파일을 한글자 씩 읽어 들임
	{
		if (ch != '\n')	count++; // 개행문자를 만나기 전까지 카운터 증가
		else count = 0; // 개행문자를 만나면 카운터 초기화

		if (i == 0) str[i] = ch;
		else if (count == 80)
		{
			str[i] = '\n'; // count == 80 즉 현재 줄의 문자가 80자 이상이라면 개행문자를 더함
			str[i + 1] = ch; // 이번에 읽어 들인 문자는 개행의 다음번에 추가
			count = 0;
			i++;
		}
		else str[i] = ch;
		i++;
	}
	str[i] = '\0';
	fp_out = fopen(fname, "w");
	fprintf(fp_out, "%s", str); // 재구성한 문자열(str)을 기존파일에 다시 저장

	fclose(fp_in);
	fclose(fp_out);
}

NODE* Move(NODE *head, NODE *tail, NODE *current, int data)
{
	/* Move 함수에 구현되어 있는 기능 키들은 입력시 ASCII 코드가 2byte로 입력 되기 때문에 getch()로 입력 받을 경우
	항상 첫번째 바이트는 224로 시작함 때문에 224가 입력되면 Move함수로 이동 되며 이후 두번쨰 바이트(버퍼에 남아있는) 값에 따라 해당 루틴 실행 */

	// ↑ 입력 시 current->up 리턴
	if (data == 72)
	{
		if (current->up == NULL);
		else current = current->up;
	}
	// ← 입력 시 current->pre 리턴
	if (data == 75)
	{
		if (current->pre == head || current->pre == NULL)
			return head;
		else current = current->pre;
	}
	// → 입력 시 current->next 리턴
	if (data == 77)
	{
		if (current->next == tail || current->next == NULL);
		else current = current->next;
	}
	// ↓ 입력 시 current->down 리턴
	if (data == 80)
	{
		if (current->down == NULL);
		else current = current->down;
	}

	//Home 입력 시 각 줄의 첫번째 노드 리턴,  2012244030 김의섭(제공)
	if (data == 71) // Home
	{
		while (current != head && current->pre->data != 10)
			current = current->pre;
	}
	//End 입력 시 각 줄의 마지막 노드 리턴(만약 마지막 노드가 tail 이라면 tail->pre 리턴),  2012244030 김의섭(제공)
	if (data == 79) // End
	{
		while (current->next != tail && current->next->data != 10)
			current = current->next;
		if (current == tail) return current->pre;
	}

	return current;

}
#pragma endregion 2012244048 김경보

#pragma region
void Exit(NODE *head, NODE *tail)
{
	// head ~ tail 까지 재귀적으로 호출하며 노드에 할당되어 있는 메모리 공간 해제
	// 할당괸 모든 공간을 해제 하면 프로그램 종료
	/*if (head == tail) return;
	else
	{
		Exit(head->next, tail);
		free(head);
	}*/
	NODE* temp;
	for (temp = head;temp != tail;temp = temp->next)
	{
		if (temp == head || tail);
		else free(temp);
	}
	return 0;
}

NODE* Delete(NODE * head, NODE *tail, NODE *current, int data)
{
	NODE *temp, *temp2, *temp3;
	NODE *follow = current;
	if (current != tail && current != NULL) current = current->next;
	if (data == 8) //BackSpace
	{
		if (follow == head) return follow;
		else
		{
			//삭제될 노드(follow)의 pre와 next를 연결
			follow->pre->next = current;
			current->pre = follow->pre;

			//follow(삭제할 문자)가 개행문자인 경우
			if (follow->data == 10)
			{
				if (current->down != NULL)  // follow->data 가 개행 문자 이면서 current(follow->next)가 가르키는 down이 있는 경우
				{

					temp = follow;
					while (temp->pre != head && temp->pre->data != 10)  // follow의 줄에 첫번째 노드로 temp를 이동 시킴
						temp = temp->pre;

					temp2 = current->down;
					while (temp->data != 10 && temp != NULL) //temp를 그 줄의 끝까지 이동 시키며 
					{
						if (temp2 != tail && temp2->data != 10) //temp2가 그 줄의 끝이 아니라면
						{
							temp->down = temp2;                 //temp와 temp2 up, down 연결
							temp->down->up = temp;
						}
						else temp->down = temp->pre->down;  // temp2가 그 줄의 끝이라면 temp의 down을 temp->pre의 down으로

						if (temp2 != tail && temp2->data != 10) temp2 = temp2->next; //temp2가 그 줄의 끝이 아니라면 temp2 증가
						temp = temp->next;
					}
				}
				else // follow->data 가 개행 문자 이면서 current(follow->next)가 가르키는 down이 없는 경우
				{
					if (follow->up != NULL) temp = follow->up;
					else temp = follow; //temp->up이 null이 아니면 temp가 follow를 가르킴

					while (temp->pre != head && temp->pre->data != 10) // temp를 그 줄의 첫번째 노드로 이동 시킴
						temp = temp->pre;

					temp2 = current;
					while (temp2->pre != head && temp2->pre->data != 10) //temp2(current)를 그 줄의 첫번째로 이동 시킴
						temp2 = temp2->pre;


					while (temp2->data != 10 && temp2 != NULL && temp2 != tail) //temp2(current) 가 그 줄의 끝이 아닌 동안 반복
					{
						if (temp->data != 10 && temp != NULL) //temp가 그 줄의 끝이 아니면 temp와 temp2 연결
						{
							temp->down = temp2;
							temp2->up = temp;
						}
						else temp2->up = temp2->pre->up; // temp가 그 줄의 끝이면 temp2->up은 temp2 이전의 업
						if (temp != NULL && temp->data != 10) temp = temp->next;
						temp2 = temp2->next;
					}
				}
				free(follow);
			}

			//follow(삭제할 문자)가 개행문자가 아닌 경우
			else
			{

				if (follow->down != NULL)
				{
					// 이 경우 follow가 있는 줄이 맨 첫번째 줄 이기 때문에 follow->up(temp->up)은 고려할 필요 없음
					if (follow->up == NULL)
					{
						//temp, temp2 사용
						temp = follow;
						while (temp->pre != head && temp->pre->data != 10) // temp(follow)를 그줄의 첫번 째로 이동 시킴
							temp = temp->pre;

						temp2 = follow->down;
						while (temp2->pre != head && temp2->pre->data != 10) //temp2(follow->down)를 그 줄의 첫번째 노드로 이동 시킴
							temp2 = temp2->pre;

						while ((temp->data != 10 && temp != NULL) || (temp2->data != 10 && temp2 != tail && temp2 != NULL)) // temp, temp2가 그줄의 끝이 아닌 동안
						{
							//temp와 temp2가 그 줄의 끝이 아니라면 서로의 up, down 연결 시킴(temp와 temp2는 각 줄의 첫번쨰 노드 부터 같은 속도로 증가)
							if ((temp->data != 10 && temp != NULL) && (temp2->data != 10 && temp2 != tail && temp2 != NULL))
							{
								temp2->up = temp;
								temp->down = temp2;
							}
							// temp와 temp2중 하나로도 그 줄의 끝이 라면 그에 맞는 처리를 함
							else
							{
								if ((temp->data == 10 || temp == NULL))
									temp2->up = temp2->pre->up;

								if (temp2->data == 10 || temp2 == tail && temp2 == NULL)
									temp->down = temp2->pre;
							}
							if (temp->data != 10 && temp != NULL) temp = temp->next;
							if (temp2->data != 10 && temp2 != tail && temp2 != NULL) temp2 = temp2->next;
						}
					}

					// follow의 up 과 down이 모두 null이 아닌 경우로 3줄의 상하 관계를 고려 해야함
					else
					{
						//temp3, temp, temp2 사용
						temp = follow;
						temp2 = follow->down;
						temp3 = follow->up;
						while (temp->pre != head && temp->pre->data != 10)
							temp = temp->pre;
						while (temp2->pre != head && temp2->pre->data != 10)
							temp2 = temp2->pre;
						while (temp3->pre != head && temp3->pre->data != 10)
							temp3 = temp3->pre;
						// 각 줄의 첫 노드로 temp3(윗줄), temp, temp2(아랫줄) 이동시킴

						while ((temp->data != 10 && temp != NULL) || (temp2->data != 10 && temp2 != NULL && temp2 != tail) ||
							(temp3->data != 10 && temp3 != NULL)) //temp, temp2, temp3가 모두 각 줄의 끝에 도달하면 반복문 종료
						{
							// 000(temp, temp2, tmep3 모두 각줄의 끝이 아닌 경우)
							if ((temp->data != 10 && temp != NULL) && (temp2->data != 10 && temp2 != tail && temp2 != NULL)
								&& (temp3->data != 10 && temp3 != NULL))
							{
								temp2->up = temp;
								temp->up = temp3;
								temp->down = temp2;
								temp3->down = temp;
							}

							// temp, temp2, tmep3 중 하나라도 각 줄의 끝에 도달 한 경우
							// temp, temp2, temp3 가 각 줄의 끝에 도달 하고 안하고를 0,1로 치환 하면 8가지 경우의 수가 있음 
							// 여기서 000(모두 각줄의 끝이 아닌 경우)는 위에서 처리 하였으므로 밑에서 나머지 6가지 처리
							// (111의 경우 반복문이 종료되므로 처리 하지 않음)
							else
							{

								if (temp->data == 10)
								{
									if (temp2->data == 10 || temp2 == tail) // 110
									{
										temp3->down = temp->pre;
										if (temp3->next->data == 10)
											temp3->next->down = temp->pre;
									}
									else if (temp3->data == 10) //101
									{
										temp3->down = temp->pre;
										temp2->up = temp->pre;
									}
									else // 100
									{
										temp3->down = temp->pre;
										if (temp3->next->data == 10)
											temp3->next->down = temp->pre;
										temp->up = temp->pre->up;
										temp->down = temp->pre->down;
									}
								}
								else
								{
									if ((temp3->data == 10) && (temp2->data == 10 || temp2 == tail)) // 011
									{
										temp->up = temp->pre->up;
										temp->down = temp->pre->down;
										if (temp->next->data == 10)
											temp->next->down = temp->pre->down;
									}
									else if (temp2->data == 10 || temp2 == tail) // 010
									{
										temp->up = temp3;
										temp3->down = temp3->pre->down;
										temp->down = temp2->pre;
										if (temp->next->data == 10)
											temp->next->down = temp->pre->down;
									}
									else if (temp3->data == 10) //001
									{
										temp->up = temp->pre->up;
										temp->down = temp2;
										temp2->up = temp;
										if (temp->next->data == 10)
											temp->next->down = temp->pre->down;
									}
								}
							}

							if (temp->data != 10 && temp != NULL) temp = temp->next;
							if (temp2->data != 10 && temp2 != tail && temp2 != NULL) temp2 = temp2->next;
							if (temp3->data != 10 && temp3 != NULL) temp3 = temp3->next;
						}
					}
					free(follow);
				}
				else
				{
					//follow의 up과 down이 모두 null 인 경우 즉, 문자열이 한줄인 경우
					if (follow->up == NULL)
					{
						follow->pre->next = current;
						current->pre = follow->pre;
						free(follow);
					}

					// 이 경우 follow가 맨 아랫 줄 이기 때문에 follow->down은 고려할 필요가 없음
					else //follow->up != 일때
					{
						temp = follow->up;
						while (temp->pre != head && temp->pre->data != 10)
							temp = temp->pre;

						temp2 = follow;
						while (temp2->pre != head && temp2->pre->data != 10)
							temp2 = temp2->pre;

						while ((temp->data != 10 && temp != NULL) || (temp2->data != 10 && temp2 != tail && temp2 != NULL))
						{
							if ((temp->data != 10 && temp != NULL) && (temp2->data != 10 && temp2 != tail && temp2 != NULL))
							{
								temp2->up = temp;
								temp->down = temp2;
							}
							else
							{
								if ((temp->data == 10 || temp == NULL))
									temp2->up = temp2->pre->up;

								if (temp2->data == 10 || temp2 == tail || temp2 == NULL)
									temp->down = temp2->pre;
							}
							if (temp->data != 10 && temp != NULL) temp = temp->next;
							if (temp2->data != 10 && temp2 != tail && temp2 != NULL) temp2 = temp2->next;
							if (temp->next->data == 10) temp->next->down = temp->pre->down;
						}
						free(follow);
					}
				}
			}
		}
	}
	else if (data == 83) //Del
	{
	}

	return current->pre;
}

NODE* insert(NODE *head, NODE *tail, NODE *current, int data)
{
	NODE *follow = current;  // 모든 문자는 follow 뒤에 current 앞에 삽입됨
	current = current->next;
	NODE *Up_temp = NULL, *temp2 = NULL, *temp = NULL, *empty = NULL;
	NODE *newnode = (NODE*)malloc(sizeof(NODE));

	if (data == 13 || data == 10) data = 10; // 키보드로 입력하는 enter의 ASCII는 13 '\n'문자의 ASCII는 10 따라서 모든 개행 문자를 10으로 저장
	newnode->data = data;
	newnode->pre = NULL, newnode->down = NULL, newnode->up = NULL, newnode->next = NULL;

#pragma region 개행문자(enter//13//10) 삽입 시
	if (data == 13 || data == 10)
	{
		/*  개행문자 삽입 시 개행문자 뒤에 tail이 온다면 개행문자 뒤에 아무것도 존재 하지 않아
		포커스 함수 호출시 커서가 빈 공간을 가르키지 못하기 때문에 empty노드를 할당하여 빈 공간 표현 */
		if (current == tail)
		{
			empty = (NODE*)malloc(sizeof(NODE));

			follow->next = newnode;
			newnode->pre = follow;
			newnode->next = empty;
			newnode->up = follow->up;
			newnode->down = empty;
			tail->pre = empty;
			temp = follow;

			while ((temp->data != 10) && (temp != head))  //각 줄의 시작으로 temp를 이동 시킴
			{
				temp->down = empty;
				temp = temp->pre;
			}

			empty->data = NULL;
			empty->up = temp->next;
			empty->next = tail;
			empty->pre = newnode;
			tail->pre = empty;
			empty->down = NULL;
			empty->empt = 1;

			return empty;
		}

		// 개행문자(삽입될 문자) 뒤(current)에 tail이 오지 않을 때
		else
		{
			follow->next = newnode;
			newnode->pre = follow;
			newnode->next = current;
			current->pre = newnode;
			newnode->up = current->up;
			newnode->down = current;

			temp = current;
			while (temp->data != data)  //각 줄의 시작으로 temp를 이동 시킴
			{
				temp = temp->pre;
			}

			if (current->next == NULL || current->next == tail) // current가 각 줄의 끝 인 경우
			{
				current->up->down = newnode;
				current->up = temp;
				temp->down = current;
				current->down = NULL;
			}
			else
			{
				if (current->up != NULL) current->up->down = newnode;
				current->up = temp;
				temp->down = current;
				if (current->down != NULL) current->down->up = current;
				current->next->up = follow;
			}
		}

		return newnode;
	}
#pragma endregion 

#pragma region 문자 삽입 시		
	else
	{
		// 문자 입력 시 문자의 입력 위치가 첫번째 노드 인 경우(head->next 인 경우)
		if (follow == head)
		{
			follow->next = newnode;
			newnode->pre = follow;
			newnode->next = current;
			current->pre = newnode;

			// newnode의 뒤에(current) 기존에 입력한 노드가 있다면
			if (current != NULL && current != tail)
			{
				newnode->up = NULL;
				newnode->down = current->down;
				if (newnode->down != NULL) newnode->down->up = newnode;

				temp = current;

				if (temp->down == NULL) temp->down = NULL;
				else
				{
					while (temp->data != 10 && temp != tail)
					{
						temp->up = NULL;
						if (temp->next->data == 10 || temp->down->next == tail);
						else
						{
							temp->down = temp->next->down;
							temp->down->up = temp;
						}
						if (temp->data == 10)	temp->down = temp->pre->down;

						temp = temp->next;
					}
				}
			}
		}

		else
		{
			/* 삽입 하려는 위치의 노드에 empt멤버 값이 1 인 경우(위 개행문자 입력시 등장 한 empyt노드인 경우)
			newnode를 새로 추가 할 필요 없으 data를 empty노드에 저장하고 empt멤버를 1이 아닌 수로 바꿔줌 */
			if (follow->empt == 1)
			{
				follow->empt = 0;
				follow->data = data;
				return follow;
				free(newnode);
			}

			// 예외적인 노드(empty노드)가 아닌 경우
			else
			{
				follow->next = newnode;
				newnode->pre = follow;

				// 항상 down이 없기 때문에 고려안해도됨
				if (current == tail)
				{
					tail->pre = newnode;
					newnode->next = tail;
					newnode->down = NULL;
					//follow->up == NULL 이며 current==tail 이란 소린 즉, 문자열이 한줄 인 경우
					if (follow->up == NULL)	newnode->up = NULL;

					/* current는 tail이지만 follow의 up은 존재 하기 때문에 down은 고려할 필요가 없지만 up은 고려해야함
					즉 follow줄과 follow->up 줄 간의 연결만 고려 */
					else
					{
						// temp(윗줄)가 줄의 끝이 아니라면
						if ((follow->up->next->data != 10) && (follow->up->next != NULL))
						{
							newnode->up = follow->up->next;
							newnode->up->down = newnode;
							temp = newnode->up;
							while ((temp->data != 10)) // 윗줄이 끝이 아닌 동안 반복하며 윗줄고 아랫줄 연결
							{
								temp->down = newnode;
								temp = temp->next;
								if (temp->data == 10) temp->down = newnode;
							}
						}

						// temp(윗줄)가 줄의 끝이 라면
						else
						{
							if (follow->up->next->data != 10 && follow->up->next != NULL)
							{
								newnode->up = follow->up->next;
								newnode->up->down = newnode;
							}
							else newnode->up = follow->up;
						}
					}
				}

				// 상황에 따라 2줄 혹은 3줄을 고려 해야함
				else if (current != tail)
				{
					newnode->next = current;
					current->pre = newnode;

#pragma region 
					if (current->up == NULL) newnode->up = NULL;

					else
					{
						newnode->up = current->up;
						newnode->up->down = newnode;
						temp = current;

						// temp(current)가 각 줄의 끝이 아닌 동안 반복
						while ((temp->data != 10) && (temp != tail))
						{
							if (newnode->up->next->data == 10);
							else temp->up = temp->next->up;
							temp = temp->next;
						}
					}
#pragma endregion newnode->up 처리
#pragma region 
					if (current->down == NULL) newnode->down = NULL;
					else //if (follow->down != NULL)
					{
						// 그림을 그리며... 의식의 흐름대로 짜서 글로 설명하기 어렵습니다...
						if (follow->down->next != tail && follow->down->next != NULL && follow->down->next->data != 10)
						{
							temp = follow->next;
							temp2 = follow->down->next;

							while (temp2->data != 10 && temp2 != tail && temp2 != NULL)
							{
								newnode->down = follow->down->next;
								newnode->down->up = newnode;

								if (temp->next != NULL && temp->next->data != 10)
								{
									if (temp2->next->next != tail && temp2->next->next != NULL && temp2->next->next->data != 10)
										temp->next->down = temp2->next;
									else temp->next->up = temp;

									temp = temp->next;
								}
								else if (temp->next->data == 10)
								{
									temp->next->down = temp2;
									if (temp2 != tail) temp2->up = temp;
								}
								temp2 = temp2->next;
							}
						}
						else newnode->down = follow->down;
					}
#pragma endregion newnode->down 처리
				}
			}
		}
		return newnode;
	}
#pragma endregion
}
#pragma endregion 2012244030 김의섭

#pragma region
//파일 업로드 함수
NODE* FileUpload(NODE *head, NODE* tail)
{
	WSADATA wsaData;// WSADATA struct는 window sockets 초기화 정보를 저장하는데 사용됨
	SOCKET sock;
	SOCKADDR_IN serverAddr;
	/*IPv4 주소체계에서 사용하는 구조체다. 소켓 프로그램은 범용 주소 구조체로 sockaddr을 사용하지만, 
	  주소체계의 종류에 따라 별도의 전용 구조체를 만들어 사용하는게 아무래도 편리할 것이다. 
	  참고로, 기타 다른 주소체계 중 Local Unix 주소 체계는 sockaddr_un 구조체를 사용한다.
	  [출처] sockaddr, sockaddr_in 구조체|작성자 괄목상대 */ 
	char myIp[100];
	char port[100];
	char inputName[100];
	char fileName[100];

	printf("Input server IP : ");
	gets(myIp);

	printf("Input server port : ");
	gets(port);

	printf("Input your name : ");
	gets(inputName);

	printf("Input your file name : ");
	gets(fileName);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// 윈도우 소켓을 사용한다고 운영체제에 알림
	{
		fputs("Connect() error.... \n", stderr);
		printf("Please press any key to continue....");
		getch();
		system("cls");
		return 0;
	}
	sprintf(name, "[%s]", inputName);
	sprintf(file, "<%s>", fileName);
	sock = socket(PF_INET, SOCK_STREAM, 0);//소켓을 하나 생성한다.

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(myIp);
	serverAddr.sin_port = htons(atoi(port));

	if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)// 서버에 접속한다.
	{
		fputs("Connect() error.... \n", stderr);
		printf("Please press any key to continue....");
		getch();
		system("cls");
		return 0;
	}
	UpSendMsg(sock); // 업로드 요청 메세지 전송 함수 호출
	
	printf("File uploading complete!\n");

	printf("Please press any key to continue....");
	getch();
	system("cls");
	closesocket(sock);//소켓을 종료한다.
	WSACleanup();//윈도우 소켓 사용중지를 운영체제에 알린다.
	return 0;
}
//업로드 요청 메세지 전송 함수 (글쓴이, 제목, 내용을 합쳐 서버로 전송)
unsigned UpSendMsg(SOCKET sock)
{
	char nameMsg[NAME_SIZE + NAME_SIZE + BUF_SIZE + 1];
	NODE *temp = head;
	int i = 0;
	for (temp = head->next; temp != tail; temp = temp->next)
	{
		msg[i] = temp->data;
		i++;
	}
	msg[i] = '\0';
	sprintf(nameMsg, "u%s%s%s", name, file, msg); // 업로드할 파일의 이름과 작성자, 내용을 하나의 문자열로 합친다.
	send(sock, nameMsg, strlen(nameMsg), 0);//nameMsg를 서버에게 전송한다.

	return 0;
}


// 다운로드 요청 함수
NODE* FileDownload(NODE *head, NODE* tail)
{
	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN serverAddr;
	HANDLE recvThread; // 커널오브젝트를 건드리기 위한 구조체
	char *down = "down";
	char myIp[100];
	char port[100];
	char inputName[100];
	char fileName[100];

	printf("Input server IP : ");
	gets(myIp);

	printf("Input server port : ");
	gets(port);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// 윈도우 소켓을 사용한다고 운영체제에 알림
	{
		fputs("Connect() error.... \n", stderr);
		printf("Please press any key to continue....");
		getch();
		system("cls");
		return 0;
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);//소켓을 하나 생성한다.

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(myIp);
	serverAddr.sin_port = htons(atoi(port));

	if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)//서버에 접속한다.
	{
		fputs("Connect() error.... \n", stderr);
		printf("Please press any key to continue....");
		getch();
		system("cls");
		return 0;
	}

	send(sock, down, strlen(down), 0);
	recvThread = (HANDLE)_beginthreadex(NULL, 0, DownRecvMsg, (void*)&sock, 0, NULL);//메시지 수신용 쓰레드가 실행된다.
	WaitForSingleObject(recvThread, INFINITE);//수신용 쓰레드가 중지될때까지 기다린다.

	//파일 리스트가 empty 여서 리턴되면 아랫줄이 실행안되게 해야함
	if (listFlag != 1)
	{
		recvThread = (HANDLE)_beginthreadex(NULL, 0, DownRecvMsg, (void*)&sock, 0, NULL);//메시지 수신용 쓰레드가 실행된다.
		WaitForSingleObject(recvThread, INFINITE);//수신용 쓰레드가 중지될때까지 기다린다.
	}

	//클라이언트가 종료를 시도한다면 이줄 아래가 실행된다.
	if (fileNameFlag == 1)
		printf("File download error!\n");
	else
		printf("File download complete!\n");

	printf("Please press any key to continue....");
	getch();
	system("cls");
	closesocket(sock);//소켓을 종료한다.
	WSACleanup();//윈도우 소켓 사용중지를 운영체제에 알린다.
	listFlag = 0;
	fileNameFlag = 0;
	return 0;
}
#pragma endregion 김의섭 김경보

#pragma region
// 다운로드 요청을 서버로 전송 하는 함수
unsigned DownSendMsg(SOCKET sock)
{
	char fileName[20];
	char fmsg[21];
	if (msg[0] == 't')
	{
		printf("Input you want file name : ");
		gets(fileName);
		sprintf(fmsg, "r%s", fileName);
		send(sock, fmsg, strlen(fmsg), 0);//nameMsg를 서버에게 전송한다.
	}
	return 0;
}
#pragma endregion 2012244048 김경보

#pragma region
// 다운로드 요청 시 서버가 전송해 주는 데이터를 처리하는 함수 
unsigned WINAPI DownRecvMsg(void* arg)
{
	HANDLE sendThread;
	SOCKET sock = *((SOCKET*)arg);//서버용 소켓을 전달한다.
	NODE* temp;
	char autor_title[BUF_SIZE];
	int strLen, i;
	while (1)
	{
		strLen = recv(sock, msg, BUF_SIZE - 1, 0);// 서버로부터 메시지를 수신한다.
		if (strLen == -1)	return -1;            // 예외처리
		msg[strLen] = '\0';

		//파일 리스트를 받은 경우
		if (msg[0] == 't')
		{
			printf("=================================\n");
			printf("[File list] | [autor] <fileName> \n");
			printf("=================================");
			if (msg[3] == 'E') // 서버의 파일 리스트가 비어 있다면
			{
				listFlag = 1;
				printf("\nThe list of files is empty..");
				return 0;
			}	
			else // 파일 리스트가 비어 있지 않으면 DownSendMsg호출 -> 다운받을 파일 이름 입력하여 전송
			{
				for (i = 2; i < strlen(msg); i++)
					printf("%c", msg[i]);
				printf("\n");
				DownSendMsg(sock);
				return 0;
			}
		}

		//파일을 받은 경우
		if (msg[0] == 'f')
		{
			if (msg[2] == 'N') // 입력한 파일명에 해당하는 파일이 없을 시 종료
			{
				fileNameFlag = 1;
				printf("Not found file..\n");
				return 0;
			}
			else
			{
				// 기존에 있던 리스트 해제(기존 파일 초기화)
				Exit(head, tail);
				
				
				/*head = (NODE*)malloc(sizeof(NODE));
				tail = (NODE*)malloc(sizeof(NODE));*/
				head->data = NULL; head->up = NULL; head->down = NULL; head->pre = NULL;
				tail->data = NULL; tail->up = NULL; tail->down = NULL; tail->next = NULL;
				head->next = tail;	tail->pre = head;
				current = head;

				// 서버로 부터 전송 받은 파일(문자열)을 읽어 리스트에 연결
				for (i = 2; i < strlen(msg); i++)
				{	
					current = insert(head, tail, current, msg[i]);
					//printf("%c", msg[i]);
				}
				return 0;
			}
		}
		
	}
}
#pragma endregion 2012244030 김의섭