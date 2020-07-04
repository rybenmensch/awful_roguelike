#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>
#include "structs.h"

#define QUIT 0
#define DEATH 1
#define SUCC 2

#define WALL '#'
#define FLOOR ' '

char **g_map;
int max_x;
int max_y;
entity enemy;
entity p;

void raus(void);
void splash(char msg[]);
char **setup_map();
void drawmap();
void place_enemy();
void create_map();
void msg_center(char msg[]);
int game();
void inventory(entity *p);
void help();
void cave(int y, int x);
void gen_dungeon(dungeon *d, int y_r, int x_r);
void connect_dungeon(dungeon *d1, dungeon *d2);
void erode(int c0, int c1, int c2, int c3, int c4);
void eatshit();
void check_bounds(entity *p, int y, int x);
void run(entity *p, int speed, int y, int x);
int is_in_bounds(int y, int x);
int is_coll(int y, int x);
int is_inv_cmd(char c);
int is_stop(int y, int x);
int is_move(char c);
int is_wall(int y, int x);
void beweg(entity *p, char c);
void intro(char msg_splash[], char msg_c[]);
void clr();

int main(){
	time_t t;
    srand((unsigned) time(&t));
	atexit(raus);
	initscr();
	noecho();
	curs_set(FALSE);
	g_map = setup_map();
	create_map();
	//intro("eatshit", "MOTHERFUCKER ARENA");
	drawmap();
	int ret = game();
	clear();
	switch(ret){
		case DEATH:	msg_center("you died.");
					break;
		case QUIT:	msg_center("merci fuer gar nuet - thanks for \"playing\"!");
					break;
		case SUCC: 	msg_center("you fuckin did it!");
					break;
		default:	msg_center("??????");
	}
	sleep(1);
}

void raus(void){
	if(g_map){
		for(int y=0;y<max_y;y++){
			free(g_map[y]);
		}
		free(g_map);
	}
	endwin();
	printf("I like the Beach Boys!\n");
}

void splash(char msg[]){
	int max_y = 0, max_x = 0;
	int y = 0, x = 0;

	int targettime = 2; //in seconds
	int refresh = 2000;
	int reps = (targettime * 1000000) / refresh;
	int c=0;

	while(1){
		getmaxyx(stdscr, max_y, max_x);
		int ry, rx;
		ry = rand()%max_y;
		rx = rand()%max_x;
		mvprintw(ry, rx, msg);
		usleep(refresh);
		refresh();
		if(c++ == reps){
			break;
		}
	}

	clear();
}

char **setup_map(){
	getmaxyx(stdscr, max_y, max_x);
	char **map = malloc(max_y*sizeof(char *));
	for(int y=0;y<max_y;y++){
		map[y] = malloc(max_x);
	}
	return map;
}

void place_enemy(){
	//monte carlo the enemy into the field
	int y=0, x=0;
	do{
		y = rand()%(max_y-1)+1;
		x = rand()%(max_x-1)+1;
	}while(g_map[y][x] == WALL && y != p.y && x != p.x);

	do{
		y = rand()%(max_y-1)+1;
		x = rand()%(max_x-1)+1;
	}while(g_map[y][x] == WALL && y != p.y && x != p.x);

	enemy.y = y;
	enemy.x = x;

	g_map[enemy.y][enemy.x] = '*';
}

void create_map(){
	for(int x=0;x<max_x;x++){
		g_map[0][x] = '_';
		g_map[max_y-1][x] = '-';
	}

	for(int y=1;y<max_y-1;y++){
		g_map[y][0] = '|';
		g_map[y][max_x-1] = '|';
	}

	for(int x=1;x<max_x-1;x++){
		for(int y=1;y<max_y-1;y++){
			//g_map[y][x] = '.';
		}
	}
	place_enemy();
}

void drawmap(){
	clear();
	for(int y=0;y<max_y;y++){
		for(int x=0;x<max_x;x++){
			mvprintw(y, x, "%c", g_map[y][x]);
		}
	}
	refresh();
}

void msg_center(char msg[]){
	int max_y, max_x;
	getmaxyx(stdscr, max_y, max_x);
	int offset = strlen(msg)/2;
	mvprintw(max_y/2, max_x/2-offset, msg);
	refresh();
}

void intro(char msg_splash[], char msg_c[]){
	splash(msg_splash);
	drawmap();
	sleep(1);
	msg_center(msg_c);
	sleep(1);
}

int game(){
	p.name = "player";
	p.symbol = '@';
	p.y = max_y/2;
	p.x = max_x/2;
	p.hp_max = rand()%20 + 5;
	p.speed = 1;
	p.max_speed = 10;
	p.inventory_size = 1;
	p.hp = p.hp_max;

	//cave(max_y, max_x);
	//gen_dungeon();

	dungeon d1, d2;
	gen_dungeon(&d1, 10, 10);
	gen_dungeon(&d2, p.y, p.x);

	char c = 0;
	char cmd[10];
	int speed = 1;
	int im=0;

	//MAIN LOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOP
	while(1){
		drawmap();
		strcpy(cmd, "");
		mvprintw(p.y, p.x,"%c", p.symbol);
		mvprintw(0, 0, cmd);
		mvprintw(max_y-1, 0, "Hp: %d/%d", p.hp, p.hp_max);
		mvprintw(max_y-1, 20, "max speed: %d", p.max_speed);
		refresh();

		c = getchar();

		if(is_move(c)){
				beweg(&p, c);
		}else{
			switch(c){
				case 'x':	mvprintw(0, 0, "y to quit, any other key to continue");
						  	refresh();
						 	c = getchar();
						  	if(c=='y'){
								goto quit;
						  	}
						  	break;
				case 'i':	inventory(&p);
							break;
				case '?': 	help();
							break;
				case 'f':	eatshit();
							break;
				case 'm': 	//cave(max_y, max_x);
							clr();
							gen_dungeon(&d1, 10, 20);
							gen_dungeon(&d2, p.y, p.x);
							connect_dungeon(&d1, &d2);
							break;
				case 'n':	erode(0, 25, 50, 75, 100);
							break;
				case 'b':	clr();
							break;
			}
		}

		check_bounds(&p, max_y, max_x);

		if(p.x == enemy.x && p.y == enemy.y){
			msg_center("DAMN BOI");
			sleep(1);
			if(rand()%2){
				goto success;
			}else{
				goto death;
			}

		}
	}
	//MAIN LOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOOP

quit:
	return QUIT;
death:
	return DEATH;
success:
	return SUCC;
}

void clr(){
	for(int y=1;y<max_y-1;y++){
		for(int x=1;x<max_x-1;x++){
			g_map[y][x] = ' ';
		}
	}
}

void gen_dungeon(dungeon *d, int y_r, int x_r){
	int x_o, y_o, width, height;
	width = rand()%15 + 5;
	height = rand()%5 + 3;
	x_o = x_r-width/2+1;
	y_o = y_r-height/2-1;


	//draw ONE door
	int door_x, door_y;
	int x_or_y = rand()%2;

	if(1){
		door_y = (rand()%2)?y_o:y_o+height;	//choose either upper or lower wall
		door_x = rand()%(width-2) + x_o + 1;	//choose random point along x axis
	}else{
		door_y = rand()%(height-1) + y_o + 1;	//choose random point along y axis
		door_x = (rand()%2)?x_o:x_o+width;	//choose right or left wall

	}

	d->y_o = y_o;
	d->x_o = x_o;
	d->height = height;
	d->width = width;
	d->door_y = door_y;
	d->door_x = door_x;

	//draw walls
	for(int x=x_o+1;x<x_o+width;x++){
		g_map[y_o][x] = '-';
		g_map[y_o+height][x] = '-';
	}

	for(int y=y_o+1;y<y_o+height;y++){
		g_map[y][x_o] = '|';
		g_map[y][x_o+width] = '|';
	}

	//draw corners
	g_map[y_o][x_o] = '+';
	g_map[y_o+height][x_o] = '+';
	g_map[y_o][x_o+width] = '+';
	g_map[y_o+height][x_o+width] = '+';
	g_map[door_y][door_x] = 'A';


}

void connect_dungeon(dungeon *d1, dungeon *d2){
	int diff_y, diff_x, larger_y, larger_x, lesser_y, lesser_x;

	if(d1->door_x>d2->door_x){
		diff_x = d1->door_x - d2->door_x;
		lesser_x = d2->door_x;
		larger_x = d1->door_x;
	}else{
		diff_x = d2->door_x - d1->door_x;
		lesser_x = d1->door_x;
		larger_x = d2->door_x;
	}

	if(d1->door_y>d2->door_y){
		diff_x = d1->door_y - d2->door_y;
		lesser_y = d2->door_y;
		larger_y = d1->door_y;
	}else{
		diff_x = d2->door_y - d1->door_y;
		lesser_y = d1->door_y;
		larger_y = d2->door_y;
	}
}

void cave(int y, int x){
	//make cave not draw into the borders

	//initialize all map cells to walls
	for(int i=0;i<y;i++){
		for(int j=0;j<x;j++){
			g_map[i][j] = WALL;
		}
	}

	//pick a map cell as the starting point
	entity e = {
		.x = rand()%y,
		.y = rand()%x
	};
	e.x = p.x;
	e.y = p.y;

	//turn the selected map cell into floor.
	g_map[e.y][e.x] = FLOOR;
	int percent = rand()%40 + 20; //% of map to be turned into floor
	int target = (x * y) * ((float)percent/100);
	int floor_c = 0;

	//while insuficient cells have been turned into floor
	while(floor_c!=target){
		//take one step in a random direction
		int dir = rand()%4;
		switch(dir){
			case 0:	e.y--; //up
					break;
			case 1:	e.y++; //down
					break;
			case 2:	e.x--; //left
					break;
			case 3:	e.x++; //right
					break;
		}
		check_bounds(&e, y, x);

		//if the new map cell is wall
		if(g_map[e.y][e.x] == WALL){
			//turn the new map cell into floor..
			g_map[e.y][e.x] = FLOOR;
			//and increment the count of floor tiles
			floor_c++;
		}
	}
	//is it possible maybe to insert sub-caves?
	//into big open spaces
	erode(0, 75, 100, 100, 100);

	//create_map();
}

void erode(int c0, int c1, int c2, int c3, int c4){
	int chance = 0;
	int probs[5] = {c0, c1, c2, c3, c4};

	for(int y=2;y<max_y-2;y++){
		for(int x=2;x<max_x-2;x++){
			if(g_map[y][x] != FLOOR){
				int score = is_wall(y-1, x) + is_wall(y+1, x)+
							is_wall(y, x-1) + is_wall(y, x+1);

				chance = probs[score];
				int roll = rand()%100;
				g_map[y][x] = (roll<chance)?WALL:FLOOR;
			}
		}
	}
}

void chunky(){
	//search for big ass chunks and plant something in the middle
	int w_c_y[max_y], w_c_x[max_x];
	memset(w_c_y, 0, max_x *sizeof(int));
	memset(w_c_x, 0, max_y *sizeof(int));

	for(int y=1;y<max_y-1;y++){
		for(int x=1;x<max_x-1;x++){
			if(g_map[y][x]==WALL){
				w_c_y[y]++;
			}else{
				break;
			}
		}
	}

	for(int x=1;x<max_x-1;x++){
		for(int y=1;y<max_y-1;y++){
			if(g_map[y][x]==WALL){
				w_c_x[x]++;
			}else{
				break;
			}
		}
	}


}

//SCREENS

void help(){
	WINDOW *help_win;
	help_win = newwin(max_y, max_x, 0, 0);
	mvprintw(0, 0, "~~~~HELP~~~~");
	mvprintw(1, 0, "Press any key to exit.");
	wrefresh(help_win);
	getch();
	delwin(help_win);
}

void eatshit(){
	WINDOW *help_win;
	help_win = newwin(max_y, max_x, 0, 0);
	char txt[5][50];
	strcpy(txt[0], "    _________  ______   _____ __  ____________");
	strcpy(txt[1], "   / ____/   |/_  __/  / ___// / / /  _/_  __/");
	strcpy(txt[2], "  / __/ / /| | / /     \\__ \\/ /_/ // /  / /   ");
	strcpy(txt[3], " / /___/ ___ |/ /     ___/ / __  // /  / /    ");
	strcpy(txt[4], "/_____/_/  |_/_/     /____/_/ /_/___/ /_/     ");
	clear();
	int x_off = (max_x-strlen(txt[0]))/2;
	int y_off = (max_y/2)-3;
	for(int i=0;i<5;i++){
		mvprintw(i+y_off, x_off, "%s", txt[i]);
	}

	wrefresh(help_win);
	getch();
	delwin(help_win);
}

int is_inv_cmd(char c){
	char invset[] = {"g"};
	int invs_len = strlen(invset);
	for(int i=0;i<invs_len;i++){
		if(c == invset[i]){
			return 1;
		}
	}
	return 0;
}

void inventory(entity *p){
	WINDOW *inv_win;
	inv_win = newwin(max_y, max_x, 0, 0);
	mvprintw(0, 0, "~~~~INVENTORY~~~~");
	if(p->inventory_size == 0){
		mvprintw(1, 0, "your inventory is empty.");
		mvprintw(2, 0, "Press any key to exit.");
		wrefresh(inv_win);

		getch();
		goto i_exit;
	}

	mvprintw(1, 0, "Command list.");
	mvprintw(2, 0, "Press x to exit.");
	//print the actual inventory

	wrefresh(inv_win);

	//loop through commands

	char c;
	while(1){
		c = getch();
		if(is_inv_cmd(c)){
			//do something
			eatshit();
			goto i_exit;
		}else{
			switch(c){
				case 'x': goto i_exit;
			}
		}
	}

i_exit:
	wrefresh(inv_win);
	delwin(inv_win);
	return;
}

//MOVEMENT

int is_move(char c){
	char moveset[] = {"wasdWASDqeychjklHJKL"};
	int mvs_len = strlen(moveset);
	for(int i=0;i<mvs_len;i++){
		if(c == moveset[i]){
			return 1;
		}
	}

	return 0;
}

int is_coll(int y, int x){
	if(!is_in_bounds(y, x)){
		return 1;
	}else{
		for(int i=0;i<strlen(coll_ob);i++){
			if(g_map[y][x] == coll_ob[i]){
				return 1;
			}
		}
		return 0;
	}
}

int is_stop(int y, int x){
	if(!is_in_bounds(y, x)){
		return 1;
	}else{
		for(int i=0;i<strlen(stop_ob);i++){
			if(g_map[y][x] == stop_ob[i]){
				return 1;
			}
		}
		return 0;
	}
}

int is_wall(int y, int x){
	if(!is_in_bounds(y, x)){
		return 1;
	}else{
		return (g_map[y][x] == WALL);
	}
}

int is_in_bounds(int y, int x){
	return (y<max_y && y>=0 && x<max_x && x>=0);
}

int is_in_bounds2(int y, int x, int min_y, int min_x, int mx_y, int mx_x){
	return (y<mx_y && y>=min_y && x<mx_x && x>=min_x);

	/*
	typedef struct rect{
		int o_y;
		int o_x;
		int e_y;
		int e_x;
		//or:
		int offset[2]; //y, x
		int height;
		int width;
	}

	*/
}

void run(entity *p, int speed, int y, int x){
	for(int i=0;i<speed;i++){
		if(is_coll(p->y + y, p->x + x)){
			break;
		}else if(is_stop(p->y + y, p->x + x)){
			p->y += y;
			p->x += x;
			break;
		}else{
			p->y += y;
			p->x += x;
		}
	}
}

void beweg(entity *p, char c){
	//fall trough not working
	switch(c){
		case 'k':
		case 'w':	run(p, p->speed, -1 ,0);
					break;
		case 'j':
		case 's':	run(p, p->speed, 1, 0);
					break;
		case 'h':
		case 'a':	run(p, p->speed, 0, -1);
					break;
		case 'l':
		case 'd':	run(p, p->speed, 0, 1);
					break;
		case 'K':
		case 'W':	run(p, p->max_speed/2, -1, 0);
					break;
		case 'J':
		case 'S':	run(p, p->max_speed/2, 1, 0);
					break;
		case 'H':
		case 'A':	run(p, p->max_speed, 0, -1);
					break;
		case 'L':
		case 'D':	run(p, p->max_speed, 0, 1);
					break;
		case 'Q':	//fall through
		case 'q':	run(p, p->speed, -1 ,-1);
					break;
		case 'E':	//fall through
		case 'e':	run(p, p->speed, -1, 1);
					break;
		case 'Y':	//fall through
		case 'y':	run(p, p->speed, 1, -1);
					break;
		case 'C':	//fall through
		case 'c':	run(p, p->speed, 1, 1);
					break;
	}
}

void check_bounds(entity *p, int y, int x){
	if(p->y >= y-1){
		p->y = y-2;
	}else if(p->y <= 0){
		p->y = 1;
	}else if(p->x >= x-1){
		p->x = x-2;
	}else if(p->x <= 0){
		p->x = 1;
	}
}
