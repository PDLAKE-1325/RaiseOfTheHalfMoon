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
#pragma region 
char map0_links[] = 
{1,3,-1,
0,2,4,-1,
1,5,-1,
0,4,6,-1,
1,3,5,7,-1,
2,4,8,-1,
3,7,-1,
4,6,8,-1,
5,7,-2};
char map1_links[] = 
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
10,11,1,-2};
#pragma endregion
stage stages[] = {
    /*
    [0] - [1] - [2]
     |     |     |
    [3] - [4] - [5]
     |     |     |
    [6] - [7] - [8]
    */
   {9,map0_links},
   /*
    [00] - [01] - [02] - [03]
     |   ╲             ╱  |
    [04]   [05] - [06]   [07]
     |      |      |      |
    [08]   [09] - [10]   [11]
     |   ╱             ╲  |
    [12] - [13] - [14] - [15]
    */
   {15,map1_links},
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
// node* getNode(node* head, int index){
//     // if(head==NULL)  return NULL;
//     node* cur = head;
//     for (int i = 0; i < index; i++){
//         cur = cur->next;
//         // if(cur==NULL) return NULL;
//     }
//     return cur;
// }
void countNode(node* head){
    int cnt=0;
    while (head!=NULL)
        cnt++;
    printf("%d",cnt);
}
void createBoard(map* map){ //Node* nodes[] 받아옴
    printf("=2");
    // int rnd = rand()%map->map_cnt;
    int rnd = 0;
    stage selected_stage = stages[rnd];
    char node_cnt = selected_stage.node_cnt;
    map->nodes = createNode();
    for (int i=0; i<node_cnt-1; i++){
        node* new_node = createNode();
        node* cur = map->nodes;
        while (cur->next!=NULL)
        {
            cur = cur->next;
        }
        cur->next = new_node;
    }
    int node_num=0;
    countNode(map->nodes);
    for (int i=0;;i++){
        char cur = selected_stage.node_links[i]; // 유지보수 쉽고 보기좋음
        if(cur==-1) node_num++;
        else if(cur==-2) break;
        else {
            printf("linked %d -> %d\n",node_num,cur);
        }
    }
}
static void final_finish(game* game){
    if(!game->is_test)
        system("clear");
    if(game->gameboard->nodes!=NULL){
        node* cur = game->gameboard->nodes;
        while(cur!=NULL){
            node* next = cur->next;
            _link* cur_link = cur->linked_nodes;
            while (cur_link!=NULL){
                _link* next = cur_link->next;
                free(cur_link);
                cur_link = next;
            }
            free(cur->obj_card);
            free(cur);
            cur = next;
        }
    }
    free(game->gameboard);
}
bool _keys[256];
unsigned char __keys[] = {'w','a','s','d'};
static void setActiveKeys(unsigned char* keys, int size){
    for (int i = 0; i < 256; i++)
        _keys[i]=false;

    for (int i = 0; i < size; i++)
        _keys[keys[i]]=true;
}
static void input(game* game){
    for (int i=0;i<256;i++)
        game->input[i] = false;

    while (_kbhit())
    {
        unsigned char ch = _getch();
        if(!_keys[ch]) continue;
        game->input[ch] = true;
    }
}
static void render(game* game){

}
static void tick(game* game){
    if(!game->is_test)
        system("clear");
    input(game);
    game->update(game);
    render(game);
}
void launch(game* game){
    printf("=1");
    game->start(game);
    createBoard(game->gameboard);
    printf("=/1");
    while (!game->is_finished){
        tick(game); 
#ifdef _WIN32
			Sleep((int)((1.0 / game->TPS) * 1000));
#else
			usleep((int)((1.0 / game->TPS) * 1000000));
#endif
    }
    game->finish(game);
    final_finish(game);
}
static map* newMap(){
    map* map = (struct map*)malloc(sizeof(struct map));
    map->nodes=NULL;
    map->map_cnt = sizeof(stages)/sizeof(stage);
    return map;
}
game* newGame(void(*start)(game*), void(*finish)(game*), void(*update)(game*), int TPS, bool test){
    if(!TPS) TPS=10;
	game* game = (struct game*)malloc(sizeof(struct game));
    for (int i = 0; i < 256; i++)
        game->input[i]=false;    
    setActiveKeys(__keys,sizeof(__keys));
    game->is_test = test;
    game->TPS = TPS;
    game->start = start;
    game->update = update;
    game->finish = finish;
    game->game_phase = 0;
    game->is_finished = false;
    printf("=0\n");
    game->gameboard = newMap();
    printf("=/0\n");
    return game;
}
