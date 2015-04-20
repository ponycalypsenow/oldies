#ifndef KEYFRAMERH
#define KEYFRAMERH

#include "stdafx.h"
#include "render.h"

/*

  keyframer

*/

class KEY{
public:
	float frame;
	VECTOR rotate;
	VECTOR rotatet0;
	VECTOR rotatet1;
	VECTOR scale;
	VECTOR scalet0;
	VECTOR scalet1;
	VECTOR translate;
	VECTOR translatet0;
	VECTOR translatet1;

	KEY *previous,*next;
};

#define TT_MODEL 1
#define TT_LIGHT 2
#define TT_CAMERA 3

class TRACKTYPE{
public:
	char t;

	TRACKTYPE(void);
	TRACKTYPE(char);
};

#define EB_RESET 1
#define EB_STOP 2
#define EB_REPEAT 3

class ENDBEHAVIOR{
public:
	char behavior;

	ENDBEHAVIOR(void);
	ENDBEHAVIOR(char);
};

class TRACK:public LIST<KEY>{
public:
	int matrixid,parentid,targetid;
	TRACKTYPE type;
	VECTOR rotate;
	ENDBEHAVIOR rotatebehavior;
	VECTOR scale;
	ENDBEHAVIOR scalebehavior;
	VECTOR translate;
	ENDBEHAVIOR translatebehavior;
	MATRIX pivot;
	MATRIX *matrix;
	TRACK *parent;
	TRACK *target;

	TRACK(void);

	void Do(float);

	TRACK *previous,*next;
};

class TEXTURELIST:public LIST<TEXTURE>{
public:

	TEXTURE *Search(char *);
};

class MODELLIST:public LIST<MODEL>{
public:

	MODEL *Search(int);
	MODEL *Search(char *);
	void Draw(void);
};

class LIGHTLIST:public LIST<LIGHT>{
public:
	
	LIGHT *Search(int);
};

class CAMERALIST:public LIST<CAMERA>{
public:

	CAMERA *Search(int);
};

class TRACKLIST:public LIST<TRACK>{
public:

	TRACK *Search(int);
	void Do(float);
};

class SCENE{
public:
	TEXTURELIST texturelist;
	MODELLIST modellist;
	LIGHTLIST lightlist;
	CAMERALIST cameralist;
	TRACKLIST tracklist;

	void Do(float);
	void Draw(void);
};

#endif
