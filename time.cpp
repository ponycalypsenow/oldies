#include "time.h"

/*

  time

*/

static float lf;
static float ls;

TIME::TIME(void){
	frame=scene=0;
	lf=ls=(float)GetTickCount()/1000.0f;
}

void TIME::Get(void){
	float actual=(float)GetTickCount()/1000.0f;
	frame=actual-lf;
	scene=actual-ls;
	lf=actual;
}
