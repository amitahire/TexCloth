#include "Cloth.h"

Cloth::Cloth(Vector3 worldPosSet){ 

	clothPos = worldPosSet;
	//Physics Particle Representation
	pNode = new PhysicsNode*[CLOTH_WIDTH];
	for(int i = 0; i < CLOTH_WIDTH;i++){
		pNode[i] = new PhysicsNode[CLOTH_HEIGHT];
	}

	//Generate Renderering Part
	numVertices = CLOTH_WIDTH*CLOTH_HEIGHT;
	numIndices = (CLOTH_WIDTH-1)*(CLOTH_HEIGHT-1)*6;
	vertices = new Vector3[numVertices];
	textureCoords = new Vector2[numVertices];
	indices = new GLuint[numIndices];	
	numIndices = 0;

	for (int i = 0; i < CLOTH_WIDTH ; ++ i ) {
		for (int j = 0; j < CLOTH_HEIGHT ; ++ j ) {			

			pNode[i][j].m_position.x = clothPos.x;
			pNode[i][j].m_position.y = j * particleApart_Height + clothPos.y;
			pNode[i][j].m_position.z = i * particleApart_Width + clothPos.z;

			//TODO - Cloth collision.
			//pNode [i][j].m_colli_radius = colliSenseRadius;
			pNode [i][j].SetInverseMass(Cloth_Mass);	
			particleList.push_back(&pNode[i][j]);
			
			int offset = ( i * CLOTH_WIDTH ) + j ;
			vertices[offset] = pNode[i][j].m_position;
			textureCoords[offset] = Vector2(1-i*clothTexture_Width, 1-j*clothTexture_Height);

			if(i<CLOTH_WIDTH-1){
				if(j<CLOTH_HEIGHT-1){
					int a = ( i * ( CLOTH_WIDTH )) + j ;
					int b = (( i +1) * ( CLOTH_WIDTH )) + j ;
					int c = (( i +1) * ( CLOTH_WIDTH )) + ( j +1);
					int d = ( i * ( CLOTH_WIDTH )) + ( j +1);
					indices [numIndices ++] = c ;
					indices [numIndices ++] = b ;
					indices [numIndices ++] = a ;
					indices [numIndices ++] = a ;
					indices [numIndices ++] = d ;
					indices [numIndices ++] = c ;
				}
			}

		}
	}
	GenerateTangents();
	GenerateNormals();
	BufferData();	

	//Bounding Box For ClothMap
	physicsBound = new PhysicsNode();

	physicsBound->collBox->ListPtr = &particleList;

	PhysicsSystem::GetPhysicsSystem().AddNode(physicsBound);
}

void Cloth::Draw(){
	RecalculatePosition();
	Mesh::Draw();
}

void Cloth::RecalculatePosition(){
	//4 Direction Force
	int totalDamage = 0;
	for (int i = 0 ; i < CLOTH_WIDTH; i ++){
		for(int j = 0; j < CLOTH_HEIGHT; j++){	
			Vector3 f0 = Vector3(0,0,0);
			Vector3 f1 = Vector3(0,0,0);
			Vector3 f2 = Vector3(0,0,0);
			Vector3 f3 = Vector3(0,0,0);
			pNode [i][j].hit = false;

			//Force On Top
			if(j+1< CLOTH_HEIGHT){
				Vector3 temp = pNode [i][j].m_position - pNode [i][j+1].m_position;
				temp.Normalise();
				f3 = (pNode [i][j].m_position - pNode [i][j+1].m_position - temp*Vector3(particleApart_Height));
			}				

			//Force Below
			if(j-1>=0){
				Vector3 temp = pNode [i][j].m_position - pNode [i][j-1].m_position;
				temp.Normalise();
				f2 = (pNode [i][j].m_position - pNode [i][j-1].m_position- temp*Vector3(particleApart_Height));

			}

			//Force On Right
			if(i+1< CLOTH_WIDTH){
				Vector3 temp = pNode [i][j].m_position - pNode [i+1][j].m_position;
				temp.Normalise();
				f1 = (pNode [i][j].m_position - pNode [i+1][j].m_position- temp*Vector3(particleApart_Width));

			}			
			
			//Force On Left
			if(i-1>=0){
				Vector3 temp = pNode [i][j].m_position - pNode [i-1][j].m_position;
				temp.Normalise();
				f0 = (pNode [i][j].m_position - pNode [i-1][j].m_position- temp*Vector3(particleApart_Width));

			}


			if(pNode [i][29].hit == true){ pNode [i][28].hit = true; } //Hack fix 
			if(pNode [29][j].hit == true){ pNode [28][j].hit = true; } //Hack fix 
			Vector3 F = -(Vector3(KS)*(f0+f1+f2+f3));				
			pNode [i][j].m_force = F - Vector3(KD) * pNode[i][j].m_linearVelocity;
			pNode [i][j].m_force.y += pNode [i][j].GetMass() * G*GravityOn ;
		}
	}



	for (int i = 0 ; i < CLOTH_WIDTH; i ++){
		for(int j = 0; j < CLOTH_HEIGHT; j++){		

			// accelerations
			Vector3 a = pNode [i][j].m_force*(1.0f/pNode [i][j].GetMass());

			// velocities
			pNode[i][j].m_linearVelocity += a * DT;

			// positions
			pNode[i][j].m_position += pNode[i][j].m_linearVelocity * DT ;

			SetFixPoint();

			pNode[i][j].s1->pos = pNode[i][j].m_position;	

			//Assign First Particle
			if(i == 0 && j == 0){
				maxXYZ = pNode[i][j].m_position;
				minXYZ = pNode[i][j].m_position;
			} else {
				SetBoundingBox(pNode[i][j].m_position);
			}
		}
	}

	//Recalculate Bounding Box
	float lengthX = (maxXYZ.x - minXYZ.x)/2;
	float lengthY = (maxXYZ.y - minXYZ.y)/2;
	float lengthZ = (maxXYZ.z - minXYZ.z)/2;
	
	physicsBound-> collBox->pos = Vector3((minXYZ.x + lengthX), (minXYZ.y + lengthY),(minXYZ.z + lengthZ));
	physicsBound-> collBox->halfdims = Vector3(lengthX,lengthY,lengthZ);
	RebondIndices();
	ReBufferValue();
}

void Cloth::SetBoundingBox(Vector3 position){
	if(maxXYZ.x < position.x){
		maxXYZ.x = position.x;
	}
	if(maxXYZ.y < position.y){	
		maxXYZ.y = position.y;
	}
	if(maxXYZ.z < position.z){	
		maxXYZ.z = position.z;
	}
	if(minXYZ.x > position.x){
		minXYZ.x = position.x;
	}
	if(minXYZ.y > position.y){
		minXYZ.y = position.y;
	}
	if(minXYZ.z > position.z){
		minXYZ.z = position.z;
	}
}

void Cloth::RebondIndices(){	
	int IndicesCounter = 0;
	for (int i = 0; i < CLOTH_WIDTH -1; ++ i ) {
		for (int j = 0; j < CLOTH_HEIGHT -1; ++ j ) {
			if(pNode [i][j].hit == false){						
				int a = ( i * ( CLOTH_WIDTH )) + j ;
				int b = (( i +1) * ( CLOTH_WIDTH )) + j ;
				int c = (( i +1) * ( CLOTH_WIDTH )) + ( j +1);
				int d = ( i * ( CLOTH_WIDTH )) + ( j +1);

				indices [IndicesCounter ++] = c ;
				indices [IndicesCounter ++] = b ;
				indices [IndicesCounter ++] = a ;

				indices [IndicesCounter ++] = a ;
				indices [IndicesCounter ++] = d ;
				indices [IndicesCounter ++] = c ;
			} else {

				int a = (i * ( CLOTH_WIDTH )) + j ;
				int b = ((i +1) * ( CLOTH_WIDTH )) + j ;
				int c = (( i +1) * ( CLOTH_WIDTH )) + ( j +1);
				int d = ( i * ( CLOTH_WIDTH )) + ( j +1);
						
				//Top Right Box
				indices [IndicesCounter ++] = c ;
				indices [IndicesCounter ++] = c ;
				indices [IndicesCounter ++] = c ;

				indices [IndicesCounter ++] = c ;
				indices [IndicesCounter ++] = c ;
				indices [IndicesCounter ++] = c ;
				//Top Left Box
				if(j != 0){
					int temp = IndicesCounter;
					temp = temp - 12;
					int value = temp;
					if(temp<numIndices)indices [temp++] = indices [value];
					if(temp<numIndices)indices [temp++] = indices [value];
					if(temp<numIndices)indices [temp++] = indices [value];
						
					if(temp<numIndices)indices [temp++] = indices [value];
					if(temp<numIndices)indices [temp++] = indices [value];
					if(temp<numIndices)indices [temp++] = indices [value];
				}
				//Bottom Left Box
				if(i != 0){
					if(j != 0){				
						int temp = IndicesCounter;
						temp = (temp-12)-(6*CLOTH_WIDTH);
						int value = temp;
						if(temp<numIndices)indices [temp ++] = indices [value];
						if(temp<numIndices)indices [temp ++] = indices [value];
						if(temp<numIndices)indices [temp ++] = indices [value];

						if(temp<numIndices)indices [temp ++] = indices [value];
						if(temp<numIndices)indices [temp ++] = indices [value];
						if(temp<numIndices)indices [temp ++] = indices [value];
					}
				}						
				//Bottom Right Box
				if(i != 0){								
					int temp = IndicesCounter;
					temp = temp-(6*(CLOTH_WIDTH));
					int value = temp;
					if(temp<numIndices)indices [temp ++] = indices [value];
					if(temp<numIndices)indices [temp ++] = indices [value];
					if(temp<numIndices)indices [temp ++] = indices [value];

					if(temp<numIndices)indices [temp ++] = indices [value];
					if(temp<numIndices)indices [temp ++] = indices [value];
					if(temp<numIndices)indices [temp ++] = indices [value];
				}
			}
		}
	}
}

void Cloth::ReBufferValue(){
	for (int x = 0; x < CLOTH_WIDTH ; ++ x ) {
		for (int y = 0; y < CLOTH_HEIGHT ; ++ y ) {			
			int offset = ( x * CLOTH_WIDTH ) + y;
			vertices[offset] = pNode[x][y].m_position;
		}
	}

	GenerateTangents();
	GenerateNormals();
	RebufferNormal();

	glBindBuffer(GL_ARRAY_BUFFER,bufferObject[VERTEX_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER,numVertices*sizeof(Vector3),vertices,GL_DYNAMIC_DRAW);

	glBindBuffer (GL_ARRAY_BUFFER , bufferObject [NORMAL_BUFFER]);
	glBufferData (GL_ARRAY_BUFFER , numVertices * sizeof ( Vector3 ) ,normal , GL_DYNAMIC_DRAW );

	glBindBuffer(GL_ARRAY_BUFFER,bufferObject[TANGENT_BUFFER]);
	glBufferData(GL_ARRAY_BUFFER,numVertices*sizeof(Vector3),tangents,GL_DYNAMIC_DRAW);
	
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,bufferObject[INDEX_BUFFER]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER,numIndices*sizeof(GLuint),indices,GL_DYNAMIC_DRAW);
}

void Cloth::SetFixPoint(){
	if(fixedPoint){

			pNode[0][CLOTH_HEIGHT-1].m_position.x = clothPos.x;			
			pNode[0][CLOTH_HEIGHT-1].m_position.y = clothSize_Height+clothPos.y;
			pNode[0][CLOTH_HEIGHT-1].m_position.z = clothPos.z;

			pNode[CLOTH_WIDTH-1][CLOTH_HEIGHT-1].m_position.x = clothPos.x;	
			pNode[CLOTH_WIDTH-1][CLOTH_HEIGHT-1].m_position.y = clothSize_Height+clothPos.y;		
			pNode[CLOTH_WIDTH-1][CLOTH_HEIGHT-1].m_position.z = clothSize_Width + clothPos.z;

			//Static pos - Top
			for (int i =0; i < CLOTH_WIDTH; i++){
				pNode[i][CLOTH_WIDTH-1].m_position. x = i * clothSize_Width/(CLOTH_WIDTH-1)+ clothPos.x;
				pNode[i][CLOTH_WIDTH-1].m_position.y = clothSize_Width+clothPos.y;
				pNode[i][CLOTH_WIDTH-1].m_position. z = clothSize_Width+clothPos.z;	
			}
	}
}

void Cloth::ResetClothPoint(){
	for (int i =0; i < CLOTH_WIDTH ; i ++){
		for(int j = 0; j < CLOTH_HEIGHT; j++){
				pNode[i][j].m_position.x = clothPos.x;
				pNode[i][j].m_position.y = j * particleApart_Height + clothPos.y;
				pNode[i][j].m_position.z = i * particleApart_Width + clothPos.z;
		}
	}	
}

void Cloth::RebufferNormal(){
for (int i = 0; i < CLOTH_WIDTH -1; ++ i ) {
	for (int j = 0; j < CLOTH_HEIGHT-1 ; ++ j ) {
			int origin = (i * ( CLOTH_WIDTH )) + j ;
			int top = ((i +1) * ( CLOTH_WIDTH )) + j ; 	
			int bottom = ((i -1) * ( CLOTH_WIDTH )) + j ; 
			int left = ( i * ( CLOTH_WIDTH )) + ( j +1);
			int right = ( i * ( CLOTH_WIDTH )) + ( j -1);

			Vector3 surfaceNormal = normal[origin];
			if(top >= 0 && top < numVertices){
				surfaceNormal += normal[top];
			}
			if(bottom >= 0 && bottom < numVertices){
				surfaceNormal += normal[bottom];
			}
			if(left >= 0 && left < numVertices){
				surfaceNormal += normal[left];
			}
			if(right >= 0 && right < numVertices){
				surfaceNormal += normal[right];
			}
			surfaceNormal.Normalise();
			pNode[i][j].collisionNormal = surfaceNormal;
			pNode[i][j].useSelfNormal = true;
		}
	}
}

void Cloth::AddForce(int i) {

		if(i == 0){
		//cout << "0" <<endl;
		for ( int i = 0; i < clothScale; i++) {
			pNode[i][clothScale-1].m_position.z = pNode[i][clothScale-1].m_position.z + 200.0f;
			//pNode[ballsqr-1][i].m_position.z = pNode[ballsqr-1][i].m_position.z - 1.0f;
			//pNode[i][clothScale-1].m_position.z = pNode[i][clothScale-1].m_position.z - 10.0f;

		}
		}
		if(i == 3){
			
		//	cout << "3" <<endl;
		//	pNode[(clothScale-1)/2][(clothScale-1)/2].m_position.y = pNode[(int)(clothScale-1)/2][(int)(clothScale-1)/2].m_position.z + 5.0f;
			for ( int i = 0; i < clothScale; i++) {
			pNode[clothScale-1][i].m_position.y = pNode[clothScale-1][i].m_position.y + 200.0f;
			pNode[i][clothScale-1].m_position.y = pNode[clothScale-1][i].m_position.y + 200.0f;

			//pNode[ballsqr-1][i].m_position.z = pNode[ballsqr-1][i].m_position.z - 1.0f;
			//pNode[i][clothScale-1].m_position.z = pNode[i][clothScale-1].m_position.z - 10.0f;

		}
		}



}

