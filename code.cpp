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
int numEnemy = 12;        //敌人的子
int numMy = 12;        //我的子
int longestJump = 1;  //最大跳跃步数
int findStep = 0;     //搜索层数
int sameNum = 0;      //相同的吃子步数
int copyStep = 0;     //longestJump的复制体
int alpha = -10000000;
int beta = 10000000;
int pace = 0;          //步数
size_t size = sizeof(struct Command);
char change[20][100] = { {0},{0} };                         //储存搜索时哪一步棋子变成了王
struct Command oneJump[20][4] = { {0},{0}, 0 };             //一个子的跳跃可选择情况
struct Command oneMove[20][4] = { {0},{0}, 0 };             //一个子的行走可选择情况
struct Command temp[20][48] = { {0},{0}, 0 };               //保存该层的所有的可吃情况
struct Command eatType[20][48] = { {0},{0}, 0 };                //记录吃的子的种类
struct Command moveCmd = { .x = {0},.y = {0},.numStep = 2 };     //储存移动信息
struct Command jumpCmd = { .x = {0},.y = {0},.numStep = 0 };     //储存跳跃吃子信息
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

void printBoard()//打印棋盘
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

BOOL isInBound(int x, int y)//判断是否在棋盘内
{
	return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}

BOOL myJudge(int x, int y, int pace)//是否是我方行棋
{
	return (board[x][y] == MY_FLAG || board[x][y] == MY_KING)
		&& (findStep % 2 == pace);
}

BOOL enemyJudge(int x, int y, int pace)//是否是敌方行棋
{
	return (board[x][y] == ENEMY_FLAG || board[x][y] == ENEMY_KING)
		&& (findStep % 2 == pace);
}

BOOL eatMy(int midX, int midY)//判断被吃的是不是我方棋子
{
	return (board[midX][midY] == MY_FLAG || board[midX][midY] == MY_KING)
		&& ((findStep % 2 == 0) || (findStep == 0));
}

BOOL eatEnemy(int midX, int midY)//判断被吃的是不是敌方棋子
{
	return (board[midX][midY] == ENEMY_FLAG || board[midX][midY] == ENEMY_KING)
		&& ((findStep % 2 == 1) || (findStep == 0));
}

void rotateCommand(struct Command* cmd)//反转棋盘
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

int myOrEnemy()//判断身份
{
	if (findStep % 2 == 1)
		return numMy;
	else
		return numEnemy;
}

int tryToMove(int x, int y, int moveWay)//尝试前进
{
	int newX, newY;
	int direction = 0;//储存可以移动的方向
	moveWay = 0;
	if (board[x][y] == ENEMY_KING || board[x][y] == MY_KING)
	{
		direction = 4;
	}
	if (board[x][y] == ENEMY_FLAG || board[x][y] == MY_FLAG)
	{
		direction = 2;
	}
	for (int i = 0; i < direction; i++)//遍历所有棋子
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
		if (isInBound(newX, newY) && board[newX][newY] == EMPTY)//可以走
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
		return -1; //不能走
}

void tryToJump(int x, int y, int currentStep) //尝试吃子
{
	int newX, newY, midX, midY;
	char tmpFlag;
	jumpCmd.x[currentStep] = x;
	jumpCmd.y[currentStep] = y;
	jumpCmd.numStep++;
	for (int i = 0; i < 4; i++) //四个方向判断
	{
		newX = x + jumpDir[i][0];
		newY = y + jumpDir[i][1];
		midX = (x + newX) / 2;
		midY = (y + newY) / 2;
		if (isInBound(newX, newY) && ((myJudge(midX, midY, 0)) 
			|| (enemyJudge(midX, midY, 1))) && (board[newX][newY] == EMPTY))//是哪方行棋
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
	}//找到步数更深的替换掉
	else if (jumpCmd.numStep == longestJump && jumpCmd.numStep != 1)
	{
		sameNum++;
		memcpy(&oneJump[findStep - 1][sameNum], &jumpCmd, size);
	}//若步数相同，则向后累加
	jumpCmd.numStep--;
}

void isChange(int step)//判断在搜索过程中是否变王
{
	for (int i = 0; i < BOARD_SIZE; i++)//是否变王
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

void place(struct Command cmd, int step)//移动
{
	int count = 0;      //计算吃子个数
	int midX, midY, curFlag;
	curFlag = board[cmd.x[0]][cmd.y[0]];
	for (int i = 0; i < cmd.numStep - 1; i++)
	{
		board[cmd.x[i]][cmd.y[i]] = EMPTY;
		board[cmd.x[i + 1]][cmd.y[i + 1]] = curFlag;
		if (abs(cmd.x[i] - cmd.x[i + 1]) == 2)//有吃子的过程
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
			if (findStep != 0)//判断是否是自己的回合
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
 * 你的代码开始
 */

 /**
  * You can define your own struct and variable here
  * 你可以在这里定义你自己的结构体和变量
  */

  /**
   * 你可以在这里初始化你的AI
   */

void changeType(int step, struct Command cmd)//将搜索中变王的棋子还原
{
	if (change[findStep - 1][step] == 1 && board[cmd.x[0]][cmd.y[0]] == ENEMY_KING)//还原棋子类型
	{
		board[cmd.x[0]][cmd.y[0]] = ENEMY_FLAG;
	}
	else if (change[findStep - 1][step] == 2 && board[cmd.x[0]][cmd.y[0]] == MY_KING)
	{
		board[cmd.x[0]][cmd.y[0]] = MY_FLAG;
	}
}

void unPlaceJump(struct Command cmd, int step)//还原棋盘
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

void unPlaceMove(struct Command cmd, int a1, int move)//还原棋盘
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

int estimate()//评估函数
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

int aiFind(int* num, int* numChecked, char* boardX, char* boardY)//搜索棋盘
{
	int mark = 0;   //记录有没有吃子的步数
	int maxStep = 1;
	numMyFlag = myOrEnemy();
	for (int i = 0; i < BOARD_SIZE; i++)//遍历棋盘
	{
		for (int j = 0; j < BOARD_SIZE; j++)
		{
			if (board[i][j] > 0 && ((myJudge(i, j, 1)) || (enemyJudge(i, j, 0))))//判断那方在下
			{
				boardX[*numChecked] = i, boardY[*numChecked] = j;
				(*numChecked)++;
				tryToJump(i, j, 0);
				if (copyStep > maxStep)//找到的步数比之前的还要多，更新最大步数
				{
					mark = 1;
					for (*num = 0; *num <= sameNum; (*num)++)
					{
						memcpy(&temp[findStep - 1][*num], &oneJump[findStep - 1][*num], size);
					}
					maxStep = copyStep;
				}
				else if (copyStep == maxStep && copyStep != 1)//和之前一样，加入备选
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
			if (*numChecked == numMyFlag)//搜索完毕
			{
				i = BOARD_SIZE;
				break;
			}
		}
	}
	return mark;
}

void canJump(int* bestMove, int* step, int* alpha, int* beta)//搜索中有可以吃子的步数
{
	int point;
	place(temp[findStep - 1][*step], *step);
	if (findStep == 10)      //修改层数
	{
		point = -estimate();
	}
	else
		point = -minMax(-(*beta), -(*alpha));//负极大值
	if (point > * bestMove)
	{
		*bestMove = point;
		if (findStep == 1)    //储存最优选择
		{
			memcpy(&command, &temp[findStep - 1][*step], size);
		}
	}
	unPlaceJump(temp[findStep - 1][*step], *step);
}

int canMove(int* moveWay, int* bestMove, int* pieces, int* alpha, int* beta)//搜索中只有移动
{
	int point;
	int over = 0;
	for (int move = 0; move <= *moveWay - 1; move++)
	{
		place(oneMove[findStep - 1][move], 4 * (*pieces) + move + 50);
		if (findStep == 10)//修改层数
		{
			point = -estimate();
		}
		else
			point = -minMax(-(*beta), -(*alpha));//负极大值
		if (point > * bestMove)
		{
			*bestMove = point;
			if (findStep == 1)
			{
				memcpy(&command, &oneMove[findStep - 1][move], size);  //储存最优选择
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
 * 轮到你落子。
 * 棋盘上0表示空白，1表示黑棋，2表示白旗
 * me表示你所代表的棋子(1或2)
 * 你需要返回一个结构体Command，其中numStep是你要移动的棋子经过的格子数（含起点、终点），
 * x、y分别是该棋子依次经过的每个格子的横、纵坐标
 */

int minMax(int alpha, int beta)//极大极小
{
	findStep++;
	int bestMove = -10000000;//简单初始化
	int num = 0;
	int numChecked = 0;
	int step = 0;
	int moveWay = 0;
	int pieces = 0;
	int over = 0;
	char boardX[12] = { 0 }, boardY[12] = { 0 };
	if (aiFind(&num, &numChecked, boardX, boardY) == 1)
	{
		for (step = 0; step < num; step++)//遍历所有吃子步法
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
			for (pieces = 0; pieces < numChecked; pieces++)//棋子开始遍历
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
 * 你的代码结束
 */

 //.X.X.X.X
 //X.X.X.X.
 //.X.X.X.X
 //........
 //........
 //O.O.O.O.
 //.O.O.O.O
 //O.O.O.O.
void start(int flag) //开始
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

void turn() //自己行棋
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

void end(int x)  //结束
{
	exit(0);
}

void loop() //主体
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