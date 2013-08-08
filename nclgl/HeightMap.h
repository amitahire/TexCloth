#pragma once;

#include <string>
#include <iostream>
#include <fstream>

#include "../nclgl/Mesh.h"
#include "../GameTech(ClothDemo)/Game Technologies/PhysicsNode.h"


#define RAW_WIDTH 257
#define RAW_HEIGHT 257

#define HEIGHTMAP_X 16.0f
#define HEIGHTMAP_Z 16.0f
#define HEIGHTMAP_Y 1.25f
#define HEIGHTMAP_TEX_X 1.0f/16.0f
#define HEIGHTMAP_TEX_Z 1.0f/16.0f

//class HeightMap : public Mesh {
//
//public:
//	HeightMap(std::string name);
//	~HeightMap(void){};
//
//	virtual void Draw();
//
//	void SetTextureLayer(GLuint tex,int i){textureLayer[i] = tex;}
//	GLuint GetTextureLayer(int i) {return textureLayer[i];}
//
//private:
//		GLuint textureLayer[5]; // Different textures layers.
//
//};

class HeightMap : public Mesh {
public :
	HeightMap(std::string name);
	~ HeightMap ( void ){ 
		glDeleteTextures(5,textureLayer);
		//delete mapTree;
		//for(int i = 0; i < RAW_HEIGHT;i++){
		//	delete [] particleNode[i];
		//}
		//delete [] particleNode;
	};	
	
	////Collision Detection
	//PhysicsNode** particleNode;	//2D Array of Particle
	//vector<PhysicsNode*> particleList; //Pointer of All Nodes
	//PhysicsNode* physicsBound; //HeightMap Bounding Box
	//OctTree* mapTree; //HeightMap Structured Collision Tree

	virtual void Draw();
	//Getter and Setter of Texture Layer (Use for multiTexturing)
	/* textureLayer[0] == Top of the mountain */
	/* textureLayer[1] == Middle of the mountain */
	/* textureLayer[2] == Bottom of the mountain */
	/* textureLayer[3] == Seabed of the mountain */
	/* textureLayer[4] == BumpTex for middle*/
	void SetTextureLayer(GLuint tex,int i){textureLayer[i] = tex;}
	GLuint GetTextureLayer(int i) {return textureLayer[i];}
private: 
	GLuint textureLayer[5];
};


