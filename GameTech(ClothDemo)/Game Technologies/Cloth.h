#pragma once
#include "../../nclgl/Mesh.h"
#include "../../nclgl/Vector3.h"
#include <math.h>
#include <vector>

#include "PhysicsNode.h"
#include "PhysicsSystem.h"
using std::vector;

const float DT = 0.1f; //Time Slice
const int GravityOn = 1; //0 = off, 1 = on
const bool fixedPoint = true;
const bool fixedPoint_Flag = true;

//Cloth Physics 
const float G = -3.0f; 
const float KS = 30.0f; //30
const float KD = 0.3f; //0.3
const float Cloth_Mass = 1.0f;

//Define number of point in the cloth
const int CLOTH_WIDTH = 30;
const int CLOTH_HEIGHT = 30;
const int CLOTH_MAXNUMPOINT = CLOTH_WIDTH*CLOTH_HEIGHT;
const int clothScale = 30;
const float particleApart_Width =  (clothScale*CLOTH_WIDTH)/(CLOTH_WIDTH-1);
const float particleApart_Height =  (clothScale*CLOTH_HEIGHT)/(CLOTH_HEIGHT-1);

//Texture Coordinates.
const float clothTexture_Width = 1.0f / CLOTH_WIDTH;
const float clothTexture_Height = 1.0f / CLOTH_HEIGHT;


//Initial World Position
const float clothSize_Width = CLOTH_WIDTH*clothScale;
const float clothSize_Height = CLOTH_HEIGHT*clothScale;

class Cloth: public Mesh {
	
	PhysicsNode** pNode;	
	vector<PhysicsNode*> particleList;
	PhysicsNode* physicsBound;

	Vector3 maxXYZ;
	Vector3 minXYZ;

	Vector3 clothPos;



public :	

	Cloth(Vector3 worldPosSet);
	void			RecalculatePosition();
	void			ReBufferValue();
	void			RebufferNormal();
	void			SetFixPoint();
	void			ResetClothPoint();
	void			RebondIndices();
	void			SetBoundingBox(Vector3 position);
	void			AddForce(int i);
	virtual void	Draw();

	~Cloth(void){
		for(int i = 0; i < CLOTH_HEIGHT;i++){
			delete [] pNode[i];
		}
		delete [] pNode;
	};
};