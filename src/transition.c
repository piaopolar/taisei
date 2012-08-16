/*
 * This software is licensed under the terms of the MIT-License
 * See COPYING for further information. 
 * ---
 * Copyright (C) 2011, Lukas Weber <laochailan@web.de>
 */

#include "transition.h"
#include "menu/ingamemenu.h"
#include "global.h"

static Transition transition;

float trans_fade(Transition *t) {
	if(t->frames <= t->dur1)
		return t->frames/(float)t->dur1;
	else
		return (t->dur1+t->dur2-t->frames)/(float)t->dur2;
}

void TransFadeBlack(Transition *t) {
	fade_out(trans_fade(t));
}

void TransFadeWhite(Transition *t) {
	colorfill(1,1,1,trans_fade(t));
}

void set_transition(TransitionRule rule, int dur1, int dur2) {
	if(!rule)
		return;
		
	memset(&transition, 0, sizeof(Transition));
	transition.rule = rule;
	transition.dur1 = dur1;
	transition.dur2 = dur2;
}

void draw_transition(void) {
	if(!transition.rule)
		return;
	
	transition.rule(&transition);
	
	transition.frames++;
	
	if(transition.frames > transition.dur1 + transition.dur2)
		memset(&transition, 0, sizeof(Transition));
}

int transition_isset(void) {
	return !!transition.rule;
}
