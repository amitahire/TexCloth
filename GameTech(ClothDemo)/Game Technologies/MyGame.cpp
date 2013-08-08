#include "MyGame.h"
#include "../../nclgl/HeightMap.h"
#include <time.h>


MyGame::MyGame()	{

	srand((unsigned int)time(NULL));

	gameCamera = new Camera(-30.0f,0.0f,Vector3(0,450,850));

	Renderer::GetRenderer().SetCamera(gameCamera);

	
	quad			= Mesh::GenerateQuad();
	sphere			= new OBJMesh("../../Meshes/ico.obj");


}

MyGame::~MyGame(void)	{
	/*
	We're done with our assets now, so we can delete them
	*/

	delete quad;
	delete sphere;
}


GameEntity* MyGame::BuildSphereEntity(float radius,Vector3 force,Vector3 pos,float mass){

	float r = (float)(rand ()%101) / 100.0f;
	float g = (float)(rand ()%101) / 100.0f;
	float b = (float)(rand ()%101) / 100.0f;

	LightNode* s = new LightNode(sphere,Vector4(r,g,b,0.5));
	s->drawSelf = true;
	s->nodeLight->SetRadius(radius*4);
	s->nodeLight->SetColour(Vector4(r,g,b,1));

	s->SetModelScale(Vector3(radius,radius,radius));
	s->SetBoundingRadius(radius);
	
	BallNode *c = new BallNode(Quaternion::AxisAngleToQuaterion(Vector3(1,0,0), 90.0f), pos);
	c -> SetInverseMass(mass);	
	c -> SetColliRadius(radius);
	c -> m_force = force;
	GameEntity* gameE = new GameEntity(s,c);
	gameE->ConnectToSystems();


	return gameE;


}


/*
Here's the base 'skeleton' of your game update loop! You will presumably
want your games to have some sort of internal logic to them, and this
logic will be added to this function.
*/
void MyGame::UpdateGame(float msec) {

	//////////////////////////////////////////////////////
	if(gameCamera) {
		gameCamera->UpdateCamera(msec);
		//CamPos = gameCamera->GetPosition();
	}

	for(vector<GameEntity*>::iterator i = allEntities.begin(); i != allEntities.end(); ++i) {
		(*i)->Update(msec);
	}


	// Reference Lines - X, Y and Z.
	Renderer::GetRenderer().DrawDebugLine(DEBUGDRAW_PERSPECTIVE, Vector3(0,10000,0),Vector3(0,-10000,0), Vector3(0,0,1), Vector3(0,0,1));
	Renderer::GetRenderer().DrawDebugLine(DEBUGDRAW_PERSPECTIVE, Vector3(10000,0,0),Vector3(-10000,0,0), Vector3(0,1,0), Vector3(0,1,0));
	Renderer::GetRenderer().DrawDebugLine(DEBUGDRAW_PERSPECTIVE, Vector3(0,0,10000),Vector3(0,0,-10000), Vector3(1,0,0), Vector3(1,0,0));
}






