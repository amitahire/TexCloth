/******************************************************************************
Class:Renderer		
Implements:OGLRenderer
Author:Rich Davison	<richard.davison4@newcastle.ac.uk> and YOU!
Description: For this module, you are being provided with a basic working
Renderer - to give you more time to work on your physics and AI!

It is basically the Renderer from the Graphics For Games Module as it was
by Tutorial 7 - Scene Management. It will split nodes up into those that are
opaque and transparent, and render accordingly. 

The only new bits are the ability to add and remove scene nodes, and set the
camera - as these are now to be controlled by your 'game'.

The renderer is one of those things that really there should be only one of.
I think it was a mistake to add support for multiple Windows / Renderers to
the code download, as it just overcomplicated things. To this end, The 
Renderer for this module is a singleton-like class, as with the 
PhysicsSystem. Thus, it is created, accessed, and destroyed via static 
functions. You don't have to do it this way if you don't want, and you are
of course free to add in the advanced graphical features from other tutorials
into this Renderer as you see fit. 


-_-_-_-_-_-_-_,------,   
_-_-_-_-_-_-_-|   /\_/\   NYANYANYAN
-_-_-_-_-_-_-~|__( ^ .^) /
_-_-_-_-_-_-_-""  ""   

*//////////////////////////////////////////////////////////////////////////////


#pragma once

#include "../../nclgl/OGLRenderer.h"
#include "../../nclgl/SceneNode.h"
#include "../../nclgl/Camera.h"
#include "../../nclgl/Frustum.h"
#include "../../nclgl/TextMesh.h"
#include "../../nclgl/HeightMap.h"
#include "../../nclgl/SeaMap.h"
#include "../../nclgl/ParticleEmitter.h"
#include "../../nclgl/OBJMesh.h" //Could be a problem later on. 

#include "PhysicsNode.h"
#include <algorithm>
#include "Cloth.h"
#include "../../nclgl/LightNode.h"


class Renderer : public OGLRenderer	{
public:
	virtual void RenderScene();
	virtual void UpdateScene(float msec);

	//Draw Text - FPs counter.
	float fps;
	float renderTime;
	void DrawText(const std::string &text, const Vector3 &position, const float size, const bool perspective = false);


	void	SetCamera(Camera*c);

	void	AddNode(SceneNode* n);

	void	RemoveNode(SceneNode* n);

	//Statics
	static bool Initialise() {
		instance = new Renderer(Window::GetWindow());
		return instance->HasInitialised();
	}

	static void Destroy() {
		delete instance;
	}
	
	static Renderer& GetRenderer() { return *instance;}

//protected:	
	Renderer(Window &parent);
	virtual ~Renderer(void);

	void	init_Shader();
	void	init_Components();

	void	init_Root();
	void	init_Cloth();
	void	init_Skybox();
	void	init_HeightMap();

	//Deffered Rendering
	void	init_Object_DR_Lights();
	void	init_Object_DR_Buffer();

	void	init_ParticleEmitter();
	void	init_Font();

	//Back.
	void	init_Light();
		Light*		sharedLight;

	void	init_Enable();
	void	init_Matrix();
	void	init_Others();
 
	//SceneBuffer
	void	init_SceneBuffer();
		GLuint SceneFBO;
		GLuint SceneBufferColourTex;
		GLuint SceneBufferDepthTex;

	void	init_DamageBuffer();
		GLuint DamageFBO;
		GLuint DamageBufferDepthTex;
		GLuint DamageBufferColourTex[2];
		Shader* damageShader;


	void	BuildNodeLists(SceneNode* from);
	void	SortNodeLists();
	void	ClearNodeLists();
	void	DrawCloth();
	void	DrawNodes();
	void	DrawNode(SceneNode*n);
	void Debug_Light_Draw(bool flip);
	Shader* debugLightShader;
		Frustum frameFrustum;

	//Creating Node Robot
	SceneNode *root;


	void	DrawSkybox();
		Shader* skyboxShader;
		GLuint cubeMap;
		Mesh* skybox;

	void DrawHeightmap(Shader* shaderToUse);
		HeightMap* heightMap;
		float heightMap_zoom;
		Shader* heightMapShader;

	Camera*		camera;
	

	Shader*		spotLightShader;

	OBJMesh*	sphereDeform;

	Cloth*		cloth;

	//Set Font - Set FPS
	void DrawText();
		Font* basicFont;
		Shader* fontShader;
		float fpsValue;
		int fpsReloadSpeed;
		int fpsSmoothCounter;
		float fpsBank;
		int fpsCounter;

	//Set Particle Effect
	void DrawParticle();
	void SetShaderParticleSize(float f);
		ParticleEmitter* emitter[25];
		Shader* particleShader;


	//DF stuff.
	void GenerateScreenTexture ( GLuint &into , bool depth = false );
		Shader * sceneShader ;// Shader to fill our GBuffers
		Shader * pointlightShader ;// Shader to calculate lighting
		Shader * combineShader ; // shader to stick it all together
		OBJMesh * lightSphere ; // Light volume
		Mesh * fullScreenQuad ; // To draw a full - screen quad
		float rotation;
		GLuint bufferFBO ; // FBO for our G- Buffer pass
		GLuint bufferColourTex ;
		GLuint bufferNormalTex ; // Normals go here
		GLuint bufferDepthTex ; // Depth goes here

		GLuint pointLightFBO ;// FBO for our lighting pass
		GLuint lightEmissiveTex ; // Store emissive lighting
		GLuint lightSpecularTex ; // Store specular lighting

	Shader* processShader;
	void	FillDamageBuffer();
	void	FillSceneBuffer();
	void	FillDRBuffers();
	// Lighting Render Pass
	void	DrawPointLights();
	// Combination Render Pass
	void	CombineBuffers ();

	void	SwitchToPerspective(float zoom);
	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
	vector<Light*> lightList;

	Shader* DR_heightMapShader;
	bool torch;

	static Renderer*	instance;
	float clock;



};

