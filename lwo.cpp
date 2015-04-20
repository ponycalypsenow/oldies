#include "lwo.h"

/*

  lwo loader

*/

#define LWO_PNTS 0x504e5453
#define LWO_POLS 0x504f4c53
#define LWO_PTAG 0x50544147
#define LWO_COLR 0x434f4c52
#define LWO_LUMI 0x4c554d49
#define LWO_DIFF 0x44494646
#define LWO_SPEC 0x53504543
#define LWO_TRAN 0x5452414e
#define LWO_VMAP 0x564d4150
#define LWO_STIL 0x5354494c
#define LWO_STIL 0x5354494c
#define LWO_IMAG 0x494d4147

struct HEAD{
	int id,size;
};

static FILE *f;
static LWO *lwo;
static int pn;
static int tn;
static int as;
static int at;
static POLYGON *p;
static char *t;

static float rread(void){
	float ret;
	char *swp=(char *)&ret;
	fread(&ret,4,1,f);
	char a=swp[3];
	char b=swp[2];
	char c=swp[1];
	char d=swp[0];
	swp[0]=a,swp[1]=b,swp[2]=c,swp[3]=d;
	return ret;
}

static DWORD dwread(void){
	DWORD ret;
	char *swp=(char *)&ret;
	fread(&ret,4,1,f);
	char a=swp[3];
	char b=swp[2];
	char c=swp[1];
	char d=swp[0];
	swp[0]=a,swp[1]=b,swp[2]=c,swp[3]=d;
	return ret;
}

static WORD wread(void){
	WORD ret;
	char *swp=(char *)&ret;
	fread(&ret,2,1,f);
	char a=swp[1];
	char b=swp[0];
	swp[0]=a,swp[1]=b;
	return ret;
}

static HEAD hread(void){
	HEAD ret;
	ret.id=dwread();
	ret.size=dwread();
	return ret;
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

static void pntscheck(int s){
	lwo->vertexnumber=s/sizeof(VERTEX);
}

static void polscheck(int s){
	fseek(f,4,SEEK_CUR);int i=4;
	while(i<s){
		WORD tmp=wread();i+=2;
		if(tmp!=3) error("engine works only with triangles, and uv maps");
		fseek(f,6,SEEK_CUR);i+=6;
		pn++;
	}
}

static void pnts(void){
	for(int i=0;i<lwo->vertexnumber;i++){
		lwo->vertex[i].x=rread();
		lwo->vertex[i].y=rread();
		lwo->vertex[i].z=rread();
	}
}

static void pols(void){
	fseek(f,4,SEEK_CUR);
	for(int i=0;i<pn;i++){
		fseek(f,2,SEEK_CUR);
		for(int j=0;j<3;j++) p[i].vertex[j]=wread();
	}
}

void ptag(void){
	fseek(f,4,SEEK_CUR);
	int o=ftell(f);
	for(int i=0;i<pn;i++){
		fseek(f,2,SEEK_CUR);
		int j=wread();
		lwo->wall[j].polygonnumber++;
	}

	fseek(f,o,SEEK_SET);
	for(i=0;i<lwo->wallnumber;i++){
		lwo->wall[i].polygon=new POLYGON[lwo->wall[i].polygonnumber];
		if(!lwo->wall[i].polygon) error("out of memory");
		lwo->wall[i].polygonnumber=0;
	}
		
	for(i=0;i<pn;i++){
		fseek(f,2,SEEK_CUR);
		int j=wread();
		for(int k=0;k<3;k++) lwo->wall[j].polygon[lwo->wall[j].polygonnumber].vertex[k]=p[i].vertex[k];
		lwo->wall[j].polygonnumber++;
	}
}

static int colrcheck(void){
	fseek(f,-4,SEEK_CUR);
	DWORD tmp=dwread();
	if(tmp==0x4f504143) return 0;
	else return 1;
}

static void colr(void){
	fseek(f,-2,SEEK_CUR);
	lwo->wall[as].material.ambient.r=rread();
	lwo->wall[as].material.ambient.g=rread();
	lwo->wall[as].material.ambient.b=rread();
	lwo->wall[as].material.ambient.a=lwo->wall[as].material.diffuse.a=lwo->wall[as].material.specular.a=lwo->wall[as].material.emissive.a=1.0f;
}

static void lumi(void){
	fseek(f,-2,SEEK_CUR);
	float color=rread();
	lwo->wall[as].material.emissive.r=lwo->wall[as].material.ambient.r*color;
	lwo->wall[as].material.emissive.g=lwo->wall[as].material.ambient.g*color;
	lwo->wall[as].material.emissive.b=lwo->wall[as].material.ambient.b*color;
}

static void diff(void){
	fseek(f,-2,SEEK_CUR);
	float color=rread();
	lwo->wall[as].material.diffuse.r=lwo->wall[as].material.ambient.r*color;
	lwo->wall[as].material.diffuse.g=lwo->wall[as].material.ambient.g*color;
	lwo->wall[as].material.diffuse.b=lwo->wall[as].material.ambient.b*color;
}

static void spec(void){
	fseek(f,-2,SEEK_CUR);
	float color=rread();
	lwo->wall[as].material.specular.r=lwo->wall[as].material.ambient.r*color;
	lwo->wall[as].material.specular.g=lwo->wall[as].material.ambient.g*color;
	lwo->wall[as].material.specular.b=lwo->wall[as].material.ambient.b*color;
}

static void tran(void){
	fseek(f,-2,SEEK_CUR);
	float alpha=rread();
	if(alpha){
		alpha=1.0f-alpha;
		lwo->wall[as].material.ambient.a=lwo->wall[as].material.diffuse.a=lwo->wall[as].material.emissive.a=lwo->wall[as].material.specular.a=alpha;
		lwo->state.s|=RS_BLEND;
	}
}

static int vmapcheck(void){
	DWORD tmp=dwread();
	if(tmp==0x54585556) return 1;
	else return 0;
}

static void vmap(int s){
	s-=4;
	fseek(f,2,SEEK_CUR);s-=2;
	for(char tmp=1;tmp;s--) fread(&tmp,1,1,f);
	fseek(f,1,SEEK_CUR);s--;
	while(s>=10){
		int i=wread();
		lwo->uv[i].u=rread();
		lwo->uv[i].v=rread();
		s-=10;
	}
}

static void stil(void){
	char *d=&t[at*96];
	fseek(f,-2,SEEK_CUR);
	fgets(d,32,f);
}

static void imag(void){
	fseek(f,-2,SEEK_CUR);
	int i=wread()-1;
	strcpy(lwo->wall[as].texturename,nskip(&t[i*96]));
}

LWO::LWO(char *n){
	f=0;
	f=fopen(n,"rb");
	if(!f) error("file doesn't exist");
	as=at=-1,p=0,t=0,pn=tn=0,lwo=this;
	strcpy(lwo->name,n);
	fseek(f,0,SEEK_END);
	int size=ftell(f);
	rewind(f);
	while(ftell(f)<size-7){
		HEAD h=hread();
		switch(h.id){
			case LWO_PNTS:
				pntscheck(h.size);
				break;

			case LWO_POLS:
				polscheck(h.size);
				break;

			case LWO_COLR:
				if(colrcheck()) wallnumber++;
				break;

			case LWO_STIL:
				tn++;
				break;

			default:
				break;
		}

		fseek(f,-7,SEEK_CUR);
	}

	normal=new VERTEX[vertexnumber];
	uv=new UV[vertexnumber];
	vertex=new VERTEX[vertexnumber];
	wall=new WALL[wallnumber];
	p=new POLYGON[pn];
	t=new char[96*tn];
	if(!normal||!uv||!vertex||!wall||!p||!t) error("out of memory");
	ZeroMemory(t,sizeof(t));
	rewind(f);
	while(ftell(f)<size-7){
		HEAD h=hread();
		switch(h.id){
			case LWO_PNTS:
				pnts();
				break;

			case LWO_POLS:
				pols();
				break;

			case LWO_PTAG:
				ptag();
				break;

			case LWO_COLR:
				if(colrcheck()){
					as++;
					colr();
				}

				break;

			case LWO_LUMI:
				lumi();
				break;

			case LWO_DIFF:
				diff();
				break;

			case LWO_SPEC:
				spec();
				break;

			case LWO_TRAN:
				tran();
				break;

			case LWO_VMAP:
				if(vmapcheck()) vmap(h.size);
				break;

			case LWO_STIL:
				at++;
				stil();
				break;

			case LWO_IMAG:
				imag();
				break;

			default:
				break;
		}

		fseek(f,-7,SEEK_CUR);
	}

	fclose(f);
	delete [] t;
	delete [] p;
	Normalize();
}
