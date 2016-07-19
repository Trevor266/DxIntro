#ifndef _D3DCLASS_H_
#define _D3DCLASS_H_

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>
#include <DirectXMath.h>
#include <dxgi.h>
#include <d3dcommon.h>

using namespace DirectX;

class D3DClass
{
public:

	D3DClass();
	D3DClass(const D3DClass&);
	~D3DClass();

	bool Initialize(int, int, bool, HWND, bool, float, float);
	void Shutdown();

	void BeginScene(float, float, float, float);
	void EndScene();

	ID3D11Device *GetDevice();
	ID3D11DeviceContext *GetDeviceContext();

	void GetProjectionMatrix(DirectX::CXMMATRIX);

private:


};

#endif