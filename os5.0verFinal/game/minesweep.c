
#include "type.h"
#include "stdio.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "fs.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"






/*======================================================================*/

/* global variable of game Tictactoe */
#define FALSE 0
#define TRUE 1




#define RANDOM_MAX 0x7FFFFFFF


int map[9][9];
int mine[9][9];
char presentMap[9][9];
int count = 81;



static long do_rand(unsigned long* value)
{
	long quotient, remainder, t;

	quotient = *value / 127773L;
	remainder = *value % 127773L;
	t = 16807L * remainder - 2836L * quotient;
	if (t <= 0)
		t += 0x7FFFFFFFL;
	return ((*value = t) % ((unsigned long)RANDOM_MAX + 1));
}



static unsigned long next = 1;



int rand(void)
{
	return do_rand(&next);
}



void srand(unsigned int seed)			//赋初值为种子
{
	next = seed;
}




void add(int x, int y) {
	if (x - 1 >= 0 && y - 1 >= 0) {
		map[x - 1][y - 1]++;
	}
	if (x - 1 >= 0) {
		map[x - 1][y]++;
	}
	if (x - 1 >= 0 && y + 1 <= 8) {
		map[x - 1][y + 1]++;
	}
	if (y - 1 >= 0) {
		map[x][y - 1]++;
	}
	if (y + 1 <= 8) {
		map[x][y + 1]++;
	}
	if (x + 1 <= 8 && y - 1 >= 0) {
		map[x + 1][y - 1]++;
	}
	if (x + 1 <= 8) {
		map[x + 1][y]++;
	}
	if (x + 1 <= 8 && y + 1 <= 8) {
		map[x + 1][y + 1]++;
	}
}



void Init() {
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			mine[i][j] = 0;
			map[i][j] = 0;
			presentMap[i][j] = "X";
		}
	}

	for (int i = 0; i < 10; i++)
	{
		int a = rand() % 81;
		int x = a / 9;
		int y = a % 9;
		if (mine[x][y] == 0) {
			mine[x][y] = 1;
			add(x, y);
		}
		else {
			i--;
		}
	}
	return;
}


void PrintMap() {
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			printf("%c ", presentMap[i][j]);
		}
		printf("\n");
	}
}

void testMap(){//for test
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (mine[i][j] == 1) {
				printf("* ");
			}
			else {
				printf("%d ", map[i][j]);
			}
		}
		printf("\n");
	}
}


void Minesweeper() {
	Init();
	PrintMap();
	
	return;
}

int Judge(int x, int y) {
	if (count == 10) {//如果只剩下十个雷
		return 1;
	}
	if (mine[x][y] == 1) {//说明有雷
		return 1;
	}
	else {
		presentMap[x][y] = map[x][y] + 48;
		count--;
		return 0;
	}
}

int UserInput(int fd_stdin) {
	int n;
	int x, y;
	int result = 0;
	char szCmd[80] = { 0 };
L1:	printf("choose the position you want to sweep(x):");
	n = read(fd_stdin, szCmd, 80);
	szCmd[1] = 0;
	atoi(szCmd, &x);
	printf("choose the position you want to sweep(y):");
	n = read(fd_stdin, szCmd, 80);
	szCmd[1] = 0;
	atoi(szCmd, &y);

	if ((x >= 1 && x <= 9) &&
		(y >= 1 && y <= 9)) {
		//		printf("input correct!\n");
		result = Judge(x - 1, y - 1);
	}
	else {
		printf("Input error!!!!!!\n");
		goto L1;
	}
	return result;
}



void Minesweeper1(int fd_stdin) {
	count = 100;
	char buf[80] = { 0 };
	char IsFirst = 0;
	int IsFinish = FALSE;
	while (!IsFinish)
	{
		printf("Game Start:(9*9map)\n");
		Minesweeper();
		do {
			IsFinish = UserInput(fd_stdin);
			if (!IsFinish) {
				PrintMap();
			}
		} while (!IsFinish);
		if (IsFinish)
		{
			if (count > 10) {
				printf("boom!!!!\nyou lose\n\n");
			}
			else {
				printf("you get all the mine!\n");
			}

			printf("Play Again?(y/n)");
			char cResult;
			read(fd_stdin, buf, 2);
			cResult = buf[0];
			printf("%c", cResult);
			if (cResult == 'y')
			{
				clear();
				IsFinish = FALSE;
				count = 100;
			}
			else
			{
				clear();
			}
		}
	}
}



