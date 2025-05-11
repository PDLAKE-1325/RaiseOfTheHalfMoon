#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>

// #include <stdio.h>
// #include <stdlib.h>
// #include <math.h>
// #include <time.h>
// #include <stdbool.h>

#ifdef _WIN32
#include <conio.h>
#include <windows.h> // sleep
#else
#include <termios.h>
#include <fcntl.h>
#include <term.h>
#include <unistd.h> // usleep
#endif

// 색프린트
#define Background "48;2"
#define Font "38;2"
#define cprint(str, style, r, g, b) printf("\033[%s;%d;%d;%dm%s\033[0m",style,r,g,b,str);

// 구조체
#ifndef STRUCTS
#define STRUCTS
// 카드
typedef struct object
{
	char phase;
    bool team;
}object;
// 노드 만들때 자료형마다 하나씩 한는거 싫으면 union 이나 void 있다는데, 안전하고 직관적으로 그냥 다 선언함.
typedef struct node node;
typedef struct _link
{
	node* target;
	struct _link* next;
}_link;
struct node
{
    object* obj_card;
    struct _link* linked_nodes;
	node* next;
};
// 맵 
// 메모리 효율적인 대신 포인터 조작 힘든 방법이랑
// 메모리 비효률적인데 쉽게 하는 방법 중 쉬운걸로 함.
// 그래도 포인터 조작 힘든 방법으로 할 때 어떻게 하려했는지 설명
// int** arr[] 로 한다음 포인터 포인터 주소 +1식 해가면서 탐색해서 -1이면 끝으로 인식하게하려함
// 각각 메모리 할당과 해제 부분도 따로 함수로 만들려고 하고.
// 근데 이방식하면 코드 실행시간이나 복잡도 많이 올라서 그냥 안하기로 함. 맵 데이터 얼마나된다고..
typedef struct stage{
	char node_cnt; // 메모리 절약
	char* node_links; 
}stage;
typedef struct map
{
	int map_cnt;
	node* nodes;
}map;
// 게임
typedef struct game
{
	// 겜 메인 함수
	void(*start)(struct game*);
	void(*finish)(struct game*);
	void(*update)(struct game*);
	
	// 겜 정보
	bool is_test;
    bool is_finished;
	int TPS;

	// 겜 데이터
	bool input[256];
	int game_phase;
	map* gameboard;
}game;
#endif

void launch(game* game);
game* newGame(void(*start)(game*), void(*finish)(game*), void(*update)(game*), int TPS, bool test);