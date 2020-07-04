#ifndef _RL_STRUCTS_
#define _RL_STRUCTS_

typedef enum{melee, ranged, ammo, armor, food} item_type;

char coll_ob[] = "#|_-+";
char stop_ob[] = "A";

typedef struct _i{
    char *name;
    item_type type;
    void *data;
}item;

typedef struct _i_m{
    int damage;
}i_melee;

typedef struct _i_r{
    int base_damage;
    int range;
    char ammo_ID;
    int loaded_ammo;
}i_ranged;

typedef struct _i_am{
    int damage;
    char ID;
}i_ammo;

typedef struct _i_ar{
    int defense;
}i_armor;

typedef struct _i_f{
    int heal;
}i_food;

typedef struct _e{
    char *name;
	int y;
	int x;
	char symbol;
	int hp;
	int hp_max;
    item weapon;
    item armor;
    item *inventory;
    int inventory_size;
    int speed;
    int max_speed;
}entity;

typedef struct _dungeon{
    int height;
    int width;
    int y_o;
    int x_o;
    int door_x;
    int door_y;
}dungeon;

#endif //_RL_STRUCTS_
