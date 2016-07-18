// include the basic windows and D3d headers
#include <windows.h>
#include <windowsx.h>

#include <d3d11.h>
#include <DirectXMath.h>
#include <d3dcompiler.h>

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

IDXGISwapChain *swapchain;
ID3D11Device *dev;
ID3D11DeviceContext *devcon;
ID3D11RenderTargetView *backbuffer;
ID3D11InputLayout *pLayout;
ID3D11VertexShader *pVS;
ID3D11PixelShader *pPS;
ID3D11Buffer *pVBuffer;

struct directx_color
{
	FLOAT R, G, B, A;					//For getting individual colors
	//FLOAT color[4] = { R, G, B, A };	//For passing entire RGBA color value
} DIRECTX_COLOR;

struct VERTEX
{
	FLOAT X, Y, Z; 
	directx_color Color;
};

void InitD3D(HWND hwnd);
void CleanD3D(void);
void RenderFrame(void);
void InitGraphics(void);
void InitPipeline(void);

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd,
	UINT message,
	WPARAM wParam,
	LPARAM lParam);

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow)
{
	// the handle for the window, filled by a function
	HWND hWnd;
	// this struct holds information for the window class
	WNDCLASSEX wc;

	// clear out the window class for use
	ZeroMemory(&wc, sizeof(WNDCLASSEX));

	// fill in the struct with the needed information
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	//wc.hbrBackground = (HBRUSH)COLOR_WINDOW;
	wc.lpszClassName = L"WindowClass1";

	// register the window class
	RegisterClassEx(&wc);

	RECT wr = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	AdjustWindowRect(&wr, WS_OVERLAPPED, FALSE);

	// create the window and use the result as the handle
	hWnd = CreateWindowEx(NULL,
		L"WindowClass1",    // name of the window class
		L"Our First Windowed Program",   // title of the window
		WS_OVERLAPPEDWINDOW,    // window style
		300,    // x-position of the window
		300,    // y-position of the window
		wr.right - wr.left,    // width of the window
		wr.bottom - wr.top,    // height of the window
		NULL,    // we have no parent window, NULL
		NULL,    // we aren't using menus, NULL
		hInstance,    // application handle
		NULL);    // used with multiple windows, NULL

	// display the window on the screen
	ShowWindow(hWnd, nCmdShow);

	// enter the main loop:
	InitD3D(hWnd);

	// this struct holds Windows event messages
	MSG msg = {0};

	while (TRUE)
	{
		//Check message queue for any messages
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				break;
			}
		}
		else
		{
			//Game Code goes here.
			RenderFrame();
		}
	}

	CleanD3D();

	return msg.wParam;
}

// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	// sort through and find what code to run for the message given
	switch (message)
	{
		// this message is read when the window is closed
	case WM_DESTROY:
	{
		// close the application entirely
		PostQuitMessage(0);
		return 0;
	} break;
	}

	// Handle any messages the switch statement didn't
	return DefWindowProc(hWnd, message, wParam, lParam);
}

void InitD3D(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC scd;

	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferCount = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	scd.BufferDesc.Width = SCREEN_WIDTH;
	scd.BufferDesc.Height = SCREEN_HEIGHT;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWnd;
	scd.SampleDesc.Count = 4;
	scd.Windowed = TRUE;
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	//Create a device, device context and swap chain using scd
	D3D11CreateDeviceAndSwapChain(NULL,
								  D3D_DRIVER_TYPE_HARDWARE,
								  NULL,
								  NULL,
								  NULL,
								  NULL,
								  D3D11_SDK_VERSION,
								  &scd,
								  &swapchain,
								  &dev,
								  NULL,
								  &devcon);

	//Get address of the back buffer
	ID3D11Texture2D *pBackBuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

	//Use the back buffer to create a render target
	dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();

	//Set the render target as the back buffer
	devcon->OMSetRenderTargets(1, &backbuffer, NULL);

	//Set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;

	devcon->RSSetViewports(1, &viewport);

	InitPipeline();
	InitGraphics();
}

void RenderFrame(void)
{
	//Clear back buffer to deep blue
	const FLOAT color[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	devcon->ClearRenderTargetView(backbuffer, color);

	//3D rendering on back buffer here
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	devcon->IASetVertexBuffers(0, 1, &pVBuffer, &stride, &offset);

	devcon->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	devcon->Draw(3, 0);
	//Switch back buffer and front buffer
	swapchain->Present(0, 0);

}

void InitPipeline()
{
	ID3D10Blob *VS, *PS;
	D3DCompileFromFile(L"shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, &VS, 0);
	D3DCompileFromFile(L"shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, &PS, 0);
	
	dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
	dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);

	devcon->VSSetShader(pVS, 0, 0);
	devcon->PSSetShader(pPS, 0, 0);

	D3D11_INPUT_ELEMENT_DESC ied[] = 
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};

	dev->CreateInputLayout(ied, 2, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
	devcon->IASetInputLayout(pLayout);
}

void InitGraphics()
{
	VERTEX OurVerticies[] =
	{
		{ 0.0f, 0.5f, 0.0f, DIRECTX_COLOR = {1.0f, 0.0f, 0.0f, 1.0f} },
		{ 0.45f, -0.5f, 0.0f, DIRECTX_COLOR = {0.0f, 1.0f, 0.0f, 1.0f} },
		{ -0.45f, -0.5f, 0.0f, DIRECTX_COLOR = { 0.0f, 0.0f, 1.0f, 1.0f }}
	};

	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));

	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX) * 3;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

	dev->CreateBuffer(&bd, NULL, &pVBuffer);

	D3D11_MAPPED_SUBRESOURCE ms;
	devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, OurVerticies, sizeof(OurVerticies));
	devcon->Unmap(pVBuffer, NULL);
}

void CleanD3D()
{
	//Exit fullscreen (switch to windowed)
	swapchain->SetFullscreenState(FALSE, NULL);

	pLayout->Release();
	pVS->Release();
	pPS->Release();
	pVBuffer->Release();
	swapchain->Release();
	backbuffer->Release();
	dev->Release();
	devcon->Release();
}