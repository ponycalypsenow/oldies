#include "render.h"

extern HINSTANCE hInst;
extern HWND hWnd;
extern LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);

/*

  render

*/

#define D3DFVF_D3DVERTEX (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)

struct D3DVERTEX{
	float x,y,z;
	float nx,ny,nz;
	float tu,tv;
};

static RENDERSTATE lrs;
static TEXTURE lt;
static D3DVERTEX *vb=0;
static D3D *Render=0;
static IDirect3D8 *d3d=0;
IDirect3DDevice8 *d3dd=0;

COLOR::COLOR(void){
	r=g=b=a=0;
}

COLOR::COLOR(float nr,float ng,float nb,float na){
	r=nr,g=ng,b=nb,a=na;
}

MATERIAL::MATERIAL(void){
}

MATERIAL::MATERIAL(COLOR a,COLOR d,COLOR e,COLOR s){
	ambient=a,diffuse=d,emissive=e,specular=s;
}

PIXEL::PIXEL(void){
	r=g=b=a=0;
}

PIXEL::PIXEL(BYTE nr,BYTE ng,BYTE nb,BYTE na){
	r=nr,g=ng,b=nb,a=na;
}

UV::UV(void){
	u=v=0;
}

UV::UV(float nu,float nv){
	u=nu,v=nv;
}

MODELTYPE::MODELTYPE(void){
	t=0;
}

MODELTYPE::MODELTYPE(BYTE nt){
	t=nt;
}

RENDERSTATE::RENDERSTATE(void){
	s=0;
}

RENDERSTATE::RENDERSTATE(BYTE ns){
	s=ns;
}

TEXTURE::TEXTURE(void){
	id=width=height=0;
	pitch=0,source=0;
}

TEXTURE::TEXTURE(char *n){
	strcpy(name,n);
	id=width=height=0;
	pitch=0,source=0;
}

TEXTURE::TEXTURE(int w,int h){
	width=w;
	height=h;
	d3dd->CreateTexture(w,h,1,0,D3DFMT_R8G8B8,D3DPOOL_MANAGED,(IDirect3DTexture8**)&id);
	if(!id) error("can't create texture");
	pitch=0,source=0;
}

TEXTURE::~TEXTURE(void){
}

void TEXTURE::Lock(void){
	IDirect3DTexture8 *t=(IDirect3DTexture8 *)id;
	D3DLOCKED_RECT d3dlr;	
	t->LockRect(0,&d3dlr,0,0);
	source=(PIXEL *)d3dlr.pBits;
	pitch=d3dlr.Pitch;
}

void TEXTURE::Unlock(void){
	IDirect3DTexture8 *t=(IDirect3DTexture8 *)id;
	t->UnlockRect(0);
}

WALL::WALL(void){
	id=polygonnumber=0;
	polygon=0,texture=0;
}

WALL::WALL(int p){
	id=0,polygonnumber=p;
	polygon=new POLYGON[p];
	if(!polygon) error("out of memory");
	texture=0;
}

WALL::~WALL(void){
	delete [] polygon;
}

MODEL::MODEL(void){
	id=vertexnumber=wallnumber=0;
	wall=0,vertex=0,normal=0,uv=0;
}

MODEL::MODEL(char *n){
	strcpy(name,n);
	id=vertexnumber=wallnumber=0;
	wall=0,vertex=0,normal=0,uv=0;
}

MODEL::MODEL(int v,int w){
	wallnumber=w;
	wall=new WALL[w];
	vertexnumber=v;
	vertex=new VERTEX[v];
	normal=new NORMAL[v];
	uv=new UV[v];
	if(!wall||!vertex||!normal||!uv) error("out of memory");
}

MODEL::~MODEL(void){
	delete [] wall;
	delete [] vertex;
	delete [] normal;
	delete [] uv;
}

void MODEL::Draw(void){
	Render->Draw(*this);
}

void MODEL::Normalize(void){
	for(int i=0;i<wallnumber;i++)
		for(int j=0;j<wall[i].polygonnumber;j++)
			for(int k=0;k<3;k++) wall[i].polygon[j].normal[k]=wall[i].polygon[j].vertex[k];

	for(i=0;i<vertexnumber;i++){
		normal[i]=NORMAL(0,0,0);
		for(int j=0;j<wallnumber;j++){
			for(int k=0;k<wall[j].polygonnumber;k++){
				if(wall[j].polygon[k].normal[0]==i||wall[j].polygon[k].normal[1]==i||wall[j].polygon[k].normal[2]==i){
					VECTOR a=vertex[wall[j].polygon[k].normal[0]];
					VECTOR b=vertex[wall[j].polygon[k].normal[1]];
					VECTOR c=vertex[wall[j].polygon[k].normal[2]];

					normal[i]=normal[i]+Cross(b-a,c-a);
				}
			}
		}

		normal[i].Normalize();
	}
}

LIGHTTYPE::LIGHTTYPE(void){
	t=0;
}

LIGHTTYPE::LIGHTTYPE(char nt){
	t=nt;
}

LIGHT::LIGHT(void){
	a=1,number=0;
}

void LIGHT::Set(void){
		D3DLIGHT8 d3dl;
		ZeroMemory(&d3dl,sizeof(D3DLIGHT8));
		d3dl.Range=1000,d3dl.Falloff=d3dl.Phi=d3dl.Theta=1,d3dl.Attenuation0=a;
		d3dl.Diffuse.r=color.r,d3dl.Diffuse.g=color.g,d3dl.Diffuse.b=color.b,d3dl.Diffuse.a=color.a,d3dl.Ambient.a=d3dl.Specular.a=1;
		VECTOR p=VECTOR(0,0,0)*matrix;
		VECTOR d=VECTOR(0,0,1)*matrix;
		d=d-p;
		d.Normalize();
		switch(type.t){
			case LT_DISTANT:
				d3dl.Type=D3DLIGHT_DIRECTIONAL;
				d3dl.Direction.x=d.x;
				d3dl.Direction.y=d.y;
				d3dl.Direction.z=d.z;
				break;

			case LT_POINT:
				d3dl.Type=D3DLIGHT_POINT;
				d3dl.Position.x=p.x;
				d3dl.Position.y=p.y;
				d3dl.Position.z=p.z;
				break;

			case LT_SPOT:
				//todo
				break;
		}

		d3dd->SetLight(number,&d3dl);
		d3dd->LightEnable(number,1);
}

CAMERA::CAMERA(void){
	color=COLOR(0,0,0,0);
	min=0.001f,max=1000,fovh=angle2rad(60),fovv=angle2rad(60.0f*0.75f);
}

D3D::D3D(char *n,int w,int h){
	Render=this;
	vb=new D3DVERTEX[256];
	if(!vb) error("out of memory");
	WNDCLASSEX wc={sizeof(WNDCLASSEX),CS_VREDRAW|CS_HREDRAW|CS_OWNDC,WndProc,0,0,hInst,0,0,0,0,n,0};
	RegisterClassEx(&wc);
	hWnd=CreateWindowEx(WS_EX_APPWINDOW|WS_EX_TOPMOST,n,n,0,0,0,w,h,0,0,hInst,0);
	ShowWindow(hWnd,0);
	SetFocus(hWnd);
	ShowCursor(0);
	UpdateWindow(hWnd);
	d3d=Direct3DCreate8(D3D_SDK_VERSION);
	if(!d3d) error("can`t create d3d");
	D3DDISPLAYMODE d3ddm;
	ZeroMemory(&d3ddm,sizeof(d3ddm));
	d3d->GetAdapterDisplayMode(D3DADAPTER_DEFAULT,&d3ddm);
	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory(&d3dpp,sizeof(d3dpp));
	d3dpp.SwapEffect=D3DSWAPEFFECT_FLIP;
	d3dpp.BackBufferWidth=w;
	d3dpp.BackBufferHeight=h;
	d3dpp.BackBufferFormat=D3DFMT_A8R8G8B8;
	d3dpp.BackBufferCount=1;
	d3dpp.EnableAutoDepthStencil=1;
	d3dpp.AutoDepthStencilFormat=D3DFMT_D16;
	d3d->CreateDevice(D3DADAPTER_DEFAULT,D3DDEVTYPE_HAL,hWnd,D3DCREATE_SOFTWARE_VERTEXPROCESSING,&d3dpp,&d3dd);
	if(!d3dd) error("can`t create d3dd");
	d3dd->SetVertexShader(D3DFVF_D3DVERTEX);
	d3dd->SetRenderState(D3DRS_LIGHTING,1);
	d3dd->SetRenderState(D3DRS_NORMALIZENORMALS,1);
    d3dd->SetRenderState(D3DRS_POINTSPRITEENABLE,1);
    d3dd->SetRenderState(D3DRS_POINTSCALEENABLE,1);
	d3dd->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
	d3dd->SetRenderState(D3DRS_ALPHABLENDENABLE,0);
	d3dd->SetRenderState(D3DRS_SRCBLEND,D3DBLEND_SRCALPHA);
	d3dd->SetRenderState(D3DRS_DESTBLEND,D3DBLEND_ONE);
	d3dd->SetTextureStageState(0,D3DTSS_ALPHAOP,D3DTOP_BLENDDIFFUSEALPHA);
	d3dd->SetRenderState(D3DRS_ZWRITEENABLE,1);
	d3dd->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_FLAT);
	d3dd->SetTexture(0,0);
	d3dd->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_POINT);
	d3dd->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_POINT);
}

D3D::~D3D(void){
	delete [] vb;
	d3dd->Release();
	d3d->Release();
}

void D3D::Begin(CAMERA& c){
	d3dd->Clear(0,0,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,D3DCOLOR_XRGB((BYTE)(255*c.color.r),(BYTE)(255*c.color.g),(BYTE)(255*c.color.b)),1,1);
	MATRIX m;
	m.Perspective(c.min,c.max,c.fovh,c.fovv);
	d3dd->SetTransform(D3DTS_PROJECTION,(D3DMATRIX *)&m);
	d3dd->SetTransform(D3DTS_VIEW,(D3DMATRIX *)&c.matrix);
	d3dd->BeginScene();
}

void D3D::End(void){
	d3dd->EndScene();
	d3dd->Present(0,0,0,0);
	for(int i=0;i<8;i++) d3dd->LightEnable(i,0);
}

static void set(RENDERSTATE& r){
	if((RS_CULL&r.s)&&!(RS_CULL&lrs.s))
		d3dd->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);

	if(!(RS_CULL&r.s)&&(RS_CULL&lrs.s))
		d3dd->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);

	if((RS_BLEND&r.s)&&!(RS_BLEND&lrs.s)){
		d3dd->SetRenderState(D3DRS_ALPHABLENDENABLE,1);
		d3dd->SetRenderState(D3DRS_ZWRITEENABLE,0);
	}

	if(!(RS_BLEND&r.s)&&(RS_BLEND&lrs.s)){
		d3dd->SetRenderState(D3DRS_ALPHABLENDENABLE,0);
		d3dd->SetRenderState(D3DRS_ZWRITEENABLE,1);
	}

	if((RS_SMOOTH&r.s)&&!(RS_SMOOTH&lrs.s))
		d3dd->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_GOURAUD);

	if(!(RS_SMOOTH&r.s)&&(RS_SMOOTH&lrs.s))
		d3dd->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_FLAT);

	if(!(RS_TEXTURE&r.s)&&(RS_TEXTURE&lrs.s))
		d3dd->SetTexture(0,0);

	if((RS_BLUR&r.s)&&!(RS_BLUR&lrs.s)){
		d3dd->SetRenderState(D3DRS_SHADEMODE,D3DSHADE_GOURAUD);
		d3dd->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_POINT);
		d3dd->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_POINT);
	}

	if(!(RS_BLUR&r.s)&&(RS_BLUR&lrs.s)){
		d3dd->SetTextureStageState(0,D3DTSS_MINFILTER,D3DTEXF_LINEAR);
		d3dd->SetTextureStageState(0,D3DTSS_MAGFILTER,D3DTEXF_LINEAR);
	}

	lrs=r;
}

static void set(MATRIX& m){
	d3dd->SetTransform(D3DTS_WORLD,(D3DMATRIX *)&m);
}

static void set(MATERIAL& m){
	D3DMATERIAL8 mtrl;
	mtrl.Diffuse.r=m.diffuse.r,mtrl.Diffuse.g=m.diffuse.g,mtrl.Diffuse.b=m.diffuse.b,mtrl.Diffuse.a=m.diffuse.a;
	mtrl.Ambient.r=m.ambient.r,mtrl.Ambient.g=m.ambient.g,mtrl.Ambient.b=m.ambient.b,mtrl.Ambient.a=m.ambient.a;
	mtrl.Specular.r=m.specular.r,mtrl.Specular.g=m.specular.g,mtrl.Specular.b=m.specular.b,mtrl.Specular.a=m.specular.a;
	mtrl.Emissive.r=m.emissive.r,mtrl.Emissive.g=m.emissive.g,mtrl.Emissive.b=m.emissive.b,mtrl.Emissive.a=m.emissive.a;
	d3dd->SetMaterial(&mtrl);
}

static void set(TEXTURE& t){
	if(t.id!=lt.id&&t.id){
		d3dd->SetTexture(0,(IDirect3DTexture8 *)t.id);
		lt=t;
	}
}

void D3D::Draw(MODEL& m){
	if(RS_RENDER&m.state.s){
		set(m.state);
		set(m.matrix);
		int i;
		int j=m.vertexnumber;
		switch(m.type.t){
			case MT_LINE:
				for(i=0;i<m.wallnumber;i++){
					set(m.wall[i].material);
					if(RS_TEXTURE&m.state.s) set(*m.wall[i].texture);
					for(int j=0;j<m.wall[i].polygonnumber;j++){
						for(int k=0;k<3;k++){
							vb[k].nx=m.normal[m.wall[i].polygon[j].normal[k]].x;
							vb[k].ny=m.normal[m.wall[i].polygon[j].normal[k]].y;
							vb[k].nz=m.normal[m.wall[i].polygon[j].normal[k]].z;
							vb[k].tu=m.uv[m.wall[i].polygon[j].uv[k]].u;
							vb[k].tv=m.uv[m.wall[i].polygon[j].uv[k]].v;
							vb[k].x=m.vertex[m.wall[i].polygon[j].vertex[k]].x;
							vb[k].y=m.vertex[m.wall[i].polygon[j].vertex[k]].y;
							vb[k].z=m.vertex[m.wall[i].polygon[j].vertex[k]].z;
						}

						vb[3].nx=vb[0].nx;
						vb[3].ny=vb[0].ny;
						vb[3].nz=vb[0].nz;
						vb[3].tu=vb[0].tu;
						vb[3].tv=vb[0].tv;
						vb[3].x=vb[0].x;
						vb[3].y=vb[0].y;
						vb[3].z=vb[0].z;
						d3dd->DrawPrimitiveUP(D3DPT_LINESTRIP,3,vb,sizeof(D3DVERTEX));
					}
				}

				break;

			case MT_POLYGON:
				for(i=0;i<m.wallnumber;i++){
					set(m.wall[i].material);
					if(RS_TEXTURE&m.state.s) set(*m.wall[i].texture);
					for(int j=0;j<m.wall[i].polygonnumber;j++){
						for(int k=0;k<3;k++){
							vb[k].nx=m.normal[m.wall[i].polygon[j].normal[k]].x;
							vb[k].ny=m.normal[m.wall[i].polygon[j].normal[k]].y;
							vb[k].nz=m.normal[m.wall[i].polygon[j].normal[k]].z;
							vb[k].tu=m.uv[m.wall[i].polygon[j].uv[k]].u;
							vb[k].tv=m.uv[m.wall[i].polygon[j].uv[k]].v;
							vb[k].x=m.vertex[m.wall[i].polygon[j].vertex[k]].x;
							vb[k].y=m.vertex[m.wall[i].polygon[j].vertex[k]].y;
							vb[k].z=m.vertex[m.wall[i].polygon[j].vertex[k]].z;
						}

						vb[3].nx=vb[0].nx;
						vb[3].ny=vb[0].ny;
						vb[3].nz=vb[0].nz;
						vb[3].tu=vb[0].tu;
						vb[3].tv=vb[0].tv;
						vb[3].x=vb[0].x;
						vb[3].y=vb[0].y;
						vb[3].z=vb[0].z;
						d3dd->DrawPrimitiveUP(D3DPT_TRIANGLEFAN,3,vb,sizeof(D3DVERTEX));
					}
				}

				break;

			case MT_PARTICLE://todo
				d3dd->SetRenderState(D3DRS_POINTSIZE,*((DWORD *)&m.uv[0].u));
				while(j>=0){
					for(i=0;i<256&&j>=0;i++,j--) vb[i].x=m.vertex[j].x,vb[i].y=m.vertex[j].y,vb[i].z=m.vertex[j].z;
					d3dd->DrawPrimitiveUP(D3DPT_POINTLIST,i,vb,sizeof(D3DVERTEX));
				}

				break;

			default:
				break;
		}
	}
}
