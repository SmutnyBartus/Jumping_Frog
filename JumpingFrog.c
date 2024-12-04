#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>

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

void draw_frog(WINDOW* window, FROG frog){
    wattron(window, COLOR_PAIR(FROG_COLOR));
    mvwaddch(window, frog.y, frog.x, frog.model);
    wattroff(window, COLOR_PAIR(FROG_COLOR));
}

void init_frog(WINDOW* window, FROG *frog){
    frog->x = PLAYWIN_WIDTH / 2;
    frog->y = PLAYWIN_HEIGHT - 1;
    frog->model = 'X';

    wmove(window, frog->y, frog->x);
    draw_frog(window, *frog);
}

void mainloop(WINDOW* window){
    FROG frog;
    init_frog(window, &frog);
    wrefresh(window);

    while(1){
        int c = getch();
        switch(c){
            mvwdelch(window, frog.y, frog.x); // frog disaapers of the screen if this isntruction is not in the cases
            case 'w':
                if(frog.y > 0) {
                    --frog.y;
                    draw_frog(window, frog);
                }
                break;
            case 's':
                if((frog.y + 1) < PLAYWIN_HEIGHT){
                    ++frog.y;
                    draw_frog(window, frog);
                }
                break;
            case 'a':
                if(frog.x > 0){
                    --frog.x;
                    draw_frog(window, frog);
                }
                break;
            case 'd':
                if((frog.x + 1) < PLAYWIN_WIDTH){
                    ++frog.x;
                    draw_frog(window, frog);
                }
                break;
            case 'q':
                return;
        }

        wrefresh(window);

        if(frog.y == 0){
            clear();
            wprintw(window, "wygrana");
            wrefresh(window);
            getch();
            return;
        }
    }
}

int main(){

    WINDOW* mainwin = start();

    WINDOW* playwin = subwin(mainwin, PLAYWIN_HEIGHT, PLAYWIN_WIDTH, 0, 0);
    wbkgd(playwin, COLOR_PAIR(2));

    mainloop(playwin);

    delwin(playwin);
    delwin(mainwin);
    endwin();
}