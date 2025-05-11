#include "Rothm.h"

void start(game* game){
    printf("실행");
}
void update(game* game){
    if(game->input['d']) {
        game->is_finished = true;
    }
}
void finish(game* game){
    printf("종료");
}

int main(){
    srand(time(NULL));
    printf("=-1");
    
    game* game = newGame(start,finish,update,10,true);
    printf("=2");
    launch(game);
    printf("=/2");
}
// gcc Rothm_main.c Rothm.c -o Rothm_main