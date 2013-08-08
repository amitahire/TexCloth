#include "BallNode.h"

void BallNode::init_variable(){
	float inertia = 2*PI*250/5;
	m_invInertia.SetScalingVector(Vector3(1/inertia,1/inertia,1/inertia));	
	m_gravity = Vector3(0,-20,0);
	s1->radius = m_colli_radius;
	collBox->halfdims = Vector3(m_colli_radius);
	s1->pos = m_position;
	collBox->pos = m_position;
}

BallNode::BallNode(void)	{
	target = NULL;		
	init_variable();
}

BallNode::BallNode(Quaternion orientation, Vector3 position) {
	m_orientation	= orientation;
	m_position		= position;
	init_variable();
}

BallNode::BallNode(Quaternion orientation, Vector3 position,bool setAtRest){
	BallNode(orientation,position);
	atRest = setAtRest;
}

BallNode::~BallNode(void)	{

}

void BallNode::Update(float msec) {
	msec = msec * 0.01f;
	if(!atRest){
		Vector3 acceleration = (m_force+m_gravity)*m_invMass;
		m_linearVelocity = m_linearVelocity + acceleration*msec;
		m_position = m_position + m_linearVelocity*msec;	
		m_force = Vector3(0,0,0);

		Vector3 AngularAcceleration = m_invInertia.GetScalingVector()*m_torque;		
		m_angularVelocity = m_angularVelocity + AngularAcceleration * msec;
		m_orientation = m_orientation * Quaternion::EulerAnglesToQuaternion(m_angularVelocity.x,m_angularVelocity.y,m_angularVelocity.z);
		m_torque = Vector3(0,0,0);
	
		s1->radius = m_colli_radius;	
		collBox->halfdims = Vector3(m_colli_radius);
		
		s1->pos = m_position;
		collBox->pos = m_position;	
			
		m_linearVelocity = Vector3(0.9999)*m_linearVelocity;
		m_angularVelocity = Vector3(0.9999)*m_angularVelocity;
					
		//Sleep Hack		
		float sleepSpeed = 1.0f;
		if(m_linearVelocity.x < sleepSpeed && m_linearVelocity.x > -sleepSpeed){
			if(m_linearVelocity.y < sleepSpeed && m_linearVelocity.y > -sleepSpeed){
				if(m_linearVelocity.z < sleepSpeed && m_linearVelocity.z > -sleepSpeed){
					atRest = true;
				}
			}
		}

		//Bounderies - The Walls!
		if(true){
			if(m_position.y > 4096){
				m_linearVelocity.y = -abs(m_linearVelocity.y);
			}
			if(m_position.x < 0){
				m_linearVelocity.x = abs(m_linearVelocity.x);
			}
			if(m_position.x > 4096){
				m_linearVelocity.x = -abs(m_linearVelocity.x);
			}
			if(m_position.z < 0){
				m_linearVelocity.z = abs(m_linearVelocity.z);
			}
			if(m_position.z >4096){
				m_linearVelocity.z = -abs(m_linearVelocity.z);
			}
			if(m_position.y < -500){
				m_linearVelocity.y = abs(m_linearVelocity.y);
				//m_position.y = 50;
			}
		}		
	}
	if(target) { target->SetTransform(BuildTransform());}
}

void BallNode::AddForce(Vector3 force){
	m_force += Vector3(force.x,force.y,force.z);
}