#include "stdafx.h"

/*

  error

*/

void error(char *s){
	FILE *f=fopen("error.log","at");
	fprintf(f,"%s - error. ask fungi. he will help you\n",s);
	fclose(f);
	ExitProcess(0);
}
