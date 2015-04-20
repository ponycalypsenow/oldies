#include "lws.h"

/*

  lws loader

*/

struct LINE{
	char buffer[96];
};

static FILE *f;
static LWS *lws;
static int id;

static LINE lread(void){
	LINE ret;
	fgets(ret.buffer,sizeof(ret.buffer),f);
	return ret;
}

static void lskip(void){
	LINE tmp;
	fgets(tmp.buffer,sizeof(tmp.buffer),f);
}

static char *nskip(char *s){
	char *o=s;
	while(*o!='.'){
		if(*o=='\\'||*o==':'||*o=='/') s=o+1;
		o++;
	}
	o[4]='\0';
	return s;
}

static char *sskip(char *s){
	while(*s!=' ') s++;
	return ++s;
}

static void loadobjectlayer(char *s){
	s=nskip(sskip(sskip(s)));
	lws->modellist.Add(new LWO(s));
	id++;
	lws->modellist.tail->id=id;
}

static void addnullobject(char *s){
	lws->modellist.Add(new MODEL(s));
	id++;
	lws->modellist.tail->id=id;
}

static void showobject(char *s){
	int t=atoi(sskip(s));
	switch(t){
		case 6:
			lws->modellist.tail->type=MODELTYPE(MT_POLYGON);
			lws->modellist.tail->state.s|=RS_CULL|RS_SMOOTH|RS_RENDER;
			break;

		case 5:
			lws->modellist.tail->type=MODELTYPE(MT_POLYGON);
			lws->modellist.tail->state.s|=RS_CULL|RS_RENDER;
			break;

		case 4:
			lws->modellist.tail->type=MODELTYPE(MT_LINE);
			lws->modellist.tail->state.s|=RS_CULL|RS_RENDER;
			break;

		case 3:
			lws->modellist.tail->type=MODELTYPE(MT_LINE);
			lws->modellist.tail->state.s|=RS_RENDER;
			break;

		case 2:
			lws->modellist.tail->type=MODELTYPE(MT_PARTICLE);
			lws->modellist.tail->state.s|=RS_RENDER;
			break;

		default:
			break;
	}
}

static void key(float& f,float& v,float& t0,float& t1){
	LINE l=lread();
	char *s=l.buffer;
	s=sskip(sskip(sskip(s)));
	v=(float)atof(s);
	s=sskip(s);
	f=(float)atof(s);
	s=sskip(sskip(sskip(sskip(sskip(s)))));
	t0=(float)atof(s);
	s=sskip(s);
	t1=(float)atof(s);
}

static void channel9(TRACK& t,int k){
	lskip(),lskip();
	t.Rewind();
	for(int i=0;i<k;i++,t.Next()) key(t.actual->frame,t.actual->translate.x,t.actual->translatet0.x,t.actual->translatet1.x);
	LINE l=lread();
	t.translatebehavior.behavior=atoi(sskip(sskip(sskip(sskip(l.buffer)))))+1;
	lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()) key(t.actual->frame,t.actual->translate.y,t.actual->translatet0.y,t.actual->translatet1.y);
	lskip(),lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()) key(t.actual->frame,t.actual->translate.z,t.actual->translatet0.z,t.actual->translatet1.z);
	lskip(),lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()){
		key(t.actual->frame,t.actual->rotate.y,t.actual->rotatet0.y,t.actual->rotatet1.y);
		t.actual->rotate.y=-t.actual->rotate.y;
		t.actual->rotatet0.y=-t.actual->rotatet0.y;
		t.actual->rotatet1.y=-t.actual->rotatet1.y;
	}

	l=lread();
	t.rotatebehavior.behavior=atoi(sskip(sskip(sskip(sskip(l.buffer)))))+1;
	lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()){
		key(t.actual->frame,t.actual->rotate.x,t.actual->rotatet0.x,t.actual->rotatet1.x);
		t.actual->rotate.x=-t.actual->rotate.x;
		t.actual->rotatet0.x=-t.actual->rotatet0.x;
		t.actual->rotatet1.x=-t.actual->rotatet1.x;
	}

	lskip(),lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()){
		key(t.actual->frame,t.actual->rotate.z,t.actual->rotatet0.z,t.actual->rotatet1.z);
		t.actual->rotate.z=-t.actual->rotate.z;
		t.actual->rotatet0.z=-t.actual->rotatet0.z;
		t.actual->rotatet1.z=-t.actual->rotatet1.z;
	}

	lskip(),lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()) key(t.actual->frame,t.actual->scale.x,t.actual->scalet0.x,t.actual->scalet1.x);
	l=lread();
	t.scalebehavior.behavior=atoi(sskip(sskip(sskip(sskip(l.buffer)))))+1;
	lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()) key(t.actual->frame,t.actual->scale.y,t.actual->scalet0.y,t.actual->scalet1.y);
	lskip(),lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()) key(t.actual->frame,t.actual->scale.z,t.actual->scalet0.z,t.actual->scalet1.z);
}

static void objectmotion(void){
	lws->tracklist.Add(new TRACK);
	lws->tracklist.tail->matrixid=id;
	lskip(),lskip();
	int p=ftell(f);
	lskip();
	LINE l=lread();
	int k=atoi(sskip(sskip(l.buffer)));
	for(int i=0;i<k;i++) lws->tracklist.tail->Add(new KEY);
	fseek(f,p,SEEK_SET);
	channel9(*lws->tracklist.tail,k);
}

static void addlight(void){
	lws->lightlist.Add(new LIGHT);
	id++;
	lws->lightlist.tail->id=id;	
}

static void lightmotion(void){
	lws->tracklist.Add(new TRACK);
	lws->tracklist.tail->matrixid=id;
	lskip(),lskip();
	int p=ftell(f);
	lskip();
	LINE l=lread();
	int k=atoi(sskip(sskip(l.buffer)));
	for(int i=0;i<k;i++) lws->tracklist.tail->Add(new KEY);
	fseek(f,p,SEEK_SET);
	channel9(*lws->tracklist.tail,k);
}

static void lighttype(char *s){
	s=sskip(s);
	int t=atoi(s);
	switch(t){
		case 0:
			lws->lightlist.tail->type=LIGHTTYPE(LT_DISTANT);
			break;

		case 1:
			lws->lightlist.tail->type=LIGHTTYPE(LT_POINT);
			break;

		case 2:
			lws->lightlist.tail->type=LIGHTTYPE(LT_SPOT);
			break;

		default:
			break;
	}
}

static void lightcolor(char *s){
	s=sskip(s);
	lws->lightlist.tail->color.r=(float)atof(s);
	s=sskip(s);
	lws->lightlist.tail->color.g=(float)atof(s);
	s=sskip(s);
	lws->lightlist.tail->color.b=(float)atof(s);
	lws->lightlist.tail->color.a=1;
}

static void lightintensity(char *s){
	float i=(float)atof(sskip(s));
	lws->lightlist.tail->color.r*=i;
	lws->lightlist.tail->color.g*=i;
	lws->lightlist.tail->color.b*=i;
	lws->lightlist.tail->color.a*=i;
}

static void affectcaustics(char *s){
	lws->lightlist.tail->a=(float)atof(sskip(s));
}

static void addcamera(void){
	lws->cameralist.Add(new CAMERA);
	id++;
	lws->cameralist.tail->id=id;
}

static void channel6(TRACK& t,int k){
	lskip(),lskip();
	t.Rewind();
	for(int i=0;i<k;i++,t.Next()) key(t.actual->frame,t.actual->translate.x,t.actual->translatet0.x,t.actual->translatet1.x);
	LINE l=lread();
	t.translatebehavior.behavior=atoi(sskip(sskip(sskip(sskip(l.buffer)))))+1;
	lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()) key(t.actual->frame,t.actual->translate.y,t.actual->translatet0.y,t.actual->translatet1.y);
	lskip(),lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()) key(t.actual->frame,t.actual->translate.z,t.actual->translatet0.z,t.actual->translatet1.z);
	lskip(),lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()){
		key(t.actual->frame,t.actual->rotate.y,t.actual->rotatet0.y,t.actual->rotatet1.y);
		t.actual->rotate.y=-t.actual->rotate.y;
		t.actual->rotatet0.y=-t.actual->rotatet0.y;
		t.actual->rotatet1.y=-t.actual->rotatet1.y;
	}

	l=lread();
	t.rotatebehavior.behavior=atoi(sskip(sskip(sskip(sskip(l.buffer)))))+1;
	lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()){
		key(t.actual->frame,t.actual->rotate.x,t.actual->rotatet0.x,t.actual->rotatet1.x);
		t.actual->rotate.x=-t.actual->rotate.x;
		t.actual->rotatet0.x=-t.actual->rotatet0.x;
		t.actual->rotatet1.x=-t.actual->rotatet1.x;
	}

	lskip(),lskip(),lskip(),lskip(),lskip();
	for(i=0,t.Rewind();i<k;i++,t.Next()){
		key(t.actual->frame,t.actual->rotate.z,t.actual->rotatet0.z,t.actual->rotatet1.z);
		t.actual->rotate.z=-t.actual->rotate.z;
		t.actual->rotatet0.z=-t.actual->rotatet0.z;
		t.actual->rotatet1.z=-t.actual->rotatet1.z;
	}

	t.scalebehavior=t.translatebehavior;
	for(i=0,t.Rewind();i<k;i++,t.Next()) t.actual->scale=VECTOR(1,1,1);
}

static void cameramotion(void){
	lws->tracklist.Add(new TRACK);
	lws->tracklist.tail->matrixid=id;
	lskip(),lskip();
	int p=ftell(f);
	lskip();
	LINE l=lread();
	int k=atoi(sskip(sskip(l.buffer)));
	for(int i=0;i<k;i++) lws->tracklist.tail->Add(new KEY);
	fseek(f,p,SEEK_SET);
	channel6(*lws->tracklist.tail,k);
}

static void parentitem(char *s){
	s=sskip(s);
	s++;
	lws->tracklist.tail->parentid=atoi(s)+1;
}

static void pivotposition(char *s){
	s=sskip(s);
	float x=(float)-atof(s);
	s=sskip(s);
	float y=(float)-atof(s);
	s=sskip(s);
	float z=(float)-atof(s);
	lws->tracklist.tail->pivot.Translate(VECTOR(x,y,z));
}

static void pivotrotation(char *s){
	s=sskip(s);
	float y=angle2rad((float)atof(s));
	s=sskip(s);
	float x=angle2rad((float)atof(s));
	s=sskip(s);
	float z=angle2rad((float)atof(s));
	lws->tracklist.tail->pivot.Rotate(VECTOR(x,y,z));
}

static void targetobject(char *s){
	lws->tracklist.tail->targetid=atoi(sskip(s));
}

static void zoomfactor(char *s){
	float zf=(float)atof(sskip(s));
	lws->cameralist.tail->fovh=2.0f*atanf(1.333f/zf);
	lws->cameralist.tail->fovv=lws->cameralist.tail->fovh*0.777f;
}

LWS::LWS(char *n){
	f=0;
	f=fopen(n,"rt");
	if(!f) error("file doesn't exist");
	lws=this,id=0;
	fseek(f,0,SEEK_END);
	int size=ftell(f);
	rewind(f);
	while(ftell(f)<size){
		LINE l=lread();
		if(!strncmp(l.buffer,"LoadObjectLayer",15)){
			loadobjectlayer(l.buffer);
			continue;
		}

		if(!strncmp(l.buffer,"AddNullObject",13)){
			addnullobject(l.buffer);
			continue;
		}

		if(!strncmp(l.buffer,"ShowObject",10)){
			showobject(l.buffer);
			continue;
		}

		if(!strncmp(l.buffer,"ObjectMotion",12)){
			objectmotion();
			continue;
		}

		if(!strncmp(l.buffer,"AddLight",7)){
			addlight();
			continue;
		}

		if(!strncmp(l.buffer,"LightMotion",11)){
			lightmotion();
			continue;
		}

		if(!strncmp(l.buffer,"LightType",9)){
			lighttype(l.buffer);
			continue;
		}

		if(!strncmp(l.buffer,"LightColor",10)){
			lightcolor(l.buffer);
			continue;
		}

		if(!strncmp(l.buffer,"LightIntensity",14)){
			lightintensity(l.buffer);
			continue;
		}

		if(!strncmp(l.buffer,"AffectCaustics",14)){
			affectcaustics(l.buffer);
			continue;
		}

		if(!strncmp(l.buffer,"AddCamera",9)){
			addcamera();
			continue;
		}

		if(!strncmp(l.buffer,"CameraMotion",11)){
			cameramotion();
			continue;
		}

		if(!strncmp(l.buffer,"ParentItem",10)){
			parentitem(l.buffer);
			continue;
		}

		if(!strncmp(l.buffer,"PivotPosition",13)){
			pivotposition(l.buffer);
			continue;
		}

		if(!strncmp(l.buffer,"PivotRotation",13)){
			pivotrotation(l.buffer);
			continue;
		}

		if(!strncmp(l.buffer,"TargetObject",12)){
			targetobject(l.buffer);
			continue;
		}

		if(!strncmp(l.buffer,"ZoomFactor",10)){
			zoomfactor(l.buffer);
			continue;
		}
	}

	fclose(f);
	for(tracklist.Rewind();tracklist.actual;tracklist.Next()){
		tracklist.actual->matrix=0;
		MODEL *m=modellist.Search(tracklist.actual->matrixid);
		if(m&&!tracklist.actual->matrix){
			tracklist.actual->type=TRACKTYPE(TT_MODEL);
			tracklist.actual->matrix=&m->matrix;
			continue;
		}

		LIGHT *l=lightlist.Search(tracklist.actual->matrixid);
		if(l&&!tracklist.actual->matrix){
			tracklist.actual->type=TRACKTYPE(TT_LIGHT);
			tracklist.actual->matrix=&l->matrix;
			continue;
		}

		CAMERA *c=cameralist.Search(tracklist.actual->matrixid);
		if(c&&!tracklist.actual->matrix){
			tracklist.actual->type=TRACKTYPE(TT_CAMERA);
			tracklist.actual->matrix=&c->matrix;
			continue;
		}
	}

	for(tracklist.Rewind();tracklist.actual;tracklist.Next()){
		if(tracklist.actual->parentid){
			TRACK *t=tracklist.Search(tracklist.actual->parentid);
			if(t) tracklist.actual->parent=t;
		}

		if(tracklist.actual->targetid){
			TRACK *t=tracklist.Search(tracklist.actual->targetid);
			if(t) tracklist.actual->target=t;
		}
	}
}
