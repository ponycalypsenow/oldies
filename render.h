#ifndef D3DH
#define D3DH

#include "stdafx.h"
#include "math.h"

/*

  render

*/

class MODEL;

class COLOR{
public:
	float r,g,b,a;

	COLOR(void);
	COLOR(float,float,float,float);
};

class MATERIAL{
public:
	COLOR ambient;
	COLOR diffuse;
	COLOR emissive;
	COLOR specular;

	MATERIAL(void);
	MATERIAL(COLOR,COLOR,COLOR,COLOR);
};

class PIXEL{
public:
	BYTE r,g,b,a;

	PIXEL(void);
	PIXEL(BYTE,BYTE,BYTE,BYTE);
};

class UV{
public:
	float u,v;

	UV(void);
	UV(float,float);
};

typedef VECTOR VERTEX;

typedef VECTOR NORMAL;

class POLYGON{
public:
	int normal[3];
	int uv[3];
	int vertex[3];
};

	#define MT_LINE 1
	#define MT_POLYGON 2
	#define MT_PARTICLE 3

	class MODELTYPE{
	public:
		BYTE t;

		MODELTYPE(void);
		MODELTYPE(BYTE);
	};

	#define RS_BLEND 1
	#define RS_BLUR 2
	#define RS_CULL 4
	#define RS_RENDER 8
	#define RS_SMOOTH 16
	#define RS_TEXTURE 32

	class RENDERSTATE{
	public:
		BYTE s;

		RENDERSTATE(void);
		RENDERSTATE(BYTE);
	};

	class TEXTURE{
	public:
		int id;
		char name[32];
		int width,height,pitch;
		PIXEL *source;

		TEXTURE(void);
		TEXTURE(char *);
		TEXTURE(int,int);
		~TEXTURE(void);
		void Lock(void);
		void Unlock(void);

		TEXTURE *previous,*next;
	};

	class WALL{
	public:
		int id;
		char texturename[32];
		MATERIAL material;
		int polygonnumber;
		POLYGON *polygon;
		TEXTURE *texture;

		WALL(void);
		WALL(int);
		~WALL(void);
	};

	class MODEL{
	public:
		int id;
		char name[32];
		MATRIX matrix;
		MODELTYPE type;
		RENDERSTATE state;
		int vertexnumber,wallnumber;
		NORMAL *normal;
		UV *uv;
		VERTEX *vertex;
		WALL *wall;

		MODEL(void);
		MODEL(char *);
		MODEL(int,int);
		~MODEL(void);
		void Draw(void);
		void Normalize(void);

		MODEL *previous,*next;
	};

	#define LT_DISTANT 1
	#define LT_POINT 2
	#define LT_SPOT 3

	class LIGHTTYPE{
	public:
		char t;

		LIGHTTYPE(void);
		LIGHTTYPE(char);
	};

	class LIGHT{
	public:
		int id;
		LIGHTTYPE type;
		float a;
		COLOR color;
		MATRIX matrix;
		int number;

		LIGHT(void);
		void Set(void);

		LIGHT *previous,*next;
	};

	class CAMERA{
	public:
		int id;
		COLOR color;
		MATRIX matrix;
		float min,max,fovv,fovh;

		CAMERA(void);

		CAMERA *previous,*next;
	};

	class D3D{
	public:
		D3D(char *,int,int);
		~D3D(void);

		void Begin(CAMERA&);
		void End(void);
		void Draw(MODEL&);
	};

#endif
