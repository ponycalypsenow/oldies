#include "math.h"

/*

  math 3d

*/

float angle2rad(float a){
	return a*0.0174532925199432957692369076848877f;
}

float rad2angle(float r){
	return r*57.2957795130823208767981548170142f;
}

static MATRIX RotateX(float a){
	float cosinus=cosf(a);
	float sinus=sinf(a);
	MATRIX ret;
	ret.Identity();
	ret.m[1][1]=cosinus;
	ret.m[2][2]=cosinus;
	ret.m[1][2]=-sinus;
	ret.m[2][1]=sinus;
	return ret;
}

static MATRIX RotateY(float a){
	float cosinus=cosf(a);
	float sinus=sinf(a);
	MATRIX ret;
	ret.Identity();
	ret.m[0][0]=cosinus;
	ret.m[2][2]=cosinus;
	ret.m[0][2]=sinus;
	ret.m[2][0]=-sinus;
	return ret;
}

static MATRIX RotateZ(float a){
	float cosinus=cosf(a);
	float sinus=sinf(a);
	MATRIX ret;
	ret.Identity();
	ret.m[0][0]=cosinus;
	ret.m[1][1]=cosinus;
	ret.m[0][1]=-sinus;
	ret.m[1][0]=sinus;
	return ret;
}

VECTOR::VECTOR(void){
	x=y=z=0;
}

VECTOR::VECTOR(float nx,float ny,float nz){
	x=nx,y=ny,z=nz;
}

void VECTOR::Normalize(void){
	float l=sqrtf(x*x+y*y+z*z);
	l=1.0f/l;
	x*=l,y*=l,z*=l;
}

VECTOR VECTOR::operator+(VECTOR v){
	VECTOR ret;
	ret.x=x+v.x,ret.y=y+v.y,ret.z=z+v.z;
	return ret;
}

VECTOR VECTOR::operator-(VECTOR v){
	VECTOR ret;
	ret.x=x-v.x,ret.y=y-v.y,ret.z=z-v.z;
	return ret;
}

VECTOR VECTOR::operator*(float s){
	VECTOR ret;
	ret.x=x*s,ret.y=y*s,ret.z=z*s;
	return ret;
}

VECTOR VECTOR::operator*(MATRIX m){
	VECTOR ret;
	ret.x=x*m.m[0][0]+y*m.m[1][0]+z*m.m[2][0]+m.m[3][0];
	ret.y=x*m.m[0][1]+y*m.m[1][1]+z*m.m[2][1]+m.m[3][1];
	ret.z=x*m.m[0][2]+y*m.m[1][2]+z*m.m[2][2]+m.m[3][2];
	float w=x*m.m[0][3]+y*m.m[1][3]+z*m.m[2][3]+m.m[3][3];
	if(!w)
		return VECTOR(0,0,0);
	w=1.0f/w;
	ret.x*=w,ret.y*=w,ret.z*=w;
	return ret;
}

VECTOR VECTOR::operator-(void){
	return VECTOR(-x,-y,-z);
}

VECTOR Cross(VECTOR a,VECTOR b){
	VECTOR ret;
	ret.x=a.y*b.z-a.z*b.y;
	ret.y=a.z*b.x-a.x*b.z;
	ret.z=a.x*b.y-a.y*b.x;
	return ret;
}

float Dot(VECTOR a,VECTOR b){
	return a.x*b.x+a.y*b.y+a.z*b.z;
}

MATRIX::MATRIX(void){
}

MATRIX::MATRIX(VECTOR t,VECTOR s,VECTOR r){
	Identity();
	Rotate(r);
	Scale(s);
	Translate(t);
};

void MATRIX::Identity(void){
	Zero();
	m[0][0]=m[1][1]=m[2][2]=m[3][3]=1;
}

void MATRIX::Zero(void){
	for(int i=0;i<4;i++) m[i][0]=m[i][1]=m[i][2]=m[i][3]=0;
}

void MATRIX::Perspective(float mn,float mx,float fh,float fv){    
	fh=fh*0.5f;
	fv=fv*0.5f;
	float w=cosf(fh)/sinf(fh);
	float h=cosf(fv)/sinf(fv);
	float Q=mx/(mx-mn);
	Zero();
	m[0][0]=w;    
	m[1][1]=h;    
	m[2][2]=Q;
	m[3][2]=-Q*mn;    
	m[2][3]=1;
}

void MATRIX::Target(VECTOR v,VECTOR c,float a){
	VECTOR u(0,1,0);
	VECTOR z=v-c;z.Normalize();
	VECTOR y=u-z*Dot(u,z);y.Normalize();
	VECTOR x=Cross(y,z);
	MATRIX swp;
	swp.Identity();
	swp.m[0][0]=x.x,swp.m[0][1]=y.x,swp.m[0][2]=z.x;
	swp.m[1][0]=x.y,swp.m[1][1]=y.y,swp.m[1][2]=z.y;
	swp.m[2][0]=x.z,swp.m[2][1]=y.z,swp.m[2][2]=z.z;
	swp.m[3][0]=-Dot(c,x);
	swp.m[3][1]=-Dot(c,y);
	swp.m[3][2]=-Dot(c,z);
	*this=RotateZ(-a)*swp;
}

void MATRIX::Rotate(VECTOR v){
	*this=*this*RotateZ(v.z)*RotateY(v.y)*RotateX(v.x);
}

void MATRIX::Scale(VECTOR v){
	MATRIX swp;
	swp.Identity();
	swp.m[0][0]=v.x;
	swp.m[1][1]=v.y;
	swp.m[2][2]=v.z;
	*this=*this*swp;
}

void MATRIX::Translate(VECTOR v){
	MATRIX swp;
	swp.Identity();
	swp.m[3][0]=v.x;
	swp.m[3][1]=v.y;
	swp.m[3][2]=v.z;
	*this=*this*swp;
}

MATRIX MATRIX::operator*(MATRIX r){
	MATRIX ret;
	ret.Zero();
	for(int i=0;i<4;i++){
		for(int j=0;j<4;j++){
			ret.m[i][j]+=m[0][j]*r.m[i][0];
			ret.m[i][j]+=m[1][j]*r.m[i][1];
			ret.m[i][j]+=m[2][j]*r.m[i][2];
			ret.m[i][j]+=m[3][j]*r.m[i][3];
		}
	}
	return ret;
}
