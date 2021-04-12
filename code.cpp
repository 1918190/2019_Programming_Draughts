#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// board information
#define BOARD_SIZE 8
#define EMPTY 0
#define MY_FLAG 2
#define MY_KING 4
#define ENEMY_FLAG 1
#define ENEMY_KING 3


// bool
typedef int BOOL;
#define TRUE 1
#define FALSE 0

#define MAX_STEP 15

#define START "START"
#define PLACE "PLACE"
#define TURN "TURN"
#define END "END"

int minMax(int alpha, int beta);

struct Command
{
	int x[MAX_STEP];
	int y[MAX_STEP];
	int numStep;
};

char board[BOARD_SIZE][BOARD_SIZE] = { 0 };
int myFlag;
int moveDir[4][2] = { {1, -1}, {1, 1}, {-1, -1}, {-1, 1} };
int jumpDir[4][2] = { {2, -2}, {2, 2}, {-2, -2}, {-2, 2} };
int numMyFlag;
int numEnemy = 12;        //���˵���
int numMy = 12;        //�ҵ���
int longestJump = 1;  //�����Ծ����
int findStep = 0;     //��������
int sameNum = 0;      //��ͬ�ĳ��Ӳ���
int copyStep = 0;     //longestJump�ĸ�����
int alpha = -10000000;
int beta = 10000000;
int pace = 0;          //����
size_t size = sizeof(struct Command);
char change[20][100] = { {0},{0} };                         //��������ʱ��һ�����ӱ������
struct Command oneJump[20][4] = { {0},{0}, 0 };             //һ���ӵ���Ծ��ѡ�����
struct Command oneMove[20][4] = { {0},{0}, 0 };             //һ���ӵ����߿�ѡ�����
struct Command temp[20][48] = { {0},{0}, 0 };               //����ò�����еĿɳ����
struct Command eatType[20][48] = { {0},{0}, 0 };                //��¼�Ե��ӵ�����
struct Command moveCmd = { .x = {0},.y = {0},.numStep = 2 };     //�����ƶ���Ϣ
struct Command jumpCmd = { .x = {0},.y = {0},.numStep = 0 };     //������Ծ������Ϣ
struct Command command =
{
	.x = {0},
	.y = {0},
	.numStep = 0
};

void debug(const char* str)
{
	printf("DEBUG %s\n", str);
	fflush(stdout);
}

void printBoard()//��ӡ����
{
	char visualBoard[BOARD_SIZE][BOARD_SIZE + 1] = { 0 };
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			switch (board[i][j])
			{
			case EMPTY:
				visualBoard[i][j] = '.';
				break;
			case ENEMY_FLAG:
				visualBoard[i][j] = 'O';
				break;
			case MY_FLAG:
				visualBoard[i][j] = 'X';
				break;
			case ENEMY_KING:
				visualBoard[i][j] = '@';
				break;
			case MY_KING:
				visualBoard[i][j] = '*';
				break;
			default:
				break;
			}
		}
		printf("%s\n", visualBoard[i]);
	}
}

BOOL isInBound(int x, int y)//�ж��Ƿ���������
{
	return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

BOOL myJudge(int x, int y, int pace)//�Ƿ����ҷ�����
{
	return (board[x][y] == MY_FLAG || board[x][y] == MY_KING)
		&& (findStep % 2 == pace);
}

BOOL enemyJudge(int x, int y, int pace)//�Ƿ��ǵз�����
{
	return (board[x][y] == ENEMY_FLAG || board[x][y] == ENEMY_KING)
		&& (findStep % 2 == pace);
}

BOOL eatMy(int midX, int midY)//�жϱ��Ե��ǲ����ҷ�����
{
	return (board[midX][midY] == MY_FLAG || board[midX][midY] == MY_KING)
		&& ((findStep % 2 == 0) || (findStep == 0));
}

BOOL eatEnemy(int midX, int midY)//�жϱ��Ե��ǲ��ǵз�����
{
	return (board[midX][midY] == ENEMY_FLAG || board[midX][midY] == ENEMY_KING)
		&& ((findStep % 2 == 1) || (findStep == 0));
}

void rotateCommand(struct Command* cmd)//��ת����
{
	if (myFlag == ENEMY_FLAG)
	{
		for (int i = 0; i < cmd->numStep; i++)
		{
			cmd->x[i] = BOARD_SIZE - 1 - cmd->x[i];
			cmd->y[i] = BOARD_SIZE - 1 - cmd->y[i];
		}
	}
}

int myOrEnemy()//�ж����
{
	if (findStep % 2 == 1)
		return numMy;
	else
		return numEnemy;
}

int tryToMove(int x, int y, int moveWay)//����ǰ��
{
	int newX, newY;
	int direction = 0;//��������ƶ��ķ���
	moveWay = 0;
	if (board[x][y] == ENEMY_KING || board[x][y] == MY_KING)
	{
		direction = 4;
	}
	if (board[x][y] == ENEMY_FLAG || board[x][y] == MY_FLAG)
	{
		direction = 2;
	}
	for (int i = 0; i < direction; i++)//������������
	{
		if ((board[x][y]) % 2 == 0)
		{
			newX = x + moveDir[i][0];
			newY = y + moveDir[i][1];
		}
		else
		{
			newX = x + moveDir[3 - i][0];
			newY = y + moveDir[3 - i][1];
		}
		if (isInBound(newX, newY) && board[newX][newY] == EMPTY)//������
		{
			moveCmd.x[0] = x;
			moveCmd.y[0] = y;
			moveCmd.x[1] = newX;
			moveCmd.y[1] = newY;
			memcpy(&oneMove[findStep - 1][moveWay], &moveCmd, size);
			moveWay++;
		}
	}
	if (moveWay > 0)
		return moveWay;
	else
		return -1; //������
}

void tryToJump(int x, int y, int currentStep) //���Գ���
{
	int newX, newY, midX, midY;
	char tmpFlag;
	jumpCmd.x[currentStep] = x;
	jumpCmd.y[currentStep] = y;
	jumpCmd.numStep++;
	for (int i = 0; i < 4; i++) //�ĸ������ж�
	{
		newX = x + jumpDir[i][0];
		newY = y + jumpDir[i][1];
		midX = (x + newX) / 2;
		midY = (y + newY) / 2;
		if (isInBound(newX, newY) && ((myJudge(midX, midY, 0)) 
			|| (enemyJudge(midX, midY, 1))) && (board[newX][newY] == EMPTY))//���ķ�����
		{
			board[newX][newY] = board[x][y];
			board[x][y] = EMPTY;
			tmpFlag = board[midX][midY];
			board[midX][midY] = EMPTY;
			tryToJump(newX, newY, currentStep + 1);
			board[x][y] = board[newX][newY];
			board[newX][newY] = EMPTY;
			board[midX][midY] = tmpFlag;
		}
	}
	if (jumpCmd.numStep > longestJump)
	{
		sameNum = 0;
		memcpy(&oneJump[findStep - 1][sameNum], &jumpCmd, size);
		longestJump = jumpCmd.numStep;
		copyStep = jumpCmd.numStep;
	}//�ҵ�����������滻��
	else if (jumpCmd.numStep == longestJump && jumpCmd.numStep != 1)
	{
		sameNum++;
		memcpy(&oneJump[findStep - 1][sameNum], &jumpCmd, size);
	}//��������ͬ��������ۼ�
	jumpCmd.numStep--;
}

void isChange(int step)//�ж��������������Ƿ����
{
	for (int i = 0; i < BOARD_SIZE; i++)//�Ƿ����
	{
		if (board[0][i] == ENEMY_FLAG)
		{
			board[0][i] = ENEMY_KING;
			if (findStep > 0)
			{
				change[findStep - 1][step] = 1;
			}
		}
		if (board[BOARD_SIZE - 1][i] == MY_FLAG)
		{
			board[BOARD_SIZE - 1][i] = MY_KING;
			if (findStep > 0)
			{
				change[findStep - 1][step] = 2;
			}
		}
	}
}

void place(struct Command cmd, int step)//�ƶ�
{
	int count = 0;      //������Ӹ���
	int midX, midY, curFlag;
	curFlag = board[cmd.x[0]][cmd.y[0]];
	for (int i = 0; i < cmd.numStep - 1; i++)
	{
		board[cmd.x[i]][cmd.y[i]] = EMPTY;
		board[cmd.x[i + 1]][cmd.y[i + 1]] = curFlag;
		if (abs(cmd.x[i] - cmd.x[i + 1]) == 2)//�г��ӵĹ���
		{
			midX = (cmd.x[i] + cmd.x[i + 1]) / 2;
			midY = (cmd.y[i] + cmd.y[i + 1]) / 2;
			if (eatMy(midX, midY))
			{
				numMy--;
			}
			else if (eatEnemy(midX, midY))
			{
				numEnemy--;
			}
			if (findStep != 0)//�ж��Ƿ����Լ��Ļغ�
			{
				eatType[findStep - 1][step].x[count] = board[midX][midY];
				count++;
			}
			board[midX][midY] = EMPTY;
		}
	}
	isChange(step);
}

/**
 * YOUR CODE BEGIN
 * ��Ĵ��뿪ʼ
 */

 /**
  * You can define your own struct and variable here
  * ����������ﶨ�����Լ��Ľṹ��ͱ���
  */

  /**
   * ������������ʼ�����AI
   */

void changeType(int step, struct Command cmd)//�������б��������ӻ�ԭ
{
	if (change[findStep - 1][step] == 1 && board[cmd.x[0]][cmd.y[0]] == ENEMY_KING)//��ԭ��������
	{
		board[cmd.x[0]][cmd.y[0]] = ENEMY_FLAG;
	}
	else if (change[findStep - 1][step] == 2 && board[cmd.x[0]][cmd.y[0]] == MY_KING)
	{
		board[cmd.x[0]][cmd.y[0]] = MY_FLAG;
	}
}

void unPlaceJump(struct Command cmd, int step)//��ԭ����
{
	int midX = 0, midY = 0;
	for (int i = cmd.numStep; i > 1; i--)
	{
		midX = (cmd.x[i - 2] + cmd.x[i - 1]) / 2;
		midY = (cmd.y[i - 2] + cmd.y[i - 1]) / 2;
		board[cmd.x[i - 2]][cmd.y[i - 2]] = board[cmd.x[i - 1]][cmd.y[i - 1]];
		board[cmd.x[i - 1]][cmd.y[i - 1]] = EMPTY;
		board[midX][midY] = eatType[findStep - 1][step].x[i - 2];
		if (board[midX][midY] == MY_FLAG || board[midX][midY] == MY_KING)
		{
			numMy++;
		}
		else if (board[midX][midY] == ENEMY_FLAG || board[midX][midY] == ENEMY_KING)
		{
			numEnemy++;
		}
	}
	changeType(step, cmd);
}

void unPlaceMove(struct Command cmd, int a1, int move)//��ԭ����
{
	board[cmd.x[0]][cmd.y[0]] = board[cmd.x[1]][cmd.y[1]];
	board[cmd.x[1]][cmd.y[1]] = EMPTY;
	changeType(4 * a1 + move + 50, cmd);
}



int pieceScore(int i, int j)
{
	int point = 0;
	if (board[i][j] == MY_FLAG)
	{
		point += (1080 + 12 * i + pace);
	}
	else if (board[i][j] == MY_KING)
	{
		point += (1704 + pace);
	}
	else if (board[i][j] == ENEMY_FLAG)
	{
		point -= (1164 - 12 * i + pace);
	}
	else if (board[i][j] == ENEMY_KING)
	{
		point -= (1704 + pace);
	}
	return point;
}

int estimate()//��������
{
	int point = 0;
	for (int i = 0; i < BOARD_SIZE; i++)
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			point += pieceScore(i, j);
			if (board[i][j] == MY_FLAG || board[i][j] == MY_KING)
			{
				if ((j == 0 || j == 7) && pace < 40)
				{
					point += 30;
				}
				else if (i >= 2 && i <= 6 && j >= 2 && j <= 5)
				{
					point += 24;
				}
			}
			else if (board[i][j] == ENEMY_FLAG || board[i][j] == ENEMY_KING)
			{
				if ((j == 0 || j == 7) && pace < 40)
				{
					point -= 30;
				}
				else if (i >= 1 && i <= 5 && j >= 2 && j <= 5)
				{
					point -= 24;
				}
			}
		}
	}
	return point;
}

int aiFind(int* num, int* numChecked, char* boardX, char* boardY)//��������
{
	int mark = 0;   //��¼��û�г��ӵĲ���
	int maxStep = 1;
	numMyFlag = myOrEnemy();
	for (int i = 0; i < BOARD_SIZE; i++)//��������
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			if (board[i][j] > 0 && ((myJudge(i, j, 1)) || (enemyJudge(i, j, 0))))//�ж��Ƿ�����
			{
				boardX[*numChecked] = i, boardY[*numChecked] = j;
				(*numChecked)++;
				tryToJump(i, j, 0);
				if (copyStep > maxStep)//�ҵ��Ĳ�����֮ǰ�Ļ�Ҫ�࣬���������
				{
					mark = 1;
					for (*num = 0; *num <= sameNum; (*num)++)
					{
						memcpy(&temp[findStep - 1][*num], &oneJump[findStep - 1][*num], size);
					}
					maxStep = copyStep;
				}
				else if (copyStep == maxStep && copyStep != 1)//��֮ǰһ�������뱸ѡ
				{
					for (;; (*num)++)
					{
						if (sameNum < 0)
						{
							break;
						}
						memcpy(&temp[findStep - 1][*num], &oneJump[findStep - 1][sameNum], size);
						sameNum--;	
					}
				}
				longestJump = 1;
				copyStep = 0;
			}
			if (*numChecked == numMyFlag)//�������
			{
				i = BOARD_SIZE;
				break;
			}
		}
	}
	return mark;
}

void canJump(int* bestMove, int* step, int* alpha, int* beta)//�������п��Գ��ӵĲ���
{
	int point;
	place(temp[findStep - 1][*step], *step);
	if (findStep == 10)      //�޸Ĳ���
	{
		point = -estimate();
	}
	else
		point = -minMax(-(*beta), -(*alpha));//������ֵ
	if (point > * bestMove)
	{
		*bestMove = point;
		if (findStep == 1)    //��������ѡ��
		{
			memcpy(&command, &temp[findStep - 1][*step], size);
		}
	}
	unPlaceJump(temp[findStep - 1][*step], *step);
}

int canMove(int* moveWay, int* bestMove, int* pieces, int* alpha, int* beta)//������ֻ���ƶ�
{
	int point;
	int over = 0;
	for (int move = 0; move <= *moveWay - 1; move++)
	{
		place(oneMove[findStep - 1][move], 4 * (*pieces) + move + 50);
		if (findStep == 10)//�޸Ĳ���
		{
			point = -estimate();
		}
		else
			point = -minMax(-(*beta), -(*alpha));//������ֵ
		if (point > * bestMove)
		{
			*bestMove = point;
			if (findStep == 1)
			{
				memcpy(&command, &oneMove[findStep - 1][move], size);  //��������ѡ��
			}
		}
		unPlaceMove(oneMove[findStep - 1][move], *pieces, move);
		if (*bestMove > * alpha)
		{
			*alpha = *bestMove;
		}
		if (*bestMove >= *beta)
		{
			over = 1;
			break;
		}
	}
	return over;
}
/**
 * �ֵ������ӡ�
 * ������0��ʾ�հף�1��ʾ���壬2��ʾ����
 * me��ʾ�������������(1��2)
 * ����Ҫ����һ���ṹ��Command������numStep����Ҫ�ƶ������Ӿ����ĸ�����������㡢�յ㣩��
 * x��y�ֱ��Ǹ��������ξ�����ÿ�����ӵĺᡢ������
 */

int minMax(int alpha, int beta)//����С
{
	findStep++;
	int bestMove = -10000000;//�򵥳�ʼ��
	int num = 0;
	int numChecked = 0;
	int step = 0;
	int moveWay = 0;
	int pieces = 0;
	int over = 0;
	char boardX[12] = { 0 }, boardY[12] = { 0 };
	if (aiFind(&num, &numChecked, boardX, boardY) == 1)
	{
		for (step = 0; step < num; step++)//�������г��Ӳ���
		{
			canJump(&bestMove, &step, &alpha, &beta);
			if (bestMove > alpha)
			{
				alpha = bestMove;
			}
			if (bestMove >= beta)
			{
				break;
			}
		}
	}
	else
	{
		if (numChecked == 0)
		{
			bestMove = -10000000;
		}
		else
		{
			for (pieces = 0; pieces < numChecked; pieces++)//���ӿ�ʼ����
			{
				if ((moveWay = tryToMove(boardX[pieces], boardY[pieces], 0)) > 0)
				{
					over = canMove(&moveWay, &bestMove, &pieces, &alpha, &beta);
					if (over == 1)
						break;
				}
			}
		}
	}
	memset(change[findStep - 1], 0, 100);
	findStep--;
	return bestMove;
}

/**
 * ��Ĵ������
 */

 //.X.X.X.X
 //X.X.X.X.
 //.X.X.X.X
 //........
 //........
 //O.O.O.O.
 //.O.O.O.O
 //O.O.O.O.
void start(int flag) //��ʼ
{
	memset(board, 0, sizeof(board));
	for (int i = 0; i < 3; i++)
	{
		for (int j = 0; j < 8; j += 2)
		{
			board[i][j + (i + 1) % 2] = MY_FLAG;
		}
	}
	for (int i = 5; i < 8; i++)
	{
		for (int j = 0; j < 8; j += 2)
		{
			board[i][j + (i + 1) % 2] = ENEMY_FLAG;
		}
	}
}

void turn() //�Լ�����
{
	// AI
	minMax(alpha, beta);
	place(command, 0);
	rotateCommand(&command);
	printf("%d", command.numStep);
	for (int i = 0; i < command.numStep; i++)
	{
		printf(" %d,%d", command.x[i], command.y[i]);
	}
	printf("\n");
	fflush(stdout);
	pace++;
}

void end(int x)  //����
{
	exit(0);
}

void loop() //����
{
	char tag[10] = { 0 };
	struct Command command =
	{
		.x = {0},
		.y = {0},
		.numStep = 0
	};
	int status;
	while (TRUE)
	{
		for (int i = 0; i < 6; i++)
		{
			memset(change[i], 0, 100 * sizeof(char));
		}
		memset(tag, 0, sizeof(tag));
		scanf("%s", tag);
		if (strcmp(tag, START) == 0)
		{
			scanf("%d", &myFlag);
			start(myFlag);
			printf("OK\n");
			fflush(stdout);
		}
		else if (strcmp(tag, PLACE) == 0)
		{
			scanf("%d", &command.numStep);
			for (int i = 0; i < command.numStep; i++)
			{
				scanf("%d,%d", &command.x[i], &command.y[i]);
			}
			rotateCommand(&command);
			place(command, 0);
		}
		else if (strcmp(tag, TURN) == 0)
		{
			turn();
		}
		else if (strcmp(tag, END) == 0)
		{
			scanf("%d", &status);
			end(status);
		}
		//printBoard();
	}
}

int main(int argc, char* argv[])
{
	loop();
	return 0;
}