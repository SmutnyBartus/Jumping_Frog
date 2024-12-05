#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>

#define RIVER_COLOR 1
#define ROAD_COLOR 2
#define LOG_COLOR 3
#define FROG_COLOR 4
#define CAR_COLOR 5

#define PLAYWIN_WIDTH 61
#define PLAYWIN_HEIGHT 40

WINDOW* start()
{
	WINDOW* win;

	if ( (win = initscr()) == NULL ) {
		fprintf(stderr, "Error initialising ncurses.\n");
		exit(0);
    	}

    if(has_colors())
	    start_color();

    init_pair(RIVER_COLOR, COLOR_CYAN, COLOR_CYAN);
    init_pair(ROAD_COLOR, COLOR_BLACK, COLOR_WHITE);
    init_pair(LOG_COLOR, COLOR_RED, COLOR_RED);
    init_pair(FROG_COLOR, COLOR_GREEN, COLOR_GREEN);
    init_pair(CAR_COLOR, COLOR_MAGENTA, COLOR_MAGENTA);

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
} FROG;

typedef struct{
    int x, y;
    char model;
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
    wattroff(window, COLOR_PAIR(CAR_COLOR));
}

void init_frog(WINDOW* window, FROG *frog){
    frog->x = PLAYWIN_WIDTH / 2;
    frog->y = PLAYWIN_HEIGHT - 1;
    frog->model = 'X';
    draw_frog(window, *frog);
}

void draw_car(WINDOW* window, CAR car){
    wattron(window, COLOR_PAIR(CAR_COLOR));
    mvwaddch(window, car.y, car.x, car.model);
    wattroff(window, COLOR_PAIR(CAR_COLOR));
}

void init_car(WINDOW* window, CAR* car, int n){
    if(rand() % 2 == 1){
        car->x = PLAYWIN_WIDTH;
        car->model = '<';
    }
    else{
        car->x = 0;
        car->model = '>';
    }
    car->y = n;
    draw_car(window, *car);
}

void process_input(FROG* frog, char c){
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

void mainloop(WINDOW* window) {
    srand(time(NULL));
    int frog_frames = 10; //frames for frogs
    int frames = 0; //frames for every other object
    FROG frog;
    CAR car[10];

    int tab[PLAYWIN_HEIGHT];
    for(int i = 0; i < PLAYWIN_HEIGHT; i++)
        tab[i] = i;

    shuffle(tab, PLAYWIN_HEIGHT);


    init_frog(window, &frog);

    for(int i = 0; i < 10; i++)
        init_car(window, &car[i], tab[i]);

    wrefresh(window);

    nodelay(window, TRUE);

    while (1) {
        //hande user input
        int c = wgetch(window);
        if (c == 'q') {
            return;
        }


        //handle car movement
        if(frames == 4){
            for(int i = 0; i < 10; i++){
                mvwaddch(window, car[i].y, car[i].x, ' ');

                if(car[i].model == '<')
                    car[i].x--;
                else if(car[i].model == '>')
                    car[i].x++;

                //to do make the car[i] wait before apearing again
                if(car[i].x < 0 || car[i].x > PLAYWIN_WIDTH){
                    if(car[i].model == '<')
                        car[i].x = PLAYWIN_WIDTH;
                    else if(car[i].model == '>')
                        car[i].x = 0;
                }

                draw_car(window, car[i]);

                frames = 0;
            }
        }


        //handle frog movement
        if (c != ERR && frog_frames >= 10) {
            mvwaddch(window, frog.y, frog.x, ' ');
            process_input(&frog, c);
            draw_frog(window, frog);
            frog_frames = 0;
        }

        //check collsion
        for(int i = 0; i < 10; i++)
            if(frog.x == car[i].x && frog.y == car[i].y){
                return;
            }


        // Render world
        wrefresh(window);

        // Wait
        frog_frames++;
        frames++;
        usleep(33333);
    }
}


int main(){

    WINDOW* mainwin = start();

    WINDOW* playwin = subwin(mainwin, PLAYWIN_HEIGHT, PLAYWIN_WIDTH, 0, 0);
    wbkgd(playwin, COLOR_PAIR(ROAD_COLOR));

    mainloop(playwin);

    delwin(playwin);
    delwin(mainwin);
    endwin();
}