#include "Renderer.h"
#include <math.h>
#include <sstream>


Renderer* Renderer::instance = NULL;


///////////////////////////////////////////////
////1. Construction and Destructor.			///
///////////////////////////////////////////////
Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{	

	instance = this;
	init_Shader();
	init_Components();
	//init_Camera();		
	init_Light();
	init_Enable();
	init_Matrix();
	init_Others();
	init = true;
	
}

Renderer::~Renderer(void)	{

	//Deleting Shader
	delete heightMapShader;		
	delete skyboxShader;
	delete particleShader;	
	delete fontShader;
	delete sceneShader;
	delete DR_heightMapShader;
	delete combineShader;
	delete pointlightShader;
	currentShader = 0;

	//Delete Components
	delete root;

	delete skybox;
	for(int i = 0; i < 25; i++){
		delete emitter[i];
	}

	delete fullScreenQuad;
	delete lightSphere;	
		
	glDeleteTextures(1,&bufferColourTex);
	glDeleteTextures(1,&bufferNormalTex);
	glDeleteTextures(1,&bufferDepthTex);
	glDeleteTextures(1,&lightEmissiveTex);
	glDeleteTextures(1,&lightSpecularTex);	
	glDeleteFramebuffers(1,&bufferFBO);
	glDeleteFramebuffers(1,&pointLightFBO);
	

	delete sharedLight;

	//2e.Delete_SceneBuffer;
	glDeleteTextures (1, &SceneBufferColourTex );
	glDeleteTextures (1 ,&SceneBufferDepthTex );
	glDeleteFramebuffers (1 ,&SceneFBO );
		
	
	//2e.Delete_SceneBuffer;
	glDeleteTextures (2, DamageBufferColourTex);	
	glDeleteTextures (1 ,&DamageBufferDepthTex );
	glDeleteFramebuffers (1 ,&DamageFBO );

	delete sphereDeform;

}


//////////////////////////////////////////////////////////////////////////
/////////2. Init														//
//////////////////////////////////////////////////////////////////////////

void Renderer::init_Shader() {

	//For Heightmap
	heightMapShader = new Shader(SHADERDIR"heightMap_Vertex.glsl",SHADERDIR"heightMap_Fragment.glsl");
	if(!heightMapShader->LinkProgram()) { return; }
	
	DR_heightMapShader = new Shader(SHADERDIR"heightMap_Vertex.glsl",SHADERDIR"heightMap_Fragment_withDR.glsl");
	if(!DR_heightMapShader->LinkProgram()) { return; }


	//For Skybox
	skyboxShader = new Shader (SHADERDIR"skyboxVertex.glsl",SHADERDIR"skyboxFragment.glsl");
	if(!skyboxShader->LinkProgram()) { return; }
	

	//For Particle
	particleShader = new Shader(SHADERDIR"vertex.glsl",SHADERDIR"fragment.glsl",SHADERDIR"geometry.glsl");
	if(!particleShader->LinkProgram()) { return; }

	//For Font
	fontShader = new Shader(SHADERDIR"TexturedVertex.glsl", SHADERDIR"TexturedFragment.glsl");
	if(!fontShader->LinkProgram()) { return; }

	//For Deferred Rendering
	sceneShader = new Shader(SHADERDIR"bumpvertex.glsl",SHADERDIR"bufferFragment.glsl");
	if (!sceneShader->LinkProgram()) { return; }

	combineShader = new Shader (SHADERDIR"combinevert.glsl",SHADERDIR"combinefrag.glsl");
	if (!combineShader->LinkProgram()) { return; }

	pointlightShader = new Shader (SHADERDIR"pointlightvertex.glsl",SHADERDIR"pointlightfragment.glsl");
	if (!pointlightShader -> LinkProgram ()) { return; }	

	spotLightShader = new Shader (SHADERDIR"pointlightvertex.glsl",SHADERDIR"spotlightfragment.glsl");
	if (!spotLightShader -> LinkProgram ()) { return; }

	//For Map Deformation
	damageShader = new Shader (SHADERDIR"basicVertex.glsl",SHADERDIR"basicFrag.glsl");
	if (!damageShader -> LinkProgram ()) { return; }



}

void Renderer::init_Components() {

	// Init Root.
	init_Root();

	// Init World Components.
	init_HeightMap();

	init_Skybox();
	init_Cloth();

	// Init Deffered Rendering.
	init_Object_DR_Lights();
	init_Object_DR_Buffer();
	
	init_ParticleEmitter();
	init_Font();

	//Init_SceneBuffer
	init_SceneBuffer();
	init_DamageBuffer();

}

void Renderer::init_Light() {
	//Light.
	//light			= new Light(Vector3((RAW_HEIGHT * HEIGHT_X / 2.0f), 500.0f, (RAW_HEIGHT * HEIGHT_Z / 2.0f)), Vector4(1,1,1,1),(RAW_WIDTH * HEIGHT_X)* 2.0f);
	//light			= new Light(Vector3( 500.0f, (RAW_HEIGHT * HEIGHT_Z / 2.0f), (RAW_HEIGHT * HEIGHT_X / 2.0f)), Vector4(1,1,1,1),(RAW_WIDTH * HEIGHT_X)* 2.0f);
	//sharedLight			= new Light ( Vector3 (750.0f ,1500.0f ,4050.0f ),Vector4 (1,1,1,1.0) , ( RAW_WIDTH * HEIGHT_X ) / 0.05f );
	sharedLight			= new Light ( Vector3 (7770.0f ,9570.0f ,600.0f ),Vector4 (1,1,1,1.0) , ( RAW_WIDTH * HEIGHT_X ) / 0.05f );


}




void Renderer::init_Root() {
	root		= new SceneNode();
}

void Renderer::init_Cloth() {

	cloth = new Cloth(Vector3(1596,900,1596));
	cloth -> SetTexture ( SOIL_load_OGL_texture (TEXTUREDIR"chequred_flag_1.jpg",SOIL_LOAD_AUTO , SOIL_CREATE_NEW_ID , SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS ));	
	cloth -> SetBumpMap ( SOIL_load_OGL_texture (TEXTUREDIR"grassbump.jpg",SOIL_LOAD_AUTO , SOIL_CREATE_NEW_ID , SOIL_FLAG_MIPMAPS | SOIL_FLAG_TEXTURE_REPEATS));
	if (!cloth->GetTexture()) {return ;}
}

void Renderer::init_HeightMap() {
	heightMap = new HeightMap(TEXTUREDIR"terrain.raw");
	//Setup Map Texture:
	//Setup BumpMap Texture:
	heightMap -> SetBumpMap ( SOIL_load_OGL_texture (TEXTUREDIR"Barren RedsDOT3.jpg", SOIL_LOAD_AUTO ,SOIL_CREATE_NEW_ID,SOIL_FLAG_MIPMAPS));
	heightMap->SetTextureLayer( SOIL_load_OGL_texture (TEXTUREDIR"snow.jpg",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_MIPMAPS),0);
	heightMap->SetTextureLayer( SOIL_load_OGL_texture (TEXTUREDIR"grassHill.jpg",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_MIPMAPS),1);
	heightMap->SetTextureLayer( SOIL_load_OGL_texture (TEXTUREDIR"paper.jpg",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_MIPMAPS),2);
	heightMap->SetTextureLayer( SOIL_load_OGL_texture (TEXTUREDIR"seabed.jpg",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_MIPMAPS),3);
	heightMap->SetTextureLayer( SOIL_load_OGL_texture (TEXTUREDIR"grassbump.jpg",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_MIPMAPS),4);

	for(int i = 0; i < 5; i++){ if(!heightMap->GetTextureLayer(i)) { return;} }
	if(!heightMap->GetBumpMap()){ return;}

	SetTextureRepeating(heightMap->GetBumpMap(),true);
	for(int i = 0; i < 5; i++){ SetTextureRepeating(heightMap->GetTextureLayer(i),true); }
}

void Renderer::init_Skybox() {

	skybox			= Mesh::GenerateQuad();
	skybox->SetTexture(SOIL_load_OGL_texture(TEXTUREDIR"water.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS));

	cubeMap			= SOIL_load_OGL_cubemap(
											TEXTUREDIR"Thuliac_Night_West.bmp",TEXTUREDIR"Thuliac_Night_East.bmp", 
											TEXTUREDIR"Thuliac_Night_Top.bmp", TEXTUREDIR"Thuliac_Night_Bottom.bmp",
											TEXTUREDIR"Thuliac_Night_North.bmp", TEXTUREDIR"Thuliac_Night_South.bmp", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	
	if(!cubeMap || !skybox->GetTexture()) {
		return;
	}
	SetTextureRepeating(skybox->GetTexture(), true);

}

void Renderer::init_Object_DR_Lights() {

	//Init Light Shape
	lightSphere = new OBJMesh ();
	if (! lightSphere -> LoadOBJMesh (MESHDIR"sphere_t.obj")) { return ; }

}

void Renderer::init_Object_DR_Buffer() {

	//Init Quad for display
	fullScreenQuad = Mesh::GenerateQuad();

	//Generate FBO's
	glGenFramebuffers (1 ,&bufferFBO);
	glGenFramebuffers (1 ,&pointLightFBO);
	
	GLenum buffers[2];
	buffers[0] = GL_COLOR_ATTACHMENT0;
	buffers[1] = GL_COLOR_ATTACHMENT1;
	
	//Generate Texture for Buffer Use
	GenerateScreenTexture(bufferDepthTex,true);
	GenerateScreenTexture(bufferColourTex);
	GenerateScreenTexture(bufferNormalTex);
	GenerateScreenTexture(lightEmissiveTex);
	GenerateScreenTexture(lightSpecularTex);

	//First Pass Buffer - From Richard Tutorial.
	glBindFramebuffer(GL_FRAMEBUFFER , bufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,bufferColourTex,0);
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,bufferNormalTex,0);
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_DEPTH_ATTACHMENT,GL_TEXTURE_2D,bufferDepthTex,0);
	glDrawBuffers(2,buffers);	
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE){ return;}
	
	//Second Pass Buffer
	glBindFramebuffer(GL_FRAMEBUFFER,pointLightFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,lightEmissiveTex,0);
	glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT1,GL_TEXTURE_2D,lightSpecularTex,0);
	glDrawBuffers(2,buffers);
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE) { return;}

	glBindFramebuffer(GL_FRAMEBUFFER,0);


}

void Renderer::GenerateScreenTexture(GLuint & into,bool isDepth) {
	glGenTextures(1,&into);
	glBindTexture(GL_TEXTURE_2D,into);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D,0,isDepth?GL_DEPTH_COMPONENT24:GL_RGBA8,width,height,0,isDepth?GL_DEPTH_COMPONENT:GL_RGBA,GL_UNSIGNED_BYTE,NULL);
	glBindTexture(GL_TEXTURE_2D,0);
}

void Renderer::init_ParticleEmitter(){
	for(int i = 0; i < 25; i++){ emitter[i] = new ParticleEmitter(); }
}

void Renderer::init_Font() {
	basicFont = new Font(SOIL_load_OGL_texture(TEXTUREDIR"tahoma.tga",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_COMPRESS_TO_DXT),16,16);
	//basicFont = new Font(SOIL_load_OGL_texture(TEXTUREDIR"tahoma-edited2.tga",SOIL_LOAD_AUTO,SOIL_CREATE_NEW_ID,SOIL_FLAG_COMPRESS_TO_DXT),16,16);	
}


void Renderer::init_SceneBuffer() {
	// Generate our scene depth texture ...
	glGenTextures (1, & SceneBufferDepthTex );
	glBindTexture ( GL_TEXTURE_2D , SceneBufferDepthTex );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_NEAREST );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_NEAREST );
	glTexImage2D ( GL_TEXTURE_2D , 0, GL_DEPTH24_STENCIL8 , width , height ,0, GL_DEPTH_STENCIL , GL_UNSIGNED_INT_24_8 , NULL );

	glGenTextures (1, & SceneBufferColourTex);
	glBindTexture ( GL_TEXTURE_2D , SceneBufferColourTex);
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_NEAREST );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_NEAREST );
	glTexImage2D ( GL_TEXTURE_2D , 0, GL_RGBA8 , width , height , 0, GL_RGBA , GL_UNSIGNED_BYTE , NULL );
	
	glGenFramebuffers (1 ,& SceneFBO ); // We 'll render the scene into this
	glBindFramebuffer ( GL_FRAMEBUFFER , SceneFBO );
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT ,GL_TEXTURE_2D , SceneBufferDepthTex , 0);
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_STENCIL_ATTACHMENT ,GL_TEXTURE_2D , SceneBufferDepthTex , 0);
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 ,GL_TEXTURE_2D , SceneBufferColourTex, 0);
	// We can check FBO attachment success using this command !
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE||!SceneBufferDepthTex||!SceneBufferColourTex) {return ;}
	glBindFramebuffer ( GL_FRAMEBUFFER , 0);

}

void Renderer::init_DamageBuffer() {
		// Generate our scene depth texture ...
	glGenTextures (1, & DamageBufferDepthTex );
	glBindTexture ( GL_TEXTURE_2D , DamageBufferDepthTex );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_NEAREST );
	glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_NEAREST );
	glTexImage2D ( GL_TEXTURE_2D , 0, GL_DEPTH24_STENCIL8 , width , height ,0, GL_DEPTH_STENCIL , GL_UNSIGNED_INT_24_8 , NULL );

	for(int i = 0; i < 2;i++){
		glGenTextures (1, & DamageBufferColourTex[i]);
		glBindTexture ( GL_TEXTURE_2D , DamageBufferColourTex[i]);
		glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_S , GL_CLAMP );
		glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_WRAP_T , GL_CLAMP );
		glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MAG_FILTER , GL_NEAREST );
		glTexParameterf ( GL_TEXTURE_2D , GL_TEXTURE_MIN_FILTER , GL_NEAREST );
		glTexImage2D ( GL_TEXTURE_2D , 0, GL_RGBA8 , width , height , 0, GL_RGBA , GL_UNSIGNED_BYTE , NULL );
	}
	glGenFramebuffers (1 ,& DamageFBO ); // We 'll render the scene into this
	glBindFramebuffer ( GL_FRAMEBUFFER , DamageFBO );
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_DEPTH_ATTACHMENT ,GL_TEXTURE_2D , DamageBufferDepthTex , 0);
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_STENCIL_ATTACHMENT ,GL_TEXTURE_2D , DamageBufferDepthTex , 0);
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT0 ,GL_TEXTURE_2D , DamageBufferColourTex[0], 0);
	
	glFramebufferTexture2D ( GL_FRAMEBUFFER , GL_COLOR_ATTACHMENT1 ,GL_TEXTURE_2D , DamageBufferColourTex[1], 0);
	// We can check FBO attachment success using this command !
	if(glCheckFramebufferStatus(GL_FRAMEBUFFER)!=GL_FRAMEBUFFER_COMPLETE||!DamageBufferDepthTex||!DamageBufferColourTex[0]||!DamageBufferColourTex[1]) {return ;}
	glBindFramebuffer ( GL_FRAMEBUFFER , 0);
}


void Renderer::init_Enable() {	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_BLEND);

}


void Renderer::init_Matrix() {
	projMatrix = Matrix4::Perspective(1.0f,80000.0f,(float)width/(float)height,45.0f);
	modelMatrix.ToIdentity();
	viewMatrix.ToIdentity();
}

void Renderer::init_Others() {

	instance		= this;
	clock = 20.0f;
	heightMap_zoom = 70.0f;
	fps = 0.0f;
	renderTime = 0.0f;
	torch = false;

	sphereDeform = new OBJMesh();
	sphereDeform ->LoadOBJMesh(MESHDIR"ico.obj");
	
	
}


//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void	Renderer::RenderScene()	{

	//using Framebuffers.
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	BuildNodeLists(root);

	SortNodeLists();

	FillSceneBuffer();

	FillDRBuffers();

	DrawPointLights();

	CombineBuffers();

	SwapBuffers();

	ClearNodeLists();

}


void	Renderer::UpdateScene(float msec)	{
	
	//Variable Updates.
	fpsValue = 1000.0f/ msec;

	camera->UpdateCamera(msec);

	rotation = msec * 0.01f;	

	viewMatrix = camera -> BuildViewMatrix();

	frameFrustum.FromMatrix(projMatrix*viewMatrix);	

	for(int i = 0; i < 25; i++){ emitter[i] -> Update(msec); }

	root -> Update ( msec );

	if( Window :: GetKeyboard () -> KeyDown ( KEYBOARD_K )) {
		cloth->AddForce(0);
	}
	if( Window :: GetKeyboard () -> KeyDown ( KEYBOARD_J )) {
		cloth->AddForce(3);
	}
	
	if( Window :: GetKeyboard () -> KeyDown ( KEYBOARD_T )) {
		torch = (torch == false) ? true : false;

	}
	
}



void	Renderer::DrawCloth() {
	SetCurrentShader (sceneShader);
	glUniform3fv ( glGetUniformLocation ( currentShader -> GetProgram () ,"cameraPos") ,1 ,( float *)& camera -> GetPosition ());
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram () ,"diffuseTex") , 0);
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram () ,"bumpTex") , 1);
	UpdateShaderMatrices ();
	
	if(Window::GetKeyboard()->KeyDown(KEYBOARD_R)) {
		cloth->ResetClothPoint();
	} else {
		cloth -> Draw();		
	}


	modelMatrix.ToIdentity();
	glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA );	
	glUseProgram (0);	
	glDisable(GL_CULL_FACE);
}

void	Renderer::DrawNode(SceneNode*n)	{
	if(n->GetMesh()) {
		glUniformMatrix4fv(glGetUniformLocation(currentShader->GetProgram(), "modelMatrix"),	1,false, (float*)&(n->GetWorldTransform()*Matrix4::Scale(n->GetModelScale())));
		glUniform4fv(glGetUniformLocation(currentShader->GetProgram(), "nodeColour"),1,(float*)&n->GetColour());
		//glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "useTexture"), (int)n->GetMesh()->GetTexture());
		n->Draw(*this);
	}
}

void	Renderer::BuildNodeLists(SceneNode* from)	{
	if(frameFrustum.InsideFrustum (* from )) {
		Vector3 dir = from -> GetWorldTransform().GetPositionVector() - (camera -> GetPosition()); 
		
		from -> SetCameraDistance(Vector3::Dot(dir,dir));
		if(from -> GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		} else {
			nodeList.push_back(from);
		}
		
		for ( vector<SceneNode *>::const_iterator i = from -> GetChildIteratorStart(); i != from -> GetChildIteratorEnd(); ++i) {
			BuildNodeLists ((* i ));
		}
	}
	if(!dynamic_cast<LightNode*>(from) == NULL){
			LightNode* temp = (LightNode*)from;
			temp->nodeLight->SetPosition(Vector3(temp->GetWorldTransform().GetPositionVector().x,temp->GetWorldTransform().GetPositionVector().y,temp->GetWorldTransform().GetPositionVector().z));
			lightList.push_back(temp->nodeLight);
			if(temp->drawSelf == true){			
				nodeList.push_back(temp);
			}
	}
}

void	Renderer::DrawNodes()	 {

	for(vector<SceneNode*>::const_iterator i = nodeList.begin(); i != nodeList.end(); ++i ) {
		DrawNode((*i));
	}

	for(vector<SceneNode*>::const_reverse_iterator i = transparentNodeList.rbegin(); i != transparentNodeList.rend(); ++i ) {
		DrawNode((*i));
	}
}

void	Renderer::SortNodeLists()	{
	std::sort(transparentNodeList.begin(),	transparentNodeList.end(),	SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(),				nodeList.end(),				SceneNode::CompareByCameraDistance);
}

void	Renderer::ClearNodeLists()	{
	lightList.clear();
	transparentNodeList.clear();
	nodeList.clear();
	//Clearing.
}

void	Renderer::SetCamera(Camera*c) {
	camera = c;
	camera->SetPosition(Vector3(2030.0f, 1450.0f, 1120.0f));
	camera->SetPitch(-1.0f);
	camera->SetYaw(180.0f);
}

void	Renderer::AddNode(SceneNode* n) {
	root->AddChild(n);
}

void	Renderer::RemoveNode(SceneNode* n) {
	root->RemoveChild(n);
}

//////////////Object draw//////////////////////////////////

void	Renderer::DrawSkybox(){	

	SwitchToPerspective(55);
	glDepthMask ( GL_FALSE );
	SetCurrentShader (skyboxShader);	
	UpdateShaderMatrices();
	skybox -> Draw();
	glUseProgram (0);
	glDepthMask ( GL_TRUE );	
	SwitchToPerspective(heightMap_zoom);
}

void	Renderer::DrawHeightmap(Shader* shaderToUse){	
	glDisable(GL_BLEND);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);	
	SetCurrentShader (shaderToUse);	
	SetShaderLight (*sharedLight);
	
	glUniform3fv ( glGetUniformLocation ( currentShader -> GetProgram (),"cameraPos"),1,( float *)& camera -> GetPosition ());
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"bumpTex"), 1);	
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"snowTex"), 2);	
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"grassTex"), 3);	
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"sandTex"), 4);	
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"seaBedTex"), 5);	
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"bumpGrassTex"), 6);

	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"damageTex") , 7);
	glActiveTexture ( GL_TEXTURE7 );
	glBindTexture ( GL_TEXTURE_2D , DamageBufferColourTex[0]);

	modelMatrix.ToIdentity();
	SwitchToPerspective(heightMap_zoom);
	UpdateShaderMatrices();		
	heightMap -> Draw();	
	textureMatrix.ToIdentity();	
	glUseProgram(0);
	glDisable(GL_CULL_FACE);
	glEnable(GL_BLEND);
}

void Renderer::DrawParticle(){
	modelMatrix = Matrix4::Translation(Vector3(0,1500.0f,0));
	SetCurrentShader(particleShader);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);
	glBlendFunc(GL_ONE,GL_ONE );
	int i = 0;
	for(int j = 0; j < 5; j++){
		for(int k = 0; k < 5; k++){
		emitter[i]->SetParticleSize(30.0f);
		emitter[i]->SetParticleVariance(50.0f);
		emitter[i]->SetLaunchParticles(1.0f);
		emitter[i]->SetParticleLifetime(15000.0f);
		emitter[i]->SetParticleSpeed(0.005f);
		SetShaderParticleSize(emitter[i]->GetParticleSize());
		float block = (((RAW_WIDTH-1)*HEIGHTMAP_X)/4);
		modelMatrix = Matrix4::Translation(Vector3(j*block,4500.0f,k*block));
		UpdateShaderMatrices();	
		glDepthMask(false);
		emitter[i]->Draw();
		glDepthMask(true);
		i++;
		}
	}	
	glBlendFunc ( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
	modelMatrix.ToIdentity();
	glUseProgram(0);
}

void Renderer::SetShaderParticleSize(float f) {
	glUniform1f(glGetUniformLocation(currentShader->GetProgram(),"particleSize"), f);
}


void Renderer::DrawText(){
	glBlendFunc(GL_ONE,GL_ZERO);
	SetCurrentShader(fontShader);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram(), "diffuseTex"), 0);

	std::ostringstream stringStream;
	stringStream << "FPS:" << this->GetRenderer().fps;
	std::string rendererFPS(stringStream.str());
	DrawText(rendererFPS, Vector3(0,height-120,0), 20.0f,false);

	std::ostringstream stringStream1;
	stringStream1<< "Press \"T\" for Torch/Spotlight";
	std::string torchInfo(stringStream1.str());
	DrawText(torchInfo, Vector3(0,height-90,0), 20.0f,false);

	std::ostringstream stringStream2;
	stringStream2<< "Press \"R\" redraw cloth.";
	std::string redrawInfo(stringStream2.str());
	DrawText(redrawInfo, Vector3(0,height-60,0), 20.0f,false);

	std::ostringstream stringStream3;
	stringStream3<< "Press \"K\" or \"J\" to apply different forces.";
	std::string forceInfo(stringStream3.str());
	DrawText(forceInfo, Vector3(0,height-30,0), 20.0f,false);

	std::ostringstream stringStream4;
	stringStream4<< "Press \"A\" and \"D\" to move left and right. \n" << endl;
	std::string controls(stringStream4.str());
	DrawText(controls, Vector3(0,height-850,0), 16.0f,false);

	std::ostringstream stringStream5;
	stringStream5<< "Press \"W\" and \"S\" to move forward and back. \n" << endl;
	std::string controls1(stringStream5.str());
	DrawText(controls1, Vector3(0,height-830,0), 16.0f,false);

	std::ostringstream stringStream6;
	stringStream6<< "Press \"SHIFT\" and \"SPACE\" to go up and down." << endl;
	std::string controls3(stringStream6.str());
	DrawText(controls3, Vector3(0,height-810,0), 16.0f,false);

	glBlendFunc ( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
}

void Renderer::DrawText(const std::string &text, const Vector3 &position, const float size, const bool perspective)	{
	TextMesh* mesh = new TextMesh(text,*basicFont);

	if(perspective) {
		modelMatrix = Matrix4::Translation(position) * Matrix4::Scale(Vector3(size,size,1));
		viewMatrix = camera->BuildViewMatrix();
	}
	else{	
		modelMatrix = Matrix4::Translation(Vector3(position.x,height-position.y, position.z)) * Matrix4::Scale(Vector3(size,size,1));
		viewMatrix.ToIdentity();
		projMatrix = Matrix4::Orthographic(-1.0f,1.0f,(float)width, 0.0f,(float)height, 0.0f);
	}
	//Either way, we update the matrices, and draw the mesh
	UpdateShaderMatrices();
	mesh->Draw();

	modelMatrix.ToIdentity();
	SwitchToPerspective(heightMap_zoom);

	delete mesh; //Once it's drawn, we don't need it anymore!
}


/////////////////Deferred RENDERING////////////////////////



void	Renderer::FillSceneBuffer() {

	glBindFramebuffer(GL_FRAMEBUFFER ,SceneFBO);

	glClear ( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT |GL_STENCIL_BUFFER_BIT );

	DrawSkybox();
		
	DrawCloth();
	
	DrawHeightmap(heightMapShader);

	DrawParticle();
	
	glBindFramebuffer(GL_FRAMEBUFFER,0);

}

void	Renderer::FillDRBuffers() {

	glBindFramebuffer(GL_FRAMEBUFFER ,bufferFBO);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
	glClearColor(0.0,0.0,0.0,1.0);

	DrawHeightmap(DR_heightMapShader);
	
	SetCurrentShader(sceneShader);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram (),"diffuseTex") , 3);
	glUniform1i(glGetUniformLocation(currentShader->GetProgram (),"bumpTex") , 1);
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"damageTex") , 7);
	glActiveTexture ( GL_TEXTURE2 );
	glBindTexture ( GL_TEXTURE_2D , DamageBufferColourTex[0]);
	SwitchToPerspective(heightMap_zoom);
	modelMatrix.ToIdentity ();
	viewMatrix = camera -> BuildViewMatrix();
	UpdateShaderMatrices ();	
	
	heightMap -> Draw();
	DrawNodes();
	DrawCloth();	
	
	glUseProgram (0);
	glBindFramebuffer ( GL_FRAMEBUFFER , 0);

}

void	Renderer::DrawPointLights() {

	//Setup Second Pass Buffer
	glBindFramebuffer(GL_FRAMEBUFFER , pointLightFBO);
	glClearColor (0.0 ,0.0 ,0.0 ,0.0);
	glClear( GL_COLOR_BUFFER_BIT );
	glEnable(GL_BLEND);
	glBlendFunc (GL_ONE , GL_ONE );	

	//Draw Point Light.
	SetCurrentShader(pointlightShader);
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"depthTex"), 3);
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"normTex") , 4);
	glActiveTexture ( GL_TEXTURE3 );
	glBindTexture ( GL_TEXTURE_2D , bufferDepthTex );
	glActiveTexture ( GL_TEXTURE4 );
	glBindTexture ( GL_TEXTURE_2D , bufferNormalTex );
	glUniform3fv ( glGetUniformLocation ( currentShader -> GetProgram (),"cameraPos"), 1,( float *)& camera -> GetPosition ());
	glUniform2f ( glGetUniformLocation ( currentShader -> GetProgram () ,"pixelSize"), 1.0f / width , 1.0f / height );

	//For Torch/Spotlight.
	SetCurrentShader(spotLightShader);
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"depthTex"), 3);
	glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"normTex") , 4);
	glActiveTexture ( GL_TEXTURE3 );
	glBindTexture ( GL_TEXTURE_2D , bufferDepthTex );
	glActiveTexture ( GL_TEXTURE4 );
	glBindTexture ( GL_TEXTURE_2D , bufferNormalTex );
	glUniform3fv ( glGetUniformLocation ( currentShader -> GetProgram (),"cameraPos"), 1,( float *)& camera -> GetPosition ());
	glUniform2f ( glGetUniformLocation ( currentShader -> GetProgram () ,"pixelSize"), 1.0f / width , 1.0f / height );
		
	//TODO : Implements Lights - Work in Progress.
	//glEnable(GL_CULL_FACE);
	//if( dist < radius ) {
	//	glCullFace ( GL_FRONT );
	//}else {
	//	glCullFace ( GL_BACK );
	//}				
	//lightSphere -> Draw ();
	//}

	//Draw Torch
	//if(KeyboardMouseManager::GetKeyboardMouseManager().torchInUse) {
	if(torch == true) {

		Light* lightToDraw = new Light();

		Matrix4 rotation = Matrix4 :: Rotation ( camera ->GetYaw(), Vector3 (0 ,1,0)) * Matrix4 :: Rotation ( camera ->GetPitch()+90.0f, Vector3 (1 ,0 ,0));
		Matrix4 camRotation = Matrix4 :: Rotation ( camera ->GetYaw(), Vector3 (0 ,1,0)) * Matrix4 :: Rotation ( camera ->GetPitch(), Vector3 (1 ,0 ,0));

		Vector3 forward= -camRotation.GetForwardRotationVector();

		glUniform3fv(glGetUniformLocation(currentShader->GetProgram(),"spotDirIn"), 1,( float *)&forward);
		glUniform1f(glGetUniformLocation(currentShader-> GetProgram() , "degreeToSpot"),70.0f);
		
		float scale = 50000;

		Vector3 up = -camRotation.GetUpRotationVector();

		modelMatrix = Matrix4 :: Translation (camera-> GetPosition()-(up*-100))*  rotation * Matrix4 :: Scale ( Vector3 (scale,scale,scale));
		
		lightToDraw->SetPosition ( modelMatrix . GetPositionVector ());
		lightToDraw->SetRadius(2000);
		SetShaderLight (*lightToDraw);
		
		UpdateShaderMatrices ();

		float dist2 =(lightToDraw->GetPosition()-camera->GetPosition()).Length();
			
		glEnable(GL_CULL_FACE);
		if( dist2 < scale ) {
			glCullFace ( GL_FRONT );
		}else {
			glCullFace ( GL_BACK );
		}				
		lightSphere -> Draw ();
	}

	glCullFace ( GL_BACK );
	glBlendFunc ( GL_SRC_ALPHA , GL_ONE_MINUS_SRC_ALPHA );
	glDisable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER,0);
	glUseProgram (0);

}

void	Renderer::CombineBuffers() {

	//Stencil Start Here
	glEnable(GL_STENCIL_TEST);
	glDepthMask(GL_FALSE);
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	glStencilFunc(GL_ALWAYS,5,~0);
	glStencilOp(GL_REPLACE,GL_REPLACE,GL_REPLACE);
	//Things to Stencil:
		DrawText();
		lightSphere ->Draw();
		
	glDepthMask(GL_TRUE);
	glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
	glStencilFunc(GL_NOTEQUAL,5,~0);
	glStencilOp(GL_KEEP,GL_KEEP,GL_KEEP);
	//Things to Draw Behind.
		SetCurrentShader ( combineShader );
		projMatrix = Matrix4 :: Orthographic ( -1 ,1 ,1 , -1 , -1 ,1);
		UpdateShaderMatrices ();
		glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"diffuseTex") , 2);
		glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"emissiveTex") , 3);
		glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"specularTex") , 4);
		glUniform1i ( glGetUniformLocation ( currentShader -> GetProgram (),"damageTex") , 5);
		glUniform1f ( glGetUniformLocation ( currentShader -> GetProgram (),"clock") , clock);
		glActiveTexture ( GL_TEXTURE2 );
		glBindTexture ( GL_TEXTURE_2D , SceneBufferColourTex );//SceneBufferColourTex
		glActiveTexture ( GL_TEXTURE3 );
		glBindTexture ( GL_TEXTURE_2D , lightEmissiveTex );
		glActiveTexture ( GL_TEXTURE4 );
		glBindTexture ( GL_TEXTURE_2D , lightSpecularTex );
		glActiveTexture ( GL_TEXTURE5 );
		glBindTexture ( GL_TEXTURE_2D , DamageBufferColourTex[0] );
		fullScreenQuad -> Draw ();
		SwitchToPerspective(heightMap_zoom);
	glDisable(GL_STENCIL_TEST);
	GL_BREAKPOINT;
	glUseProgram (0);

}


void	Renderer::SwitchToPerspective(float zoom) {
	projMatrix = Matrix4::Perspective(1.0f,80000.0f,(float)width/(float)height,zoom);
}


