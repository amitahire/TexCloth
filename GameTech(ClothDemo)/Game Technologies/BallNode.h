#pragma once
#include "PhysicsNode.h"

class BallNode: public PhysicsNode {
public:	
	//Constructor & Deconstructor
	BallNode(void);
	BallNode(Quaternion orientation, Vector3 position);
	BallNode::BallNode(Quaternion orientation, Vector3 position,bool in_isSlept);
	~BallNode(void);

	//init Function
	void			init_variable();
	
	//Setup Force:
	void			SetGravity(Vector3 gravity)	{m_gravity = gravity;}
	Vector3			GetGravity()				{return m_gravity;}
	void			AddForce(float x,float y,float z);
	void			AddForce(Vector3 force);
	
	//Override Update Function Call
	virtual void	Update(float msec);

private:
	Vector3 m_gravity;
};