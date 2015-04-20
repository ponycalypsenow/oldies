#include "keyframer.h"

/*

  keyframer

*/

TRACKTYPE::TRACKTYPE(void){
	t=0;
}

TRACKTYPE::TRACKTYPE(char nt){
	t=nt;
}

ENDBEHAVIOR::ENDBEHAVIOR(void){
	behavior=0;
}

ENDBEHAVIOR::ENDBEHAVIOR(char b){
	behavior=b;
}

TRACK::TRACK(void){
	matrixid=parentid=targetid=0;
	rotatebehavior=scalebehavior=translatebehavior=ENDBEHAVIOR(EB_STOP);
	pivot.Identity();
	matrix=0;
	parent=target=0;
}

void TRACK::Do(float t){
	if(!head->next){
		rotate=head->rotate;
		scale=head->scale;
		translate=head->translate;
		return;
	}

	if(t>tail->frame){
		while(t>tail->frame) t-=tail->frame;
		for(Rewind();actual->next&&actual->next->frame<t;Next());
		t=(t-actual->frame)/(actual->next->frame-actual->frame);
		float t2=t*t;
		float t3=t*t2;
		float z=3*t2-t3-t3;
		float h1=1-z;
		float h2=z;
		float h3=t3-t2-t2+t;
		float h4=t3-t2;
		switch(rotatebehavior.behavior){
			case EB_RESET:
				rotate=head->rotate;
				break;

			case EB_STOP:
				rotate=tail->rotate;
				break;

			case EB_REPEAT:
				rotate=actual->rotate*h1+actual->next->rotate*h2+actual->rotatet0*h3+actual->next->rotatet1*h4;
				break;

			default:
				break;
		}

		switch(scalebehavior.behavior){
			case EB_RESET:
				scale=head->scale;
				break;

			case EB_STOP:
				scale=tail->scale;
				break;

			case EB_REPEAT:
				scale=actual->scale*h1+actual->next->scale*h2+actual->scalet0*h3+actual->next->scalet1*h4;
				break;

			default:
				break;
		}

		switch(translatebehavior.behavior){
			case EB_RESET:
				translate=head->translate;
				break;

			case EB_STOP:
				translate=tail->translate;
				break;

			case EB_REPEAT:
				translate=actual->translate*h1+actual->next->translate*h2+actual->translatet0*h3+actual->next->translatet1*h4;
				break;

			default:
				break;
		}

		return;
	}

	for(Rewind();actual->next&&actual->next->frame<t;Next());
	t=(t-actual->frame)/(actual->next->frame-actual->frame);
	float t2=t*t;
	float t3=t*t2;
	float z=3*t2-t3-t3;
	float h1=1-z;
	float h2=z;
	float h3=t3-t2-t2+t;
	float h4=t3-t2;
	rotate=actual->rotate*h1+actual->next->rotate*h2+actual->rotatet0*h3+actual->next->rotatet1*h4;
	scale=actual->scale*h1+actual->next->scale*h2+actual->scalet0*h3+actual->next->scalet1*h4;
	translate=actual->translate*h1+actual->next->translate*h2+actual->translatet0*h3+actual->next->translatet1*h4;
}

TEXTURE *TEXTURELIST::Search(char *n){
	TEXTURE *ret;
	TEXTURE *tmp=actual;
	for(Rewind();actual&&strcmp(actual->name,n);Next());
	ret=actual;
	actual=tmp;
	if(!ret||strcmp(ret->name,n)) return 0;
	else return ret;
}

MODEL *MODELLIST::Search(int i){
	MODEL *ret;
	MODEL *tmp=actual;
	for(Rewind();actual&&actual->id!=i;Next());
	ret=actual;
	actual=tmp;
	if(!ret||ret->id!=i) return 0;
	else return ret;
}

MODEL *MODELLIST::Search(char *n){
	MODEL *ret;
	MODEL *tmp=actual;
	for(Rewind();actual&&strcmp(actual->name,n);Next());
	ret=actual;
	actual=tmp;
	if(!ret||strcmp(ret->name,n)) return 0;
	else return ret;
}

void MODELLIST::Draw(void){
	for(Rewind();actual;Next())
		if(!(actual->state.s&RS_BLEND)) actual->Draw();

	for(Rewind();actual;Next())
		if(actual->state.s&RS_BLEND) actual->Draw();
}

LIGHT *LIGHTLIST::Search(int i){
	LIGHT *ret;
	LIGHT *tmp=actual;
	for(Rewind();actual&&actual->id!=i;Next());
	ret=actual;
	actual=tmp;
	if(!ret||ret->id!=i) return 0;
	else return ret;
}

CAMERA *CAMERALIST::Search(int i){
	CAMERA *ret;
	CAMERA *tmp=actual;
	for(Rewind();actual&&actual->id!=i;Next());
	ret=actual;
	actual=tmp;
	if(!ret||ret->id!=i) return 0;
	else return ret;
}

TRACK *TRACKLIST::Search(int i){
	TRACK *ret;
	TRACK *tmp=actual;
	for(Rewind();actual&&actual->matrixid!=i;Next());
	ret=actual;
	actual=tmp;
	if(!ret||ret->matrixid!=i) return 0;
	else return ret;
}

void TRACKLIST::Do(float t){
	for(Rewind();actual;Next()){
		actual->Do(t);
		actual->matrix->Identity();
		actual->matrix->Translate(actual->translate);
		actual->matrix->Scale(actual->scale);
		actual->matrix->Rotate(actual->rotate);
		*actual->matrix=*actual->matrix*actual->pivot;
	}

	for(Rewind();actual;Next())
		if(actual->parentid) *actual->matrix=*actual->parent->matrix**actual->matrix;

	for(Rewind();actual;Next()){
		if(actual->type.t==TT_CAMERA){
			VECTOR target;
			if(actual->targetid) target=VECTOR(0,0,0)**actual->target->matrix;
			else target=VECTOR(0,0,1)**actual->matrix;
			VECTOR position=VECTOR(0,0,0)**actual->matrix;
			actual->matrix->Target(target,position,actual->rotate.z);
		}
	}
}

void SCENE::Do(float t){
	tracklist.Do(t);
}

void SCENE::Draw(void){
	lightlist.Rewind();
	for(int i=0;i<8&&lightlist.actual;i++,lightlist.Next()){
		lightlist.actual->number=i;
		lightlist.actual->Set();
	}

	modellist.Draw();
}
