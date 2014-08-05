#ifndef _GAME_H_INCLUDED
#define _GAME_H_INCLUDED

#define _XM_NO_INTRINSICS_

#include "WinCreation.h"
#include <d3dx11.h>
#include <D3D11.h>
#include "GameLevel.h"
#include "Util.h"

extern HINSTANCE g_hInstance;
extern HINSTANCE g_hPrevInstance;
extern int g_nCmdShow;

class Game;
extern Game *g_Game;

DWORD lastTickTime = 0;
float deltaTime = 0;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

class Game
{
private:

	/* Propiedades de la ventana */
	HWND	_hWND;
	int		_width, _height;
	bool	_isFullscreen;

	/* Propiedades Direct3D */
	ID3D11Device				*_device;
	ID3D11DeviceContext			*_deviceContext;
	IDXGISwapChain				*_swapChain;
	ID3D11RenderTargetView		*_targetView;
	ID3D11Texture2D				*_depthTexture;
	ID3D11DepthStencilView		*_depthStencilView;
	ID3D11DepthStencilState		*_depthStencilState;
	D3D_DRIVER_TYPE				_driverType;
	D3D_FEATURE_LEVEL			_featureLevel;

	/* Estado del juego */
	bool _gameIsRunning;
	GameLevel *_gameLevel;

public:
	Game(bool isFullscreen = false)
	{
		_width = 800;
		_height = 640;
		_isFullscreen	= isFullscreen;

		if ( _isFullscreen )
			GetMonitorResolution(&_width, &_height);

		_device				= NULL;
		_deviceContext		= NULL;
		_swapChain			= NULL;
		_targetView			= NULL;
		_depthTexture		= NULL;
		_depthStencilView	= NULL;
		_depthStencilState	= NULL;
		_gameLevel			= NULL;

		g_Game = this;
	}

	~Game()
	{
		ReleaseResources();
	}

	int Run()
	{
		if ( InitWindowAndGraphics() )
		{
			_gameLevel = new SimpleRenderLevel(_device);

			MSG message;

			while ( _gameIsRunning )
			{
				if (PeekMessage(&message, _hWND, NULL, NULL, PM_REMOVE))
				{
					// Ocurrió un evento (teclado, ventana, etc)

					// Prepara el mensaje del evento
					TranslateMessage(&message);

					// Lo envía a la función que manejará el mensaje
					DispatchMessage(&message);
				}
				else
				{
					// Si no ocurrieron eventos, se dibuja un nuevo frame
					DoFrame();
				}
			}

			return (int) message.wParam;
		}

		return -1;
	}

	int GetScreenWidth() { return _width; }
	int GetScreenHeight() { return _height; }

	void Exit()
	{
		_gameIsRunning = false;
	}

private:
	void ReleaseResources()
	{
		// Liberar todos los recursos		
		if ( _device )				_device->Release();
		if ( _deviceContext )		_deviceContext->Release();
		if ( _swapChain )			_swapChain->Release();
		if ( _targetView )			_targetView->Release();
		if ( _depthTexture )		_depthTexture->Release();
		if ( _depthStencilView )	_depthStencilView->Release();
		if ( _depthStencilState )	_depthStencilState->Release();
		if ( _gameLevel )			delete _gameLevel;
	}

	bool InitWindowAndGraphics()
	{
		_hWND = CreateForm(g_hInstance, "SkeletonExampleWindow", "Ejemplo Skeleton", WindowProc, _width, _height);

		HRESULT hResult;

		D3D_DRIVER_TYPE d3dDriverTypes[] =
		{ D3D_DRIVER_TYPE_HARDWARE, D3D_DRIVER_TYPE_WARP, D3D_DRIVER_TYPE_SOFTWARE, D3D_DRIVER_TYPE_REFERENCE };
		int iDriverTypesCount = ARRAYSIZE(d3dDriverTypes);

		// Inicializamos los feature levels (versiones) de Direct3D
		D3D_FEATURE_LEVEL d3dFeatureLevels[] =
		{ D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_1, D3D_FEATURE_LEVEL_10_0 };
		int iFeatureLevelCount = ARRAYSIZE(d3dFeatureLevels);

		// Creamos el descriptor del swap chain
		/**
		*	Un swap chain es un conjunto de buffers en donde
		*	la tarjeta gráfica está constantemente dibujando.
		*	En videojuegos, normalmente existen dos buffers llamados
		*	front y back buffer. El front buffer es el que se nos muestra
		*	en pantalla, mientras que el back buffer es donde la tarjeta
		*	gráfica está dibujando el frame que se dibujará a continuación.
		*
		*	Un descriptor (en DirectX) es una estructura para
		*	definir cómo queremos que se inicialice un objeto.
		**/
		DXGI_SWAP_CHAIN_DESC dxgSwapChainDesc;
		ZeroMemory(&dxgSwapChainDesc, sizeof(dxgSwapChainDesc));
		dxgSwapChainDesc.BufferCount = 1;
		dxgSwapChainDesc.BufferDesc.Width = _width;
		dxgSwapChainDesc.BufferDesc.Height = _height;
		dxgSwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		dxgSwapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
		dxgSwapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
		dxgSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		dxgSwapChainDesc.OutputWindow = _hWND;
		dxgSwapChainDesc.Windowed = !_isFullscreen;
		dxgSwapChainDesc.SampleDesc.Count = 1;
		dxgSwapChainDesc.SampleDesc.Quality = 0;

		for (int i = 0; i < iDriverTypesCount; i++)
		{
			hResult = D3D11CreateDeviceAndSwapChain(
				NULL,
				d3dDriverTypes[i],
				NULL,
				NULL,
				d3dFeatureLevels,
				iFeatureLevelCount,
				D3D11_SDK_VERSION,
				&dxgSwapChainDesc,
				&_swapChain,
				&_device,
				&_featureLevel,
				&_deviceContext);

			if (SUCCEEDED(hResult))
			{
				_driverType = d3dDriverTypes[i];
				break; // El device y swap chain han sido creados
			}
		}

		if (FAILED(hResult))
			return false; // No se pudo crear el device ni el swap chain

		// Textura que nos servirá como back buffer
		ID3D11Texture2D *d3dBackBufferTexture;

		// Ligamos nuestra textura con el back buffer del swap chain
		hResult = _swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&d3dBackBufferTexture);
		if (FAILED(hResult))
			return false; // Error al ligar nuestra textura con el back buffer

		// Indicamos al device que nuestra textura es donde deberá dibujar
		hResult = _device->CreateRenderTargetView(d3dBackBufferTexture, NULL, &_targetView);
		if (d3dBackBufferTexture)
			d3dBackBufferTexture->Release();

		if (FAILED(hResult))
			return false; // No se pudo crear el render target

		// Ligamos el render target al pipe line de Direct3D
		//m_d3dDeviceContext->OMSetRenderTargets(1, &m_d3dRenderTarget, NULL);

		// Creamos el viewport
		D3D11_VIEWPORT d3dViewport;
		d3dViewport.Width = (float)_width;
		d3dViewport.Height = (float)_height;
		d3dViewport.MinDepth = 0.0f;
		d3dViewport.MaxDepth = 1.0f;
		d3dViewport.TopLeftX = 0.0f;
		d3dViewport.TopLeftY = 0.0f;

		_deviceContext->RSSetViewports(1, &d3dViewport);

		D3D11_TEXTURE2D_DESC depthTexDesc;
		ZeroMemory(&depthTexDesc, sizeof(depthTexDesc));
		depthTexDesc.Width = _width;
		depthTexDesc.Height = _height;
		depthTexDesc.MipLevels = 1;
		depthTexDesc.ArraySize = 1;
		depthTexDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthTexDesc.SampleDesc.Count = 1;
		depthTexDesc.SampleDesc.Quality = 0;
		depthTexDesc.Usage = D3D11_USAGE_DEFAULT;
		depthTexDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthTexDesc.CPUAccessFlags = 0;
		depthTexDesc.MiscFlags = 0;

		hResult = _device->CreateTexture2D(&depthTexDesc, NULL, &_depthTexture);
		if(FAILED(hResult))
		{
			MessageBox(0, "Error", "Error al crear la DepthTexture", MB_OK);
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = depthTexDesc.Format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		hResult = _device->CreateDepthStencilView(_depthTexture, &descDSV, &_depthStencilView);
		if(FAILED(hResult))
		{
			MessageBox(0, "Error", "Error al crear el depth stencil target view", MB_OK);
			return false;
		}

		// Ligamos el render target al pipe line de Direct3D
		_deviceContext->OMSetRenderTargets(1, &_targetView, _depthStencilView);

		D3D11_DEPTH_STENCIL_DESC descDSD;
		ZeroMemory(&descDSD, sizeof(descDSD));
		descDSD.DepthEnable = true;
		descDSD.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		descDSD.DepthFunc = D3D11_COMPARISON_LESS;
		descDSD.StencilEnable=true;
		descDSD.StencilReadMask = 0xFF;
		descDSD.StencilWriteMask = 0xFF;
		descDSD.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		descDSD.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		descDSD.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		descDSD.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		descDSD.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		descDSD.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		descDSD.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		descDSD.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		hResult = _device->CreateDepthStencilState(&descDSD, &_depthStencilState);
		if(FAILED(hResult))
		{
			MessageBox(0, "Error", "Error al crear el depth stencil state", MB_OK);
			return false;
		}
		_deviceContext->OMSetDepthStencilState(_depthStencilState, 1);

		return true;
	}

	void DoFrame()
	{
		/* Rutina de actualización */
		_gameLevel->Update(deltaTime);
		
		/* Rutina de dibujo */
		float clearColor[4] = { 0.5f, 0.1f, 0.9f, 1.0f };
		_deviceContext->ClearRenderTargetView( _targetView, clearColor );
		_deviceContext->ClearDepthStencilView( _depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0 );
		_gameLevel->Draw(_deviceContext);
		_swapChain->Present(1, 0);

		/* Actualización de delta time */
		DWORD currentTickTime = GetTickCount();
		deltaTime = (float)(currentTickTime - lastTickTime) / 1000.0f;
		lastTickTime = currentTickTime;
	}
};

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		PAINTSTRUCT paintStruct;
		HDC  hDC;

		case WM_PAINT:
		{
			hDC = BeginPaint(hWnd, &paintStruct);
			EndPaint(hWnd, &paintStruct);
		} break;

		case WM_DESTROY:
		{
			g_Game->Exit();
			PostQuitMessage(0);
		} break;
	}

	return DefWindowProc(hWnd, message, wParam, lParam);
}

#endif