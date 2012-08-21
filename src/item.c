/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information. 
 * ---
 * Copyright (C) 2011, Lukas Weber <laochailan@web.de>, Juergen Kieslich
 */

#include "item.h"
#include "global.h"
#include "list.h"

Item *create_item(complex pos, complex v, Type type) {
	Item *i = create_element((void **)&global.items, sizeof(Item));
	i->pos = pos;
	i->pos0 = pos;
	i->v = v;
	i->birthtime = global.frames;
	i->auto_collect = 0;
	i->type = type;
	
	return i;
}


void delete_item(Item *item) {
	delete_element((void **)&global.items, item);
}

void draw_items(void) {
	Item *p;
	Texture *tex = NULL;
	for(p = global.items; p; p = p->next) {
		switch(p->type){
			case Power:
				if(global.plr.power < PLR_MAXPOWER) {
					tex = get_tex("items/power");
					break;
				}
				
				p->type = Point;
			case Point:
				tex = get_tex("items/point");
				break;
			case Life:
				tex = get_tex("items/life");
				break;
			case Bomb:
				tex = get_tex("items/bomb");
				break;
			case BPoint:
				tex = get_tex("items/bullet_point");
				break;
			default:
				break;
		}
		draw_texture_p(creal(p->pos), cimag(p->pos), tex);
	}
}

void delete_items(void) {
	delete_all_elements((void **)&global.items, delete_element);
}

void move_item(Item *i) {
	int t = global.frames - i->birthtime;
	complex lim = 0 + 2I;
	
	if(i->auto_collect)	
		i->pos -= (7+i->auto_collect)*cexp(I*carg(i->pos - global.plr.pos));
	else
		i->pos = i->pos0 + log(t/5.0 + 1)*5*(i->v + lim) + lim*t;
}

void process_items(void) {
	Item *item = global.items, *del = NULL;
    int v;
	
	float r = 30;
	if(global.plr.focus > 0);
		r *= 2;
	
	while(item != NULL) {
		if(cimag(global.plr.pos) < POINT_OF_COLLECT || cabs(global.plr.pos - item->pos) < r
		|| global.frames - global.plr.recovery < 0)
			item->auto_collect = 1;
		
		move_item(item);
		
		v = collision_item(item);
		if(v == 1) {
			switch(item->type) {
			case Power:
				player_set_power(&global.plr, global.plr.power + POWER_VALUE);
				break;
			case Point:
				global.points += 100;
				break;
			case BPoint:
				global.points += 1;
				break;
			case Life:
				global.plr.lifes++;
				break;
			case Bomb:
				global.plr.bombs++;
				break;
			}
			play_sound("item_generic");
		}
		
		if(v == 1 || creal(item->pos) < -9 || creal(item->pos) > VIEWPORT_W + 9
			|| cimag(item->pos) > VIEWPORT_H + 8 ) {
			del = item;
			item = item->next;
			delete_item(del);
		} else {
			item = item->next;
		}
	}
}

int collision_item(Item *i) {
	if(cabs(global.plr.pos - i->pos) < 10)
		return 1;
	
	return 0;
}

void spawn_item(complex pos, Type type) {
	tsrand_fill(2);
	create_item(pos, 5*cexp(I*tsrand_a(0)/afrand(1)*M_PI*2), type);
}

void spawn_items(complex pos, int point, int power, int bomb, int life) {
	int i;
	for(i = 0; i < point; i++)
		spawn_item(pos, Point);
	
	for(i = 0; i < power; i++)
		spawn_item(pos, Power);
	
	for(i = 0; i < bomb; i++)
		spawn_item(pos, Bomb);
	
	for(i = 0; i < life; i++)
		spawn_item(pos, Life);
}
