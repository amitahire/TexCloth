/******************************************************************************
Class: Main.cpp
Implements: Start the context and the whole program.
Author:Rich Davison	<richard.davison4@newcastle.ac.uk> and Amit Ahire<amitahire@gmail.com>


-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////
#include "../../nclgl/Window.h"
#include "MyGame.h"

#pragma comment(lib, "nclgl.lib")

int Quit(bool pause = false, const string &reason = "") {
	PhysicsSystem::Destroy();
	Window::Destroy();
	Renderer::Destroy();

	if(pause) {
		std::cout << reason << std::endl;
		system("PAUSE");
	}

	return 0;
}

int main() {
	float fps = 0;

	PhysicsSystem::Initialise();

	if(!Window::Initialise("Game Technologies", 1500,900,false)) {
		return Quit(true, "Window failed to initialise!");
	}

	if(!Renderer::Initialise()) {
		return Quit(true, "Renderer failed to initialise!");
	}



	MyGame* game = new MyGame();

	Window::GetWindow().LockMouseToWindow(true);
	Window::GetWindow().ShowOSPointer(false);

	while(Window::GetWindow().UpdateWindow() && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		float msec = Window::GetWindow().GetTimer()->GetTimedMS();	

		game->UpdateCore(msec);	
		game->UpdateGame(msec);	
	}

	delete game;
	return Quit();
}

