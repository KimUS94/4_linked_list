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

NODE* insert(NODE *head, NODE *tail, NODE *current, int data);  // ��� ���� �Լ�       _ ���Ǽ�
NODE* Delete(NODE * head, NODE *tail, NODE *current, int data); // ��� ���� �Լ�       _ ���Ǽ�
void Exit(NODE *head, NODE *tail);                              // �޸� ���� �Լ�     _ ���Ǽ�

NODE* Move(NODE *head, NODE *tail, NODE *current, int data);   // Ŀ�� �̵� �Լ�        _ ��溸
NODE* FileIO(NODE* head, NODE* tail, int data);				   // ���� ����� �Լ�      _ ��溸
void FileRenewal(char *fname);                                 // ���� ���ġ �Լ�      _ ��溸
void setCusor(NODE *head, NODE* current);                      // Ŀ�� ��� �Լ�        _ ��溸
void ViewAll(NODE *head, NODE *tail);                          // ��� ����Ʈ ��� �Լ� _ ��溸


NODE* FileUpload(NODE *head, NODE* tail);						// ���ε� ��û �Լ�
unsigned UpSendMsg(SOCKET sock);								// ���ε� ��û �޼��� �����Լ�

NODE* FileDownload(NODE *head, NODE* tail);						// �ٿ�ε� ��û �Լ�
unsigned DownSendMsg(SOCKET sock);								// �ٿ�ε� ��û �޼��� �����Լ�
unsigned WINAPI DownRecvMsg(void* arg);							// �ٿ�ε� ��û �޼��� �����Լ�


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
			[��]	224 72
			[��] 224 75      [��]	224 80		224 7711 [��]
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
	// �ݺ������� �Լ��� ȣ���ϸ� head ~ tail ���� ���
}

void setCusor(NODE *head, NODE* current)
{
	int x = 0, y = 2;
	NODE *temp = head;
	while (temp != current)
	{
		if (temp->data == 10) // 10 = enter Ű�� �� enter�Է� �� Ŀ�� y������ ����(�Ʒ��� ������)
		{
			x = 0;
			y++;
		}
		else x++;
		temp = temp->next;
	}
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos); // �Լ� ��� http://kkikkodev.tistory.com/26 ����
																	// while�� ������ ����Ʈ�� ����� ���� ���� x, y �·Ḧ ���� ��Ű�� �ش��ϴ� x,y ��ǥ�� Ŀ���� �̵� �����ִ� �Լ�
}

NODE* FileIO(NODE* head, NODE* tail, int data)
{
	char fname[50], ch;
	FILE *fp_in, *fp_out;
	NODE* current = head;

	// F1 �Է� ��
	if (data == 59)
	{
		printf("Plese enter the File name_");
		scanf("%s", fname);
		system("cls");
		FileRenewal(fname); // ������ ���ġ ��. ex) �о���� ������ �� �ٸ��� ���� ���� üũ �Ͽ� 80���ڰ� �Ѿ� ���� ���๮�ڸ� ���� ������ �� ����
		fp_in = fopen(fname, "r");
		if (fp_in == NULL) { // ����ó��
			printf("File Error\n");
			return head;
		}
		while (!feof(fp_in)) // ������ �ѱ��� �� �о� ����Ʈ�� �߰�
		{
			fscanf(fp_in, "%c", &ch);
			current = insert(head, tail, current, ch);
		}
		fclose(fp_in);
		return head;
	}
	//F2 �Է� ��
	if (data == 60)
	{
		printf("Plese enter the SaveFile name_");
		scanf("%s", fname);
		system("cls");
		fp_out = fopen(fname, "w");
		if (fp_out == NULL) { //����ó��
			printf("File Error\n");
			return head;
		}
		while (current != tail) //current(head) ~ tail���� ����Ʈ�� �޸� ��带 ���Ͽ� ���
		{
			if (current->data == NULL);
			else fprintf(fp_out, "%c", current->data);
			current = current->next;
		}
		fclose(fp_out);
		return tail->pre;
	}
	//F3 �Է� ��
	if (data == 61)
		FileUpload(head, tail);
	//F4 �Է� ��
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

	if (fp_in == NULL) { // ����ó��
		printf("File Error\n");
		return;
	}
	while ((ch = fgetc(fp_in)) != EOF) // ������ ������ ������ �ѱ��� �� �о� ����
	{
		if (ch != '\n')	count++; // ���๮�ڸ� ������ ������ ī���� ����
		else count = 0; // ���๮�ڸ� ������ ī���� �ʱ�ȭ

		if (i == 0) str[i] = ch;
		else if (count == 80)
		{
			str[i] = '\n'; // count == 80 �� ���� ���� ���ڰ� 80�� �̻��̶�� ���๮�ڸ� ����
			str[i + 1] = ch; // �̹��� �о� ���� ���ڴ� ������ �������� �߰�
			count = 0;
			i++;
		}
		else str[i] = ch;
		i++;
	}
	str[i] = '\0';
	fp_out = fopen(fname, "w");
	fprintf(fp_out, "%s", str); // �籸���� ���ڿ�(str)�� �������Ͽ� �ٽ� ����

	fclose(fp_in);
	fclose(fp_out);
}

NODE* Move(NODE *head, NODE *tail, NODE *current, int data)
{
	/* Move �Լ��� �����Ǿ� �ִ� ��� Ű���� �Է½� ASCII �ڵ尡 2byte�� �Է� �Ǳ� ������ getch()�� �Է� ���� ���
	�׻� ù��° ����Ʈ�� 224�� ������ ������ 224�� �ԷµǸ� Move�Լ��� �̵� �Ǹ� ���� �ι��� ����Ʈ(���ۿ� �����ִ�) ���� ���� �ش� ��ƾ ���� */

	// �� �Է� �� current->up ����
	if (data == 72)
	{
		if (current->up == NULL);
		else current = current->up;
	}
	// �� �Է� �� current->pre ����
	if (data == 75)
	{
		if (current->pre == head || current->pre == NULL)
			return head;
		else current = current->pre;
	}
	// �� �Է� �� current->next ����
	if (data == 77)
	{
		if (current->next == tail || current->next == NULL);
		else current = current->next;
	}
	// �� �Է� �� current->down ����
	if (data == 80)
	{
		if (current->down == NULL);
		else current = current->down;
	}

	//Home �Է� �� �� ���� ù��° ��� ����,  2012244030 ���Ǽ�(����)
	if (data == 71) // Home
	{
		while (current != head && current->pre->data != 10)
			current = current->pre;
	}
	//End �Է� �� �� ���� ������ ��� ����(���� ������ ��尡 tail �̶�� tail->pre ����),  2012244030 ���Ǽ�(����)
	if (data == 79) // End
	{
		while (current->next != tail && current->next->data != 10)
			current = current->next;
		if (current == tail) return current->pre;
	}

	return current;

}
#pragma endregion 2012244048 ��溸

#pragma region
void Exit(NODE *head, NODE *tail)
{
	// head ~ tail ���� ��������� ȣ���ϸ� ��忡 �Ҵ�Ǿ� �ִ� �޸� ���� ����
	// �Ҵ籭 ��� ������ ���� �ϸ� ���α׷� ����
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
			//������ ���(follow)�� pre�� next�� ����
			follow->pre->next = current;
			current->pre = follow->pre;

			//follow(������ ����)�� ���๮���� ���
			if (follow->data == 10)
			{
				if (current->down != NULL)  // follow->data �� ���� ���� �̸鼭 current(follow->next)�� ����Ű�� down�� �ִ� ���
				{

					temp = follow;
					while (temp->pre != head && temp->pre->data != 10)  // follow�� �ٿ� ù��° ���� temp�� �̵� ��Ŵ
						temp = temp->pre;

					temp2 = current->down;
					while (temp->data != 10 && temp != NULL) //temp�� �� ���� ������ �̵� ��Ű�� 
					{
						if (temp2 != tail && temp2->data != 10) //temp2�� �� ���� ���� �ƴ϶��
						{
							temp->down = temp2;                 //temp�� temp2 up, down ����
							temp->down->up = temp;
						}
						else temp->down = temp->pre->down;  // temp2�� �� ���� ���̶�� temp�� down�� temp->pre�� down����

						if (temp2 != tail && temp2->data != 10) temp2 = temp2->next; //temp2�� �� ���� ���� �ƴ϶�� temp2 ����
						temp = temp->next;
					}
				}
				else // follow->data �� ���� ���� �̸鼭 current(follow->next)�� ����Ű�� down�� ���� ���
				{
					if (follow->up != NULL) temp = follow->up;
					else temp = follow; //temp->up�� null�� �ƴϸ� temp�� follow�� ����Ŵ

					while (temp->pre != head && temp->pre->data != 10) // temp�� �� ���� ù��° ���� �̵� ��Ŵ
						temp = temp->pre;

					temp2 = current;
					while (temp2->pre != head && temp2->pre->data != 10) //temp2(current)�� �� ���� ù��°�� �̵� ��Ŵ
						temp2 = temp2->pre;


					while (temp2->data != 10 && temp2 != NULL && temp2 != tail) //temp2(current) �� �� ���� ���� �ƴ� ���� �ݺ�
					{
						if (temp->data != 10 && temp != NULL) //temp�� �� ���� ���� �ƴϸ� temp�� temp2 ����
						{
							temp->down = temp2;
							temp2->up = temp;
						}
						else temp2->up = temp2->pre->up; // temp�� �� ���� ���̸� temp2->up�� temp2 ������ ��
						if (temp != NULL && temp->data != 10) temp = temp->next;
						temp2 = temp2->next;
					}
				}
				free(follow);
			}

			//follow(������ ����)�� ���๮�ڰ� �ƴ� ���
			else
			{

				if (follow->down != NULL)
				{
					// �� ��� follow�� �ִ� ���� �� ù��° �� �̱� ������ follow->up(temp->up)�� ����� �ʿ� ����
					if (follow->up == NULL)
					{
						//temp, temp2 ���
						temp = follow;
						while (temp->pre != head && temp->pre->data != 10) // temp(follow)�� ������ ù�� °�� �̵� ��Ŵ
							temp = temp->pre;

						temp2 = follow->down;
						while (temp2->pre != head && temp2->pre->data != 10) //temp2(follow->down)�� �� ���� ù��° ���� �̵� ��Ŵ
							temp2 = temp2->pre;

						while ((temp->data != 10 && temp != NULL) || (temp2->data != 10 && temp2 != tail && temp2 != NULL)) // temp, temp2�� ������ ���� �ƴ� ����
						{
							//temp�� temp2�� �� ���� ���� �ƴ϶�� ������ up, down ���� ��Ŵ(temp�� temp2�� �� ���� ù���� ��� ���� ���� �ӵ��� ����)
							if ((temp->data != 10 && temp != NULL) && (temp2->data != 10 && temp2 != tail && temp2 != NULL))
							{
								temp2->up = temp;
								temp->down = temp2;
							}
							// temp�� temp2�� �ϳ��ε� �� ���� ���� ��� �׿� �´� ó���� ��
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

					// follow�� up �� down�� ��� null�� �ƴ� ���� 3���� ���� ���踦 ��� �ؾ���
					else
					{
						//temp3, temp, temp2 ���
						temp = follow;
						temp2 = follow->down;
						temp3 = follow->up;
						while (temp->pre != head && temp->pre->data != 10)
							temp = temp->pre;
						while (temp2->pre != head && temp2->pre->data != 10)
							temp2 = temp2->pre;
						while (temp3->pre != head && temp3->pre->data != 10)
							temp3 = temp3->pre;
						// �� ���� ù ���� temp3(����), temp, temp2(�Ʒ���) �̵���Ŵ

						while ((temp->data != 10 && temp != NULL) || (temp2->data != 10 && temp2 != NULL && temp2 != tail) ||
							(temp3->data != 10 && temp3 != NULL)) //temp, temp2, temp3�� ��� �� ���� ���� �����ϸ� �ݺ��� ����
						{
							// 000(temp, temp2, tmep3 ��� ������ ���� �ƴ� ���)
							if ((temp->data != 10 && temp != NULL) && (temp2->data != 10 && temp2 != tail && temp2 != NULL)
								&& (temp3->data != 10 && temp3 != NULL))
							{
								temp2->up = temp;
								temp->up = temp3;
								temp->down = temp2;
								temp3->down = temp;
							}

							// temp, temp2, tmep3 �� �ϳ��� �� ���� ���� ���� �� ���
							// temp, temp2, temp3 �� �� ���� ���� ���� �ϰ� ���ϰ� 0,1�� ġȯ �ϸ� 8���� ����� ���� ���� 
							// ���⼭ 000(��� ������ ���� �ƴ� ���)�� ������ ó�� �Ͽ����Ƿ� �ؿ��� ������ 6���� ó��
							// (111�� ��� �ݺ����� ����ǹǷ� ó�� ���� ����)
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
					//follow�� up�� down�� ��� null �� ��� ��, ���ڿ��� ������ ���
					if (follow->up == NULL)
					{
						follow->pre->next = current;
						current->pre = follow->pre;
						free(follow);
					}

					// �� ��� follow�� �� �Ʒ� �� �̱� ������ follow->down�� ����� �ʿ䰡 ����
					else //follow->up != �϶�
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
	NODE *follow = current;  // ��� ���ڴ� follow �ڿ� current �տ� ���Ե�
	current = current->next;
	NODE *Up_temp = NULL, *temp2 = NULL, *temp = NULL, *empty = NULL;
	NODE *newnode = (NODE*)malloc(sizeof(NODE));

	if (data == 13 || data == 10) data = 10; // Ű����� �Է��ϴ� enter�� ASCII�� 13 '\n'������ ASCII�� 10 ���� ��� ���� ���ڸ� 10���� ����
	newnode->data = data;
	newnode->pre = NULL, newnode->down = NULL, newnode->up = NULL, newnode->next = NULL;

#pragma region ���๮��(enter//13//10) ���� ��
	if (data == 13 || data == 10)
	{
		/*  ���๮�� ���� �� ���๮�� �ڿ� tail�� �´ٸ� ���๮�� �ڿ� �ƹ��͵� ���� ���� �ʾ�
		��Ŀ�� �Լ� ȣ��� Ŀ���� �� ������ ����Ű�� ���ϱ� ������ empty��带 �Ҵ��Ͽ� �� ���� ǥ�� */
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

			while ((temp->data != 10) && (temp != head))  //�� ���� �������� temp�� �̵� ��Ŵ
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

		// ���๮��(���Ե� ����) ��(current)�� tail�� ���� ���� ��
		else
		{
			follow->next = newnode;
			newnode->pre = follow;
			newnode->next = current;
			current->pre = newnode;
			newnode->up = current->up;
			newnode->down = current;

			temp = current;
			while (temp->data != data)  //�� ���� �������� temp�� �̵� ��Ŵ
			{
				temp = temp->pre;
			}

			if (current->next == NULL || current->next == tail) // current�� �� ���� �� �� ���
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

#pragma region ���� ���� ��		
	else
	{
		// ���� �Է� �� ������ �Է� ��ġ�� ù��° ��� �� ���(head->next �� ���)
		if (follow == head)
		{
			follow->next = newnode;
			newnode->pre = follow;
			newnode->next = current;
			current->pre = newnode;

			// newnode�� �ڿ�(current) ������ �Է��� ��尡 �ִٸ�
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
			/* ���� �Ϸ��� ��ġ�� ��忡 empt��� ���� 1 �� ���(�� ���๮�� �Է½� ���� �� empyt����� ���)
			newnode�� ���� �߰� �� �ʿ� ���� data�� empty��忡 �����ϰ� empt����� 1�� �ƴ� ���� �ٲ��� */
			if (follow->empt == 1)
			{
				follow->empt = 0;
				follow->data = data;
				return follow;
				free(newnode);
			}

			// �������� ���(empty���)�� �ƴ� ���
			else
			{
				follow->next = newnode;
				newnode->pre = follow;

				// �׻� down�� ���� ������ ������ص���
				if (current == tail)
				{
					tail->pre = newnode;
					newnode->next = tail;
					newnode->down = NULL;
					//follow->up == NULL �̸� current==tail �̶� �Ҹ� ��, ���ڿ��� ���� �� ���
					if (follow->up == NULL)	newnode->up = NULL;

					/* current�� tail������ follow�� up�� ���� �ϱ� ������ down�� ����� �ʿ䰡 ������ up�� ����ؾ���
					�� follow�ٰ� follow->up �� ���� ���Ḹ ��� */
					else
					{
						// temp(����)�� ���� ���� �ƴ϶��
						if ((follow->up->next->data != 10) && (follow->up->next != NULL))
						{
							newnode->up = follow->up->next;
							newnode->up->down = newnode;
							temp = newnode->up;
							while ((temp->data != 10)) // ������ ���� �ƴ� ���� �ݺ��ϸ� ���ٰ� �Ʒ��� ����
							{
								temp->down = newnode;
								temp = temp->next;
								if (temp->data == 10) temp->down = newnode;
							}
						}

						// temp(����)�� ���� ���� ���
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

				// ��Ȳ�� ���� 2�� Ȥ�� 3���� ��� �ؾ���
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

						// temp(current)�� �� ���� ���� �ƴ� ���� �ݺ�
						while ((temp->data != 10) && (temp != tail))
						{
							if (newnode->up->next->data == 10);
							else temp->up = temp->next->up;
							temp = temp->next;
						}
					}
#pragma endregion newnode->up ó��
#pragma region 
					if (current->down == NULL) newnode->down = NULL;
					else //if (follow->down != NULL)
					{
						// �׸��� �׸���... �ǽ��� �帧��� ¥�� �۷� �����ϱ� ��ƽ��ϴ�...
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
#pragma endregion newnode->down ó��
				}
			}
		}
		return newnode;
	}
#pragma endregion
}
#pragma endregion 2012244030 ���Ǽ�

#pragma region
//���� ���ε� �Լ�
NODE* FileUpload(NODE *head, NODE* tail)
{
	WSADATA wsaData;// WSADATA struct�� window sockets �ʱ�ȭ ������ �����ϴµ� ����
	SOCKET sock;
	SOCKADDR_IN serverAddr;
	/*IPv4 �ּ�ü�迡�� ����ϴ� ����ü��. ���� ���α׷��� ���� �ּ� ����ü�� sockaddr�� ���������, 
	  �ּ�ü���� ������ ���� ������ ���� ����ü�� ����� ����ϴ°� �ƹ����� ���� ���̴�. 
	  �����, ��Ÿ �ٸ� �ּ�ü�� �� Local Unix �ּ� ü��� sockaddr_un ����ü�� ����Ѵ�.
	  [��ó] sockaddr, sockaddr_in ����ü|�ۼ��� ������ */ 
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

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// ������ ������ ����Ѵٰ� �ü���� �˸�
	{
		fputs("Connect() error.... \n", stderr);
		printf("Please press any key to continue....");
		getch();
		system("cls");
		return 0;
	}
	sprintf(name, "[%s]", inputName);
	sprintf(file, "<%s>", fileName);
	sock = socket(PF_INET, SOCK_STREAM, 0);//������ �ϳ� �����Ѵ�.

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(myIp);
	serverAddr.sin_port = htons(atoi(port));

	if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)// ������ �����Ѵ�.
	{
		fputs("Connect() error.... \n", stderr);
		printf("Please press any key to continue....");
		getch();
		system("cls");
		return 0;
	}
	UpSendMsg(sock); // ���ε� ��û �޼��� ���� �Լ� ȣ��
	
	printf("File uploading complete!\n");

	printf("Please press any key to continue....");
	getch();
	system("cls");
	closesocket(sock);//������ �����Ѵ�.
	WSACleanup();//������ ���� ��������� �ü���� �˸���.
	return 0;
}
//���ε� ��û �޼��� ���� �Լ� (�۾���, ����, ������ ���� ������ ����)
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
	sprintf(nameMsg, "u%s%s%s", name, file, msg); // ���ε��� ������ �̸��� �ۼ���, ������ �ϳ��� ���ڿ��� ��ģ��.
	send(sock, nameMsg, strlen(nameMsg), 0);//nameMsg�� �������� �����Ѵ�.

	return 0;
}


// �ٿ�ε� ��û �Լ�
NODE* FileDownload(NODE *head, NODE* tail)
{
	WSADATA wsaData;
	SOCKET sock;
	SOCKADDR_IN serverAddr;
	HANDLE recvThread; // Ŀ�ο�����Ʈ�� �ǵ帮�� ���� ����ü
	char *down = "down";
	char myIp[100];
	char port[100];
	char inputName[100];
	char fileName[100];

	printf("Input server IP : ");
	gets(myIp);

	printf("Input server port : ");
	gets(port);

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)// ������ ������ ����Ѵٰ� �ü���� �˸�
	{
		fputs("Connect() error.... \n", stderr);
		printf("Please press any key to continue....");
		getch();
		system("cls");
		return 0;
	}

	sock = socket(PF_INET, SOCK_STREAM, 0);//������ �ϳ� �����Ѵ�.

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = inet_addr(myIp);
	serverAddr.sin_port = htons(atoi(port));

	if (connect(sock, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)//������ �����Ѵ�.
	{
		fputs("Connect() error.... \n", stderr);
		printf("Please press any key to continue....");
		getch();
		system("cls");
		return 0;
	}

	send(sock, down, strlen(down), 0);
	recvThread = (HANDLE)_beginthreadex(NULL, 0, DownRecvMsg, (void*)&sock, 0, NULL);//�޽��� ���ſ� �����尡 ����ȴ�.
	WaitForSingleObject(recvThread, INFINITE);//���ſ� �����尡 �����ɶ����� ��ٸ���.

	//���� ����Ʈ�� empty ���� ���ϵǸ� �Ʒ����� ����ȵǰ� �ؾ���
	if (listFlag != 1)
	{
		recvThread = (HANDLE)_beginthreadex(NULL, 0, DownRecvMsg, (void*)&sock, 0, NULL);//�޽��� ���ſ� �����尡 ����ȴ�.
		WaitForSingleObject(recvThread, INFINITE);//���ſ� �����尡 �����ɶ����� ��ٸ���.
	}

	//Ŭ���̾�Ʈ�� ���Ḧ �õ��Ѵٸ� ���� �Ʒ��� ����ȴ�.
	if (fileNameFlag == 1)
		printf("File download error!\n");
	else
		printf("File download complete!\n");

	printf("Please press any key to continue....");
	getch();
	system("cls");
	closesocket(sock);//������ �����Ѵ�.
	WSACleanup();//������ ���� ��������� �ü���� �˸���.
	listFlag = 0;
	fileNameFlag = 0;
	return 0;
}
#pragma endregion ���Ǽ� ��溸

#pragma region
// �ٿ�ε� ��û�� ������ ���� �ϴ� �Լ�
unsigned DownSendMsg(SOCKET sock)
{
	char fileName[20];
	char fmsg[21];
	if (msg[0] == 't')
	{
		printf("Input you want file name : ");
		gets(fileName);
		sprintf(fmsg, "r%s", fileName);
		send(sock, fmsg, strlen(fmsg), 0);//nameMsg�� �������� �����Ѵ�.
	}
	return 0;
}
#pragma endregion 2012244048 ��溸

#pragma region
// �ٿ�ε� ��û �� ������ ������ �ִ� �����͸� ó���ϴ� �Լ� 
unsigned WINAPI DownRecvMsg(void* arg)
{
	HANDLE sendThread;
	SOCKET sock = *((SOCKET*)arg);//������ ������ �����Ѵ�.
	NODE* temp;
	char autor_title[BUF_SIZE];
	int strLen, i;
	while (1)
	{
		strLen = recv(sock, msg, BUF_SIZE - 1, 0);// �����κ��� �޽����� �����Ѵ�.
		if (strLen == -1)	return -1;            // ����ó��
		msg[strLen] = '\0';

		//���� ����Ʈ�� ���� ���
		if (msg[0] == 't')
		{
			printf("=================================\n");
			printf("[File list] | [autor] <fileName> \n");
			printf("=================================");
			if (msg[3] == 'E') // ������ ���� ����Ʈ�� ��� �ִٸ�
			{
				listFlag = 1;
				printf("\nThe list of files is empty..");
				return 0;
			}	
			else // ���� ����Ʈ�� ��� ���� ������ DownSendMsgȣ�� -> �ٿ���� ���� �̸� �Է��Ͽ� ����
			{
				for (i = 2; i < strlen(msg); i++)
					printf("%c", msg[i]);
				printf("\n");
				DownSendMsg(sock);
				return 0;
			}
		}

		//������ ���� ���
		if (msg[0] == 'f')
		{
			if (msg[2] == 'N') // �Է��� ���ϸ� �ش��ϴ� ������ ���� �� ����
			{
				fileNameFlag = 1;
				printf("Not found file..\n");
				return 0;
			}
			else
			{
				// ������ �ִ� ����Ʈ ����(���� ���� �ʱ�ȭ)
				Exit(head, tail);
				
				
				/*head = (NODE*)malloc(sizeof(NODE));
				tail = (NODE*)malloc(sizeof(NODE));*/
				head->data = NULL; head->up = NULL; head->down = NULL; head->pre = NULL;
				tail->data = NULL; tail->up = NULL; tail->down = NULL; tail->next = NULL;
				head->next = tail;	tail->pre = head;
				current = head;

				// ������ ���� ���� ���� ����(���ڿ�)�� �о� ����Ʈ�� ����
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
#pragma endregion 2012244030 ���Ǽ�