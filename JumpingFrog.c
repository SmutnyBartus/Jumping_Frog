#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>

#define CAR_COLOR 1
#define FROG_COLOR 2
#define LEAF_COLOR 3
#define MAP_COLOR 4
#define ROAD_COLOR 5
#define DARK_GREEN COLOR_GREEN + 1

#define PLAYWIN_WIDTH 37
#define PLAYWIN_HEIGHT 22
#define ROADS 15

WINDOW* start()
{
	WINDOW* win;

	if ( (win = initscr()) == NULL ) {
		fprintf(stderr, "Error initialising ncurses.\n");
		exit(0);
    	}

    if(has_colors())
	    start_color();

    init_color(DARK_GREEN, 0, 400, 0);

    init_pair(CAR_COLOR, COLOR_BLACK, COLOR_RED);
    init_pair(FROG_COLOR, COLOR_BLACK, DARK_GREEN);
    init_pair(LEAF_COLOR, COLOR_WHITE, COLOR_GREEN);
    init_pair(MAP_COLOR, COLOR_BLACK, COLOR_CYAN);
    init_pair(ROAD_COLOR, COLOR_WHITE, COLOR_BLACK);

    refresh();
    keypad(win, 1);
	noecho();
	curs_set(0);
    raw();
	return win;
}

typedef struct{
    int x, y;
    char model;
    chtype prev;
} FROG;

typedef struct{
    int x, y;
    char model;
    int speed;
} CAR;

void shuffle(int arr[], int n) {
    for (int i = n - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

void draw_frog(WINDOW* window, FROG frog){
    wattron(window, COLOR_PAIR(FROG_COLOR));
    mvwaddch(window, frog.y, frog.x, frog.model);
    wattroff(window, COLOR_PAIR(FROG_COLOR));
}

void draw_car(WINDOW* window, CAR car){
    wattron(window, COLOR_PAIR(CAR_COLOR));
    mvwaddch(window, car.y, car.x, car.model);
    wattroff(window, COLOR_PAIR(CAR_COLOR));
}

void draw_road(WINDOW* window, CAR car){
    wattron(window, COLOR_PAIR(ROAD_COLOR));
    mvwaddch(window, car.y, car.x, '-');
    wattroff(window, COLOR_PAIR(ROAD_COLOR));
}
void init_frog(WINDOW* window, FROG *frog){
    frog->x = PLAYWIN_WIDTH / 2;
    frog->y = PLAYWIN_HEIGHT - 1;
    frog->model = 'X';
    frog->prev = ' ';
    draw_frog(window, *frog);
}

void init_car(WINDOW* window, CAR* car, int y){
    if(rand() % 2 == 1){
        car->x = PLAYWIN_WIDTH;
        car->model = '<';
    }
    else{
        car->x = 0;
        car->model = '>';
    }
    car->y = y;
    car->speed = rand() % 5 + 2;
    draw_car(window, *car);
}

void init_map(WINDOW* window){
    for(int i = 0; i < PLAYWIN_WIDTH; i++){
        if(i % 9 == 0){//draw_leaf
            wattron(window, COLOR_PAIR(LEAF_COLOR));
            mvwaddch(window, 0, i, 'O');
            wattroff(window, COLOR_PAIR(LEAF_COLOR));
        }
        else{
            //draw whats bushes
            wattron(window, COLOR_PAIR(MAP_COLOR));
            mvwaddch(window, 0, i, ' ');
            wattroff(window, COLOR_PAIR(MAP_COLOR));
        }
    }
}

void init_cars(WINDOW* window, CAR car[]){
    int tab[PLAYWIN_HEIGHT - 2];

    for(int i = 0; i < PLAYWIN_HEIGHT - 2; i++)
        tab[i] = i + 1;

    shuffle(tab, PLAYWIN_HEIGHT - 2);

    for(int i = 0; i < ROADS; i++){
        for(int j = 0; j < PLAYWIN_WIDTH; j++){
            wattron(window, COLOR_PAIR(ROAD_COLOR));
            mvwaddch(window, tab[i], j, '-');
            wattroff(window, COLOR_PAIR(ROAD_COLOR));
        }
        init_car(window, &car[i], tab[i]);
    }

}

void process_frog_input(FROG* frog, char c){
    switch(c){
        case 'w':
            if(frog->y > 0)
                --frog->y;
            break;
        case 's':
            if((frog->y + 1) < PLAYWIN_HEIGHT)
                ++frog->y;
            break;
        case 'a':
            if(frog->x > 0)
                --frog->x;
            break;
        case 'd':
            if((frog->x + 1) < PLAYWIN_WIDTH)
                ++frog->x;
            break;
    }
}

int won(WINDOW* window, FROG frog){
    for (int i = 0; i < PLAYWIN_WIDTH; i += 9) {
        if ((mvwinch(window, 0, i) & A_CHARTEXT) != frog.model) {
            return 0;
        }
    }   
    return 1;
}

int collision(WINDOW* window, FROG frog, CAR car[]){
    int err;
    for(int i = 0; i < ROADS; i++){
        if(frog.y == car[i].y) {

            if(frog.x < car[i].x)
                err = 1;
            else
                err = - 1;

            if(frog.x == car[i].x || frog.x == car[i].x + err)
                    return 1;
        }
    }
    return 0;
}


void is_frog_at_leaf(WINDOW* window, FROG *frog){
    if(frog->y == 0 && frog->x % 9 == 0){
        frog->prev = ' ';
        draw_frog(window, *frog);
        frog->x = PLAYWIN_WIDTH / 2;
        frog->y = PLAYWIN_HEIGHT - 1;
        mvwaddch(window, frog->y, frog->x, frog->model);
        draw_frog(window, *frog);
    }

}

void handle_frog_movement(WINDOW* window, FROG* frog, char c){
    mvwaddch(window, frog->y, frog->x, frog->prev);
    process_frog_input(frog, c);
    frog->prev = mvwinch(window, frog->y, frog->x);
    draw_frog(window, *frog);
}

void handle_car_movement(WINDOW* window, CAR car[], int frames){
    for(int i = 0; i < ROADS; i++){
        if(frames % car[i].speed == 0){
            draw_road(window, car[i]);

            if(car[i].model == '<')
                car[i].x--;
            else if(car[i].model == '>')
                car[i].x++;

            //to do make the car[i] wait before apearing again
            if(car[i].x < 0 || car[i].x > PLAYWIN_WIDTH){
                if(car[i].model == '<'){
                    init_car(window, &car[i], car[i].y);
                }
                else if(car[i].model == '>'){
                    init_car(window, &car[i], car[i].y);
                }
            }

            draw_car(window, car[i]);
        }
    }   
}

void mainloop(WINDOW* window) {
    nodelay(window, TRUE);
    srand(time(NULL));

    int frog_frames = 10; //frames for frog
    int frames = 0; //frames for every other object

    FROG frog;
    CAR car[ROADS];

    init_map(window);
    init_cars(window, car);
    init_frog(window, &frog);

    wrefresh(window);

    while (!won(window, frog)) {

        //hande user input
        int c = wgetch(window);
        if (c == 'q') {
            return;
        }
        else if(c == 'p'){
            getch();
        }

        //handle car movement
        handle_car_movement(window, car, frames);

        //handle frog movement
        if (c != ERR && frog_frames >= 10) {
            handle_frog_movement(window, &frog, c);
            frog_frames = 0;
        }

        //check collsion
        if(collision(window, frog, car))
            return; // handle what happends when frog hits a car or car hits the frog

        //if frog jumps on leaf, reset frog
        is_frog_at_leaf(window, &frog);

        // Render world
        wrefresh(window);

        // Wait
        frog_frames++;
        frames++;

        if(frames > 1000)
            frames = 0;
        usleep(33333);
    }
}

int main(){
    WINDOW* mainwin = start();

    WINDOW* playwin = subwin(mainwin, PLAYWIN_HEIGHT, PLAYWIN_WIDTH, 0, 0);
    wbkgd(playwin, COLOR_PAIR(MAP_COLOR));

    mainloop(playwin);

    delwin(playwin);
    delwin(mainwin);
    endwin();
}