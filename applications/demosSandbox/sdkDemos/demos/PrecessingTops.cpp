/* Copyright (c) <2009> <Newton Game Dynamics>
* 
* This software is provided 'as-is', without any express or implied
* warranty. In no event will the authors be held liable for any damages
* arising from the use of this software.
* 
* Permission is granted to anyone to use this software for any purpose,
* including commercial applications, and to alter it and redistribute it
* freely
*/


#include <toolbox_stdafx.h>
#include "SkyBox.h"
#include "DemoEntityManager.h"
#include "DemoCamera.h"
#include "PhysicsUtils.h"
#include "DemoMesh.h"
#include "../toolBox/OpenGlUtil.h"

static void PhysicsApplyPrecessionTorque (const NewtonBody* body, dFloat timestep, int threadIndex)
{
	PhysicsApplyGravityForce (body, timestep, threadIndex);

	dFloat Ixx;
	dFloat Iyy;
	dFloat Izz;
	dFloat mass;
	
	dVector omega;
	dMatrix rotation;
	NewtonBodyGetOmega(body, &omega[0]); 
	NewtonBodyGetMatrix(body, &rotation[0][0]);
	NewtonBodyGetMassMatrix (body, &mass, &Ixx, &Iyy, &Izz);

	//apply gyroscope torque   
	omega = rotation.UnrotateVector(omega);
	dVector gyro(omega.m_x * Ixx, omega.m_y * Iyy, omega.m_z * Izz, 0.0f);
	dVector torque (rotation.RotateVector(omega * gyro));
	NewtonBodySetTorque(body, &torque.m_x);
}



void PrecessingTops (DemoEntityManager* const scene)
{
	scene->CreateSkyBox();

	// customize the scene after loading
	// set a user friction variable in the body for variable friction demos
	// later this will be done using LUA script
	dMatrix offsetMatrix (dGetIdentityMatrix());

	CreateLevelMesh (scene, "flatPlane.ngd", 1);

	dVector location (0.0f, 0.0f, 0.0f, 0.0f);
	dVector size (1.5f, 2.0f, 2.0f, 0.0f);

	// create an array of cones 
	int count = 10;

	// all shapes use the x axis as the  axis of symmetry, to make an upright cone we apply a 90 degree rotation local matrix
	dMatrix shapeOffsetMatrix (dRollMatrix(-3.141592f/2.0f));
	AddPrimitiveArray(scene, 10.0f, location, size, count, count, 3.0f, _CONE_PRIMITIVE, 0, shapeOffsetMatrix);

	// till the cont 30 degrees, and apply a local high angular velocity
	dMatrix matrix (dRollMatrix (-15.0f * 3.141592f / 180.0f));
	dVector omega (0.0f, 50.0f, 0.0f);
	omega = matrix.RotateVector (omega);
	dVector damp (0.0f, 0.0f, 0.0f, 0.0f);

	NewtonWorld* const world = scene->GetNewton();
	for (NewtonBody* body = NewtonWorldGetFirstBody(world); body; body = NewtonWorldGetNextBody(world, body)) {
	
		NewtonCollision* const collision = NewtonBodyGetCollision(body);
		if (NewtonCollisionGetType (collision) == SERIALIZE_ID_CONE) {
			dMatrix bodyMatrix;
			NewtonBodyGetMatrix(body, &bodyMatrix[0][0]);
			matrix.m_posit = bodyMatrix.m_posit;
			NewtonBodySetMatrix(body, &matrix[0][0]);

			NewtonBodySetOmega (body, &omega[0]);
			NewtonBodySetForceAndTorqueCallback (body, PhysicsApplyPrecessionTorque);

			NewtonBodySetAutoSleep (body, 0);
			NewtonBodySetLinearDamping(body, 0.0f);
			NewtonBodySetAngularDamping (body, &damp[0]);
		}
	}

	// place camera into position
	dMatrix camMatrix (dGetIdentityMatrix());
	dQuaternion rot (camMatrix);
	dVector origin (-40.0f, 5.0f, 0.0f, 0.0f);
	scene->SetCameraMatrix(rot, origin);
}



