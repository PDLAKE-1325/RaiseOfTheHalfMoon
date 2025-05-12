#include "Rothm.h"
#ifndef _WIN32
int _kbhit()
{
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
}
int _getch()
{
	struct termios oldt, newt;
	int ch;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

	return ch;
}
#endif

// 맵 정보
// 첫수는 노드 수 -1로 노드 구분 -2로 끝구분, 따로 연결 리스트 구조체 안만들어서 하기로함. 안정성 떨어지는데 귀찮음...
// 그리고 이어지는 노드는 무조건 해당 노드 8방 위치 중 하나에 있어야함
// 맵 노드 위치도 정사각형 아니게 짤 수도 있었는데 너무 귀찮음
#pragma region 
int map0_links[] = 
{1,3,-1,
0,2,4,-1,
1,5,-1,
0,4,6,-1,
1,3,5,7,-1,
2,4,8,-1,
3,7,-1,
4,6,8,-1,
5,7,-2};
int map1_links[] = 
{1,4,5,-1,
0,2,-1,
1,3,-1,
2,6,7,-1,
0,8,-1,
0,6,9,-1,
3,5,10,-1,
3,11,-1,
4,12,-1,
5,10,12,-1,
6,9,15,-1,
7,15,-1,
8,9,13,-1,
12,14,-1,
13,15,-1,
10,11,14,-2};
int map2_links[] ={
    4,-1,
    4,-1,
    4,-1,
    4,-1,
    0,1,2,3,5,6,7,8,-1,
    4,-1,
    4,-1,
    4,-1,
    4,-2
};
#pragma endregion
stage stages[] = {
// ╱ :2, ╲ :4, ╳ :6, - :8, | :10   == 랜더할때
    /*
    [0] - [1] - [2]
     |     |     |
    [3] - [4] - [5]
     |     |     |
    [6] - [7] - [8]
    */
   {8,3,map0_links},
   /*
    [00] - [01] - [02] - [03]
     |   ╲             ╱  |
    [04]   [05] - [06]   [07]
     |      |      |      |
    [08]   [09] - [10]   [11]
     |   ╱             ╲  |
    [12] - [13] - [14] - [15]
    */
   {15,4,map1_links},
   /*
    [0] - [1] - [2]
     |     |     |
    [3] - [4] - [5]  / 4쪽으로 모인형태 ㄱㄱ
     |     |     |
    [6] - [7] - [8]
    */
//    {9,3,map2_links},
};
// ╱ ╳ ╲ 
//🌑🌒🌓🌔🌕🌖🌗🌘

node* createNode(){
    node* node = (struct node*)malloc(sizeof(struct node));
    node->linked_nodes = NULL;
    node->obj_card = NULL;
    node->next = NULL;
    return node;
}

void connectNode(node* from, node* to){
    _link* __link = (struct _link*)malloc(sizeof(struct _link));
    __link->target = to;
    __link->next = from->linked_nodes;
    from->linked_nodes = __link;
}

node* createNodes(int cnt){
    node* head = createNode();
    node* cur = head;
    for (int i = 0; i < cnt; i++){
        cur->next = createNode();
        cur = cur->next;
    }
    return head;
}

node* getNode(node* head, int index){
    node* cur = head;
    for (int i = 0; i < index; i++)
        cur = cur->next;
    return cur;    
}

int getNodeIndex(node* head, node* target){
    node* cur = head;
    int index = 0;
    while (cur!=NULL&&cur!=target){
        index++;
        cur = cur->next;
    }
    if(cur==NULL) return -1;
    return index;
}

// 노드 연결선 인덱스 찾기
#pragma region 
int leftUp(int n, int w) { return (n/w)*(w*3-2)-(w*(n/w+1)-n)*2; }
int up(int n, int w) { return (n/w)*(w*3-2)-(w*(n/w+1)-n)*2+1; }
int rightUp(int n, int w) { return (n/w)*(w*3-2)-(w*(n/w+1)-n)*2+2; }
int left(int n, int w) { return (n/w)*(w*2-2)+n-1; }
int right(int n, int w) { return (n/w)*(w*2-2)+n; }
int leftDown(int n, int w) { return (n+w)/w*(w*3-2)-(w*(n/w+1)-n)*2; }
int down(int n, int w) { return (n+w)/w*(w*3-2)-(w*(n/w+1)-n)*2+1; }
int rightDown(int n, int w) { return (n+w)/w*(w*3-2)-(w*(n/w+1)-n)*2+2; }
#pragma endregion
int(*linkerPos[])(int, int) = {leftUp, up, rightUp, left, right, leftDown, down, rightDown};
int linkerPositionCheck(int width, int cur_pos, int target){

    // -w-1 -w 1-w
    // -1 cur 1
    // w-1 w w+1
    int offset[] ={-width-1, -width, 1-width, -1, 1, width-1, width, width+1};
    for (int i = 0; i < 8; i++)
        if(cur_pos+offset[i]==target) return i;
    return -1;
}

void createBoard(map* map){ //Node* nodes[] 받아옴
    int rnd = rand()%map->map_cnt;
    map->map_num = rnd;
    stage selected_stage = stages[rnd];
    map->map_width = selected_stage.width;
    int node_cnt = selected_stage.node_cnt;
    map->nodes = createNodes(node_cnt);
    node* cur_node = map->nodes;
    for (int i=0;;i++){
        int cur = selected_stage.node_links[i];
        if(cur==-1) cur_node = cur_node->next;
        else if(cur==-2) break;
        else connectNode(cur_node,getNode(map->nodes,cur));
    }
    cur_node = map->nodes;
    int w = selected_stage.width;
    int linker_size = (w-1)*w+(w+w-1)*(w-1)+1;
    int* linkers = (int*)malloc(linker_size * sizeof(int));
    int add_offset[]={2,5,1,4,4,1,5,2};
    // ╱ :1, ╲ :2, ╳ :3, - :4, | :5    == 체크할 때 더할 값
    // 양쪽 연결이니 두번씩 더해질거라 x2함
    // ╱ :2, ╲ :4, ╳ :6, - :8, | :10   == 랜더할때
    memset(linkers,0,linker_size*sizeof(int));
    while (cur_node!=NULL){
        _link* cur_link = cur_node->linked_nodes;
        while (cur_link!=NULL)
        {
            int n = getNodeIndex(map->nodes,cur_node);
            int target_n = getNodeIndex(map->nodes,cur_link->target);
            int lpc = linkerPositionCheck(w,n,target_n);
            if(lpc!=-1)
                linkers[linkerPos[lpc](n,w)]+=add_offset[lpc];
            else printf("버그남 ㅋㅋ");
            cur_link = cur_link->next;
        }
        cur_node = cur_node->next;
    }
    map->char_linkers = linkers;
}
// ╱ :1, ╲ :2, ╳ :3, - :4, | :5    == 체크할 때 더할 값
// 양쪽 연결이니 두번씩 더해질거라 x2함
// ╱ :2, ╲ :4, ╳ :6, - :8, | :10   == 랜더할때

static void freeGame(game* game){
    node* cur = game->gameboard->nodes;
    free(game->gameboard->char_linkers);
    while(cur!=NULL){
        node* next = cur->next;
        _link* cur_link = cur->linked_nodes;
        while (cur_link!=NULL){
            _link* next = cur_link->next;
            free(cur_link);
            cur_link = next;
        }
        if(cur->obj_card!=NULL)
            free(cur->obj_card);
        free(cur);
        cur = next;
    }
    for (int i = 0; i < 3; i++)
        free(game->card_queue[i]);
    free(game->gameboard);
    free(game);
}

static void input(game* game){
    for (int i=0;i<256;i++)
        game->input[i] = false;

    while (_kbhit())
    {
        unsigned char ch = _getch();
        game->input[ch] = true;
    }
}

// int countNode(node* head){
//     node* cur = head;
//     int cnt=0;
//     while (cur!=NULL)
//     {
//         cnt++;
//         cur = cur->next;
//     }
//     return cnt;
// }

char moon_icon[][5] = {"🌑", "🌒", "🌓", "🌔", "🌕", "🌖", "🌗", "🌘"};
char line_icon[][5] = {" ", " ", "╱", " ", "╲", " ", "╳", " ", "-", " ", "|"};

//🌑:0 🌒:1 🌓:2 🌔:3 🌕:4 🌖:5 🌗:6 🌘:7
// ╱ :2, ╲ :4, ╳ :6, - :8, | :10   == 랜더할때
static void render(game* game){
    if(game->game_phase==1){
    int* linkers = game->gameboard->char_linkers;
    node* head = game->gameboard->nodes;

    int width = stages[game->gameboard->map_num].width;
    int line_str_idx = 0;
    for (int i = 0; i < width; i++)
    {
        for (int j = 0; j < width; j++)
        {
            bool on_cursor = false;
            if(game->gameboard->x_cursor==j&&game->gameboard->y_cursor==i) on_cursor = true;
            object* card = getNode(head,i*width+j)->obj_card;
            
            if(card==NULL){ 
                    cprint("[",Font,on_cursor?255:0,on_cursor?255:0,0);
                    printf("%2d",i*width+j);
                    cprint("]",Font,on_cursor?255:0,on_cursor?255:0,0);
            }
            else {
                cprint("[",Font,on_cursor?255:(card->team?255:0),on_cursor?255:0,on_cursor?255:(card->team?0:255));
                printf("%s",moon_icon[card->phase]);
                cprint("]",Font,on_cursor?255:(card->team?255:0),on_cursor?255:0,on_cursor?255:(card->team?0:255));
                }

            if(j!=width-1){
                // printf("%d",line_str_idx);
                printf("%s",line_icon[linkers[line_str_idx++]]);
            }
            // if(j!=width-1) printf("%s",line_icon[linkers[i*width+j]]);
        }
        printf("\n ");
        if(i!=width-1)
            for (int j = 0; j < width*2; j++)
                if(j!=width-1){
                    // printf("%d",line_str_idx);
                    if(j%2==0){
                        printf("%s  ",line_icon[linkers[line_str_idx++]]);
                    }else{
                        printf("%s ",line_icon[linkers[line_str_idx++]]);
                    }
                }
        printf("\n");
    }
    printf("\n| 카드 큐\n>");
    for (int i = 0; i < 3; i++)
        printf("[%s] ",moon_icon[game->card_queue[i]->phase]);
    printf("\n\n| 게임 정보\n%s", game->game_status);
    printf("\n\n\nW/A/S/D: 포인터 이동\nL: 카드 놓기\n0: 게임 종료\n\n");
    // 이하 디버깅용 프린트
    // int w = stages[game->gameboard->map_num].width;
    // int linker_size = (w-1)*w+(w+w-1)*(w-1)+1;
    // for (int i = 0; i < linker_size; i++)
    //     printf("[%d] -> %d\n",i, linkers[i]);
    }else if(game->game_phase==0){
        cprint("Raise Of The Half Moon\n\n",Font, 230,100,100);
        cprint("위상 쌍 : 같은 위상의 두 카드를 이어 배치 [1점]\n보름달 쌍 : 반대 위상의 두 카드 조합 [2점]\n주기 점수 : 연속된 위상을 3장 이상 배치 [각 1점]\n카드 점수 : 모든 칸 소진 시 팀색 카드 수만큼 추가점수\n\n",Font, 0,120,200);
        cprint("Press 's' to Start\n\n",Font, 100,200,150);
    }else if(game->game_phase==2){
        printf("게임이 종료되었습니다.\n-아무 키나 눌러서 결과 확인-\n\n");
    }
    // int cnnt = countNode(game->gameboard->nodes);
    // printf("%d",cnnt);
}

static void tick(game* game){
    system("clear");
    input(game);
    game->update(game);
    render(game);
}

void launch(game* game){
    game->start(game);
    while (!game->is_finished){
        tick(game); 
#ifdef _WIN32
			Sleep((int)((1.0 / game->TPS) * 1000));
#else
			usleep((int)((1.0 / game->TPS) * 1000000));
#endif
    }
    game->finish(game);
    freeGame(game);
}

game* newGame(void(*start)(game*), void(*finish)(game*), void(*update)(game*), int TPS){
    if(!TPS) TPS=10;

	game* game = (struct game*)malloc(sizeof(struct game));
    map* map = (struct map*)malloc(sizeof(struct map));

    for (int i = 0; i < 256; i++)
        game->input[i]=false;

    game->is_finished = false;
    game->game_phase = 0;
    game->TPS = TPS;
    // game->game_status = "";
    game->card_queue = (object**)malloc(sizeof(object*)*3);

    game->start = start;
    game->update = update;
    game->finish = finish;

    game->gameboard = map;
    map->char_linkers = NULL;
    map->nodes = NULL;
    map->map_num = 0;
    map->map_width = 0;
    map->x_cursor=0;
    map->y_cursor=0;
    map->map_cnt = sizeof(stages)/sizeof(stage);

    createBoard(map);

    return game;
}




// 이하 메인

bool team = true;
int score[2]; // red,blue

void pointerUp(map* map){
    if(map->y_cursor>0) map->y_cursor--;
}
void pointerDown(map* map){
    if(map->y_cursor<map->map_width-1) map->y_cursor++;
}
void pointerLeft(map* map){
    if(map->x_cursor) map->x_cursor--;
}
void pointerRight(map* map){
    if(map->x_cursor<map->map_width-1) map->x_cursor++;;
}
object* GetRandomCard(){
    int rnd = rand()%8;
    object* new_card = (object*)malloc(sizeof(object));
    new_card->phase = rnd;
    new_card->team = team;
    team = team?false:true;
    return new_card;
}
void pullCard(game* game){
    for (int i = 0; i < 2; i++)
        game->card_queue[i] = game->card_queue[i+1];
    game->card_queue[2] = GetRandomCard();    
}
int CheckPhasePair(node* cur){
    int score = 0;
    _link* cur_link = cur->linked_nodes;
    while (cur_link!=NULL)
    {
        if(cur_link->target!=NULL&&cur_link->target->obj_card!=NULL){
            if(cur_link->target->obj_card->phase==cur->obj_card->phase){
                score++;
                cur_link->target->obj_card->team = cur->obj_card->team;
            }
        }cur_link = cur_link->next;
    }
    return score;
}
int CheckFullMoonPair(node* cur){
    int score = 0;
    _link* cur_link = cur->linked_nodes;
    while (cur_link!=NULL)
    {
        if(cur_link->target!=NULL&&cur_link->target->obj_card!=NULL){
            if(cur_link->target->obj_card->phase==(cur->obj_card->phase+4)%8){
                score+=2;
                cur_link->target->obj_card->team = cur->obj_card->team;
            }
        }cur_link = cur_link->next;
    }
    return score;
}
const int visit_size = 100;
//🌑:0 🌒:1 🌓:2 🌔:3 🌕:4 🌖:5 🌗:6 🌘:7

int getPhase(int cur, int n){
    if(cur+n==-1) return 7;
    return (cur+n)%8;
}
int CheckLunarCycle_(node* cur, node* visited[], int dir, int cnt){
    if (!cur || !cur->obj_card) return 0;
    visited[cnt] = cur;
    int score=0;
    if(cnt==3)score=3;
    else if(cnt>3) score = 1;
    node* visited_[visit_size];
    for (int i = 0; i < visit_size; i++) {
        visited_[i] = visited[i];
    }
    visited[cnt-1] = cur;
    _link* cur_link = cur->linked_nodes;
    while (cur_link!=NULL)
    {
        int flag=0;
        node* target = cur_link->target;
        if(!target|| !target->obj_card){
             cur_link = cur_link->next;
             continue;
        }
        for (int i = 0; i < visit_size; i++)
        {
            if(visited[i]==NULL) break;
            if(target==visited[i]) flag++;
        }
        if(!flag){
            if(target->obj_card->phase==getPhase(cur->obj_card->phase,dir)){
                int add = CheckLunarCycle_(target,visited,dir,++cnt);
                score+=add;
            }
        }
        cur_link = cur_link->next;
    }
    if(score!=0) cur->obj_card->team = team?false:true;
    return score;
}
int CheckLunarCycle(node* cur){
    int score=0;
    node* visited[visit_size];
    memset(visited, 0, sizeof(visited));
    visited[0] = cur;
    _link* cur_link = cur->linked_nodes;
    while (cur_link!=NULL)
    {
        node* target = cur_link->target;
        if (target && target->obj_card) {
        if(target->obj_card->phase==getPhase(cur->obj_card->phase,1)){
                score+=CheckLunarCycle_(target,visited,1,2);
        }else if(target->obj_card->phase==getPhase(cur->obj_card->phase,-1)){
            score+=CheckLunarCycle_(target,visited,-1,2);
        }
        }
        cur_link = cur_link->next;
    }
    return score;
    
    // int map_size = pow(game->gameboard->map_width,2);
    // node* visited[map_size];
    // memset(visited,0,sizeof(visited));
    // node* cur = cur;

}

void CheckGameEnd(game* game){
    node* cur = game->gameboard->nodes;
    while (cur!=NULL)
    {
        if(cur->obj_card==NULL) break;
        cur = cur->next;
    }
    if(cur==NULL){
        node* cur = game->gameboard->nodes;
        while (cur!=NULL)
        {
            if(cur->obj_card->team) score[0]++;
            else score[1]++;
            cur = cur->next;
        }
        game->game_phase=2;
    }
}
//🌑:0 🌒:1 🌓:2 🌔:3 🌕:4 🌖:5 🌗:6 🌘:7
void enterCard(game* game){
    map* map = game->gameboard;
    node* cur = getNode(map->nodes,(map->x_cursor)+(map->y_cursor)*map->map_width);
    if(cur->obj_card!=NULL) return;
    cur->obj_card = game->card_queue[0];
    int add_score=0;
    add_score+=CheckPhasePair(cur);
    add_score+=CheckFullMoonPair(cur);
    add_score+=CheckLunarCycle(cur);
    if(team) score[1]+=add_score;
    else score[0]+=add_score;
    pullCard(game);
    CheckGameEnd(game);
}


void start(game* game){
    printf("실행");
    for (int i = 0; i < 3; i++)
        game->card_queue[i] = GetRandomCard();
}

void update(game* game){
    if(game->game_phase==0&&game->input['s']) {
        game->game_phase=1;
    }else if(game->game_phase==1){
        if(game->input['0'])
            game->is_finished = true;
        else if(game->input['w']) pointerUp(game->gameboard);
        else if(game->input['a']) pointerLeft(game->gameboard);
        else if(game->input['s']) pointerDown(game->gameboard);
        else if(game->input['d']) pointerRight(game->gameboard);
        else if(game->input['l']) enterCard(game);
    }else if(game->game_phase==2){
        for (int i = 0; i < 256; i++)
            if(game->input[i]) game->is_finished =true;
    }
    if(game->game_phase==1){
    sprintf(game->game_status," Turn : %s\n Score(red:blue) %d:%d",team?"Blue":"Red",score[0],score[1]);
    }else if(game->game_phase==2){
        char* win;
        if(score[0]>score[1]) win="Red 승리";
        else if(score[0]<score[1]) win="Blue 승리";
        else win="무승부";
        sprintf(game->game_status,"최종스코어 Red %d : Blue %d\n\n> %s\n\n",score[0],score[1],win);
    }
}
void finish(game* game){
    printf("%s",game->game_status);
}

int main(){
    srand(time(NULL));
    
    game* game = newGame(start,finish,update,20);
    launch(game);
}