#include <Windows.h>
#include "Game.h"
#include "Camera.h"

int g_nCmdShow;
HINSTANCE g_hInstance, g_hPrevInstance;
Game *g_Game;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	g_nCmdShow = nCmdShow;
	g_hInstance = hInstance;
	g_hPrevInstance = hPrevInstance;

	Game *game = new Game();
	int result = game->Run();	
	delete game;

	return result;
}