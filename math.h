#ifndef MATHH
#define MATHH

#include "stdafx.h"

/*

  math 3d

*/

float angle2rad(float);
float rad2angle(float);

class VECTOR;
class MATRIX;

class VECTOR{
public:
	float x,y,z;

	VECTOR(void);
	VECTOR(float,float,float);

	void Normalize(void);

	VECTOR operator-(void);
	VECTOR operator+(VECTOR);
	VECTOR operator*(float);
	VECTOR operator*(MATRIX);
	VECTOR operator-(VECTOR);
};

VECTOR Cross(VECTOR,VECTOR);
float Dot(VECTOR,VECTOR);

class MATRIX{
public:
	float m[4][4];

	MATRIX(void);
	MATRIX(VECTOR,VECTOR,VECTOR);

	void Identity(void);
	void Zero(void);
	void Perspective(float,float,float,float);
	void Target(VECTOR,VECTOR,float);
	void Rotate(VECTOR);
	void Scale(VECTOR);
	void Translate(VECTOR);

	MATRIX operator*(MATRIX);
};

#endif MATHH
