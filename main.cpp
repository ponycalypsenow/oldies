#include "stdafx.h"
#include "time.h"
#include "render.h"
#include "keyframer.h"
#include "lwo.h"
#include "lws.h"

HWND hWnd;
HINSTANCE hInst;
D3D d3d("fungi 2oo1",800,600);
TIME time;

LWS scn("scene.lws");

LRESULT CALLBACK WndProc(HWND h,UINT m,WPARAM w,LPARAM l){
	switch(m){
		case WM_PAINT:
			time.Get();
			scn.Do(time.scene);
			d3d.Begin(*scn.cameralist.head);
			scn.Draw();
			d3d.End();
			break;
		
		case WM_DESTROY:
			PostQuitMessage(0);
			break;

		case WM_KEYDOWN:
			if(w==VK_ESCAPE) PostQuitMessage(0);
			break;
		
		default:
			return DefWindowProc(h,m,w,l);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE i,HINSTANCE p, LPSTR c, int n){
	hInst=i;
	MSG msg;
	while(GetMessage(&msg,0,0,0)){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
