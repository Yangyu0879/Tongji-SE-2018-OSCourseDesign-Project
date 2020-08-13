#include "stdio.h"

/// <summary>
/// 你最爱的全新版本
//具体测试在te里面做
/// </summary>

#define WIDTH 8
#define HEIGHT 8





/**
*	0	表示空
*	1	表示墙
*	2	表示人
*	3	表示箱子
*	4	表示球
*	5	表示推到目的地的箱子
*/
const int Cmap[HEIGHT][WIDTH] = {
	{0,0,0,1,1,1,1,0},
	{0,1,1,1,0,2,1,0},
	{0,1,0,0,3,0,1,0},
	{1,1,0,1,0,1,1,1},
	{1,0,0,1,0,1,1,1},
	{1,0,1,0,0,0,4,1},
	{1,0,3,0,0,0,4,1},
	{1,1,1,1,1,1,1,1}
};




int map[HEIGHT][WIDTH];

int x, y;


int boxs;




void initData() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			map[i][j] = Cmap[i][j];
		}
	}
	int i, j;

	printf("loading......");

	for (i = 0; i < HEIGHT; i++) {
		for (j = 0; j < WIDTH; j++) {
			if (map[i][j] == 2) {
				x = j;
				y = i;
			}
			if (map[i][j] == 3) {
				boxs++;
			}
		}
	}
}






void PrintM() {
	int i, j;
	for (i = 0; i < WIDTH; i++) {
		for (j = 0; j < HEIGHT; j++) {
			switch (map[i][j]) {
			case 0:
				printf("  ");
				break;
			case 1:
				printf("# ");
				break;
			case 2:
				printf("M ");
				break;
			case 3:
				printf("B ");
				break;
			case 4:
				printf("O ");
				break;
			case 5:
				printf("X ");
				break;
			}
		}
		printf("\n");
	}

	printf("M is you\n");
	printf("# is wall\n");	
	printf("B is Box\n");
	printf("O is your target\n");
	printf("input w a s d to move ,input q to quit\n");
	
}






void Move(int dir) {
	int newx, newy, boxx, boxy;
	switch (dir)
	{
	case 1: {
		if (y == 0) {
			return;
		}
		newx = x;
		newy = y - 1;
		boxx = x;
		boxy = y - 2;
		break;
	}
	case 2: {
		if (x == 0) {
			return;
		}
		newx = x - 1;
		newy = y;
		boxx = x - 2;
		boxy = y;
		break;
	}
	case 3: {
		if (y == HEIGHT - 1) {
			return;
		}
		newx = x;
		newy = y + 1;
		boxx = x;
		boxy = y + 2;
		break;
	}
	case 4: {
		if (x == WIDTH - 1) {
			return;
		}
		newx = x + 1;
		newy = y;
		boxx = x + 2;
		boxy = y;
		break;
	}
	default:
		break;
	}
	if (map[newy][newx] == 5) {
		return;
	}
	if (map[newy][newx] == 1) {
		return;
	}
	if (map[newy][newx] == 3) {
		if (map[boxy][boxx] == 1) {
			return;
		}

		if (map[boxy][boxx] == 4) {
			map[boxy][boxx] = 5;
			map[newy][newx] = 0;

			boxs--;
		}
		else {
			map[boxy][boxx] = 3;
		}
	}
	map[y][x] = 0;
	map[newy][newx] = 2;
	x = newx;
	y = newy;
}



void MoveBox(int fd_stdin) {

	char buf[80] = { 0 };



	char direction;
	initData();
	while (1) {
		clear();
		PrintM();

		if (!boxs) {
			break;
		}
		read(fd_stdin, buf, 2);
		direction = buf[0];
		switch (direction) {
		case 'w':
			Move(1);
			break;
		case 'a':
			Move(2);
			break;
		case 's':
			Move(3);
			break;
		case 'd':
			Move(4);
			break;
		case 'q':
			return;
			break;
		}
		
	}
	printf("you win!!!\n");
}






