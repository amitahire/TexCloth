/******************************************************************************
Class:MyGame
Implements:GameEntity
Author:Rich Davison	<richard.davison4@newcastle.ac.uk> and Amit Ahire(amitahire@gmail.com)

-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////



#pragma once
#include "GameEntity.h"
#include "GameClass.h"
#include "../nclgl/Camera.h"
#include "../nclgl/CubeRobot.h"
#include "FirstProjectile.h"
#include "MoveCube.h"
#include "MoveSphere.h"
#include "Cube.h"
#include "CubeBox0.h"
#include "CubeBox1.h"
#include "MoveSpherePhy.h"
#include "KPAXPhysicsNode.h"
#include "../../nclgl/HeightMap.h"
#include "../../nclgl/KPAXNode.h"

#include "Ball_Spring_Chain.h"
#include "BallNode.h"

class MyGame : public GameClass	{
public:
	MyGame();
	~MyGame(void);

	virtual void UpdateGame(float msec);

protected:
	GameEntity*		BuildRobotEntity();

	GameEntity*		BuildCubeEntity(float size);

	GameEntity*		BuildCube2(float size);

	GameEntity*		BuildSphereEntity(float radius,Vector3 force,Vector3 pos,float mass);



	MoveCube*		cubeObj;

	MoveSphere*		sphereObj;

	Mesh*			heightMap2;
	Vector3			position;

	springdemo_c*	sd;
	springdemo_c*	sd2;

	Mesh*			cube;
	Mesh*			quad;
	Mesh*			sphere;

	float			fps;
};

