/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information. 
 * ---
 * Copyright (C) 2011, Lukas Weber <laochailan@web.de>
 */

#include "boss.h"
#include "global.h"
#include <string.h>
#include <stdio.h>

Boss *create_boss(char *name, char *ani, complex pos) {
	Boss *buf = malloc(sizeof(Boss));
	memset(buf, 0, sizeof(Boss));	
	
	buf->name = malloc(strlen(name) + 1);
	strcpy(buf->name, name);
	buf->pos = pos;
		
	buf->ani = get_ani(ani);
	
	return buf;
}

void draw_boss_text(Alignment align, float x, float y, const char *text) {
	glColor4f(0,0,0,1);
	draw_text(align, x+1, y+1, text, _fonts.standard);
	glColor4f(1,1,1,1);
	draw_text(align, x, y, text, _fonts.standard);
}

void spell_opening(Boss *b, int time) {
	float y = VIEWPORT_H - 15;
	if(time > 40 && time <= 100)
		y -= (VIEWPORT_H-50)/60.0*(time-40);
	if(time > 100) {
		y = 35;
	}
	
	draw_boss_text(AL_Right, VIEWPORT_W, y, b->current->name);
}

void draw_extraspell_bg(Boss *boss, int time) {
	// overlay for all extra spells
	// needs tweaking
	
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	glColor4f(1,0.1,0,0.7);
	fill_screen(sin(time) * 0.015, time / 50.0, 1, "stage3/wspellclouds");
	glColor4f(1,1,1,1);
	glBlendEquation(GL_MIN);
	fill_screen(cos(time) * 0.015, time / 70.0, 1, "stage4/kurumibg2");
	fill_screen(sin(time+2.1) * 0.015, time / 30.0, 1, "stage4/kurumibg2");
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
}

void draw_boss(Boss *boss) {
	draw_animation_p(creal(boss->pos), cimag(boss->pos) + 6*sin(global.frames/25.0), boss->anirow, boss->ani);
	draw_boss_text(AL_Left, 10, 20, boss->name);
	
	if(!boss->current)
		return;
	
	if(boss->current->type == AT_Spellcard || boss->current->type == AT_SurvivalSpell || boss->current->type == AT_ExtraSpell)
		spell_opening(boss, global.frames - boss->current->starttime);
	
	if(boss->current->type != AT_Move) {
		char buf[16];
		snprintf(buf, sizeof(buf),  "%.2f", (boss->current->timeout - global.frames + boss->current->starttime)/(float)FPS);
		draw_boss_text(AL_Center, VIEWPORT_W - 20, 10, buf);
		
		int nextspell, lastspell;
		for(nextspell = 0; nextspell < boss->acount - 1; nextspell++) {
			int t = boss->attacks[nextspell].type;
			if(boss->dmg < boss->attacks[nextspell].dmglimit && (t == AT_Spellcard || t == AT_ExtraSpell))
				break;
		}
				
		for(lastspell = nextspell; lastspell > 0; lastspell--) {
			int t = boss->attacks[lastspell].type;
			if(boss->dmg > boss->attacks[lastspell].dmglimit && (t == AT_Spellcard || t == AT_ExtraSpell))
				break;
		}
		
		int dmgoffset = boss->attacks[lastspell].dmglimit;
		int dmgspan = boss->attacks[nextspell].dmglimit - boss->attacks[lastspell].dmglimit;
				
		glPushMatrix();
		glTranslatef(VIEWPORT_W-50, 4, 0);
		glScalef((VIEWPORT_W-60)/(float)dmgspan,1,1);
		glTranslatef(dmgoffset,0,0);
		int i;
		
		glColor4f(0,0,0,0.65);
		
		glPushMatrix();
		glTranslatef(-(boss->attacks[nextspell].dmglimit+boss->dmg-2)*0.5, 2, 0);
		glScalef(boss->attacks[nextspell].dmglimit-boss->dmg+2, 4, 1);
		
		draw_quad();
		glPopMatrix();
		
		for(i = nextspell; i >= 0; i--) {
			if(boss->dmg > boss->attacks[i].dmglimit)
				continue;
			
			switch(boss->attacks[i].type) {
			case AT_Normal:
				glColor3f(1,1,1);
				break;
			case AT_Spellcard:
				glColor3f(1,0.65,0.65);
				break;
			case AT_SurvivalSpell:
				glColor3f(0.5,0.5,1);
			case AT_ExtraSpell:
				glColor3f(1.0, 0.3, 0.2);
			default:
				break; // never happens
			}		
			
			glPushMatrix();
			glTranslatef(-(boss->attacks[i].dmglimit+boss->dmg)*0.5, 1, 0);
			glScalef(boss->attacks[i].dmglimit-boss->dmg, 2, 1);
			
			draw_quad();
			glPopMatrix();
		}
				
		glPopMatrix();
		
		glColor4f(1,1,1,0.7);
		
		int x = 0;
		for(i = boss->acount-1; i > nextspell; i--)
			if(boss->attacks[i].type == AT_Spellcard)
				draw_texture(x += 22, 40, "star");
		
		glColor3f(1,1,1);
	}		
}

void boss_rule_extra(Boss *boss, float alpha) {
	int cnt = 10 * max(1, alpha);
	alpha = min(2, alpha);
	
	int i; for(i = 0; i < cnt; ++i) {
		float a = i*2*M_PI/cnt + global.frames / 100.0;
		complex dir = cexp(I*(a+global.frames/50.0));
		complex pos = boss->pos + dir * (100 + 50 * psin(alpha*global.frames/10.0+2*i)) * alpha;
		complex vel = dir * 3;
		
		create_particle2c("flare", pos, rgb(1, 0.5 + 0.2 * psin(a), 0.5), FadeAdd, timeout_linear, 15, vel);
		
		int d = 5;
		if(!(global.frames % d)) {
			// Try replacing this with stain or youmu_slice
			create_particle3c("boss_shadow", pos, rgb(1, 0.5 + 0.2 * psin(a), 0.5), GrowFadeAdd, timeout_linear, 30, vel * (1 - 2 * !(global.frames % (2*d))), 3);
		}
	}
}

void process_boss(Boss *boss) {
	if(boss->current) {
		int time = global.frames - boss->current->starttime;
		int extra = boss->current->type == AT_ExtraSpell;
		int fail = boss->current->failed;
		
		// TODO: mark uncaptured normal spells as failed too (for spell bonuses and player stats)
		
		boss->current->rule(boss, time);
		
		if(extra) {
			if(time < 0)
				boss_rule_extra(boss, 1+time/(float)ATTACK_START_DELAY_EXTRA);
			else {
				float o = min(0, -5 + time/30.0);
				float s = sin(time / 90.0 + M_PI*1.2);
				float q = (time <= 150? 1 - pow(time/250.0, 2) : min(1, time/60.0));
				
				boss_rule_extra(boss, max(1-time/300.0, 0.5 + 0.2 * s) * q);
				if(o)
					boss_rule_extra(boss, max(1-time/300.0, 0.5 + 0.2 * s) - o);
			}
		}
		
		if(boss->current->type != AT_Move && boss->dmg >= boss->current->dmglimit || extra && fail)
			time = boss->current->timeout + 1;
		
		if(time > boss->current->timeout) {
			if(extra && !fail)
				spawn_items(boss->pos, 0, 0, 0, 1);
			
			boss->current->rule(boss, EVENT_DEATH);
			boss->dmg = boss->current->dmglimit + 1;
			boss->current++;
			if(boss->current - boss->attacks < boss->acount)
				start_attack(boss, boss->current);
			else
				boss->current = NULL;
		}
	}
}

void boss_death(Boss **boss) {
	petal_explosion(35, (*boss)->pos);
	free_boss(*boss);
	*boss = NULL;
	
	Projectile *p;
	for(p = global.projs; p; p = p->next)
		if(p->type == FairyProj)
			p->type = DeadProj;
	
	delete_lasers();
}

void free_boss(Boss *boss) {
	del_ref(boss);
	
	free(boss->name);
	int i;
	for(i = 0; i < boss->acount; i++)
		free_attack(&boss->attacks[i]);
	if(boss->attacks)
		free(boss->attacks);
	if(boss->zoomcolor)
		free(boss->zoomcolor);
}

void free_attack(Attack *a) {
	free(a->name);
}

void start_attack(Boss *b, Attack *a) {	
	a->starttime = global.frames + (a->type == AT_ExtraSpell? ATTACK_START_DELAY_EXTRA : ATTACK_START_DELAY);
	a->rule(b, EVENT_BIRTH);
	if(a->type == AT_Spellcard || a->type == AT_SurvivalSpell || a->type == AT_ExtraSpell)
		play_sound("charge_generic");
	
	Projectile *p;
	for(p = global.projs; p; p = p->next)
		if(p->type == FairyProj)
			p->type = DeadProj;
		
	delete_lasers();
}

Attack *boss_add_attack(Boss *boss, AttackType type, char *name, float timeout, int hp, BossRule rule, BossRule draw_rule) {
	boss->attacks = realloc(boss->attacks, sizeof(Attack)*(++boss->acount));
	Attack *a = &boss->attacks[boss->acount-1];
	
	boss->current = &boss->attacks[0];
	
	a->type = type;
	a->name = malloc(strlen(name)+1);
	strcpy(a->name, name);
	a->timeout = timeout * FPS;
	
	int dmg = 0;
	if(boss->acount > 1)
		dmg = boss->attacks[boss->acount - 2].dmglimit;
	
	a->dmglimit = dmg + hp;
	a->rule = rule;
	a->draw_rule = draw_rule;
	
	a->starttime = global.frames;
	a->failed = False;
	
	return a;
}
