#include "Rothm.h"

void start(game* game){
    printf("시이작");
}
void update(game* game){
    if(game->input['d']) {
        game->is_finished = true;
    }
}
void finish(game* game){
    printf("끝");
}

int main(){
    srand(time(NULL));
    
    game* game = newGame(start,finish,update,10,true);
    launch(game);
}
// gcc Rothm_main.c Rothm.c -o Rothm_main