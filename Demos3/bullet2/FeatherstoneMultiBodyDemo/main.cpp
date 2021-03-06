
#define ARRAY_SIZE_X 5
#define ARRAY_SIZE_Y 5
#define ARRAY_SIZE_Z 5
float scaling = 1.f;
float friction = 1.;


#include "OpenGLWindow/SimpleOpenGL3App.h"
#include "Bullet3Common/b3Vector3.h"
#include "assert.h"
#include <stdio.h>

#include "btBulletDynamicsCommon.h"

#include "BulletDynamics/Featherstone/btMultiBody.h"
#include "BulletDynamics/Featherstone/btMultiBodyConstraintSolver.h"
#include "BulletDynamics/Featherstone/btMultiBodyDynamicsWorld.h"
#include "BulletDynamics/Featherstone/btMultiBodyLinkCollider.h"
#include "BulletDynamics/Featherstone/btMultiBodyLink.h"
#include "BulletDynamics/Featherstone/btMultiBodyJointLimitConstraint.h"
#include "BulletDynamics/Featherstone/btMultiBodyJointMotor.h"
#include "BulletDynamics/Featherstone/btMultiBodyPoint2Point.h"

static b3Vector4 colors[4] =
{
	b3MakeVector4(1,0,0,1),
	b3MakeVector4(0,1,0,1),
	b3MakeVector4(0,1,1,1),
	b3MakeVector4(1,1,0,1),
};


struct btMultiBodySettings
{
	btMultiBodySettings()
	{
		m_numLinks = 0;
		m_basePosition.setZero();
		m_isFixedBase = true;
		m_usePrismatic = false;
		m_canSleep = true;
		m_createConstraints = false;
		m_disableParentCollision = false;
	}
	int			m_numLinks;
	btVector3	m_basePosition;
	bool		m_isFixedBase;
	bool		m_usePrismatic;
	bool		m_canSleep;
	bool		m_createConstraints;
	bool		m_disableParentCollision;
};

class Bullet2MultiBodyDemo
{
protected:
	btMultiBodyDynamicsWorld* m_dynamicsWorld;
	btCollisionDispatcher*	m_dispatcher;
	btBroadphaseInterface*	m_bp;
	btCollisionConfiguration* m_config;
	btMultiBodyConstraintSolver* m_solver;

public:
	Bullet2MultiBodyDemo()
	{
		m_config = 0;
		m_dispatcher = 0;
		m_bp = 0;
		m_solver = 0;
		m_dynamicsWorld = 0;
	}
	virtual void initPhysics()
	{
		m_config = new btDefaultCollisionConfiguration;
		m_dispatcher = new btCollisionDispatcher(m_config);
		m_bp = new btDbvtBroadphase();
		m_solver = new btMultiBodyConstraintSolver();
		m_dynamicsWorld = new btMultiBodyDynamicsWorld(m_dispatcher,m_bp,m_solver,m_config);
	}
	virtual void exitPhysics()
	{
		delete m_dynamicsWorld;
		m_dynamicsWorld=0;
		delete m_solver;
		m_solver=0;
		delete m_bp;
		m_bp=0;
		delete m_dispatcher;
		m_dispatcher=0;
		delete m_config;
		m_config=0;
	}

	virtual ~Bullet2MultiBodyDemo()
	{
		btAssert(m_config == 0);
		btAssert(m_dispatcher == 0);
		btAssert(m_bp == 0);
		btAssert(m_solver == 0);
		btAssert(m_dynamicsWorld == 0);
	}

};

class BasicDemo : public Bullet2MultiBodyDemo
{
	SimpleOpenGL3App* m_glApp;

	btRigidBody*	m_pickedBody;
	btTypedConstraint* m_pickedConstraint;
	btVector3 m_oldPickingPos;
	btVector3 m_hitPos;
	btScalar m_oldPickingDist;

	class btMultiBodyPoint2Point*		m_pickingMultiBodyPoint2Point;
	btAlignedObjectArray<btMultiBodyLinkCollider*> m_linkColliders;

public:
	BasicDemo(SimpleOpenGL3App* app)
	:m_glApp(app),
	m_pickedBody(0),
	m_pickedConstraint(0),
	m_pickingMultiBodyPoint2Point(0)
	{
	}
	virtual ~BasicDemo()
	{
	}

	btMultiBody* createFeatherstoneMultiBody(class btMultiBodyDynamicsWorld* world, const btMultiBodySettings& settings)
	{
		static int curColor=0;
					
	
		int cubeShapeId = m_glApp->registerCubeShape();
		
		int n_links = settings.m_numLinks;
		float mass = 13.5*scaling;
		btVector3 inertia = btVector3 (91,344,253)*scaling*scaling;
	
	
		btMultiBody * bod = new btMultiBody(n_links, mass, inertia, settings.m_isFixedBase, settings.m_canSleep);
	//		bod->setHasSelfCollision(false);

		//btQuaternion orn(btVector3(0,0,1),-0.25*SIMD_HALF_PI);//0,0,0,1);
		btQuaternion orn(0,0,0,1);
		bod->setBasePos(settings.m_basePosition);
		bod->setWorldToBaseRot(orn);
		btVector3 vel(0,0,0);
		bod->setBaseVel(vel);

		{
			
			btVector3 joint_axis_hinge(1,0,0);
			btVector3 joint_axis_prismatic(0,0,1);
			btQuaternion parent_to_child = orn.inverse();
			btVector3 joint_axis_child_prismatic = quatRotate(parent_to_child ,joint_axis_prismatic);
			btVector3 joint_axis_child_hinge = quatRotate(parent_to_child , joint_axis_hinge);
        
			int this_link_num = -1;
			int link_num_counter = 0;

		

			btVector3 pos = btVector3 (0,0,9.0500002)*scaling;

			btVector3 joint_axis_position = btVector3 (0,0,4.5250001)*scaling;

			for (int i=0;i<n_links;i++)
			{
				float initial_joint_angle=0.3;
				if (i>0)
					initial_joint_angle = -0.06f;

				const int child_link_num = link_num_counter++;

			

				if (settings.m_usePrismatic)// && i==(n_links-1))
				{
						bod->setupPrismatic(child_link_num, mass, inertia, this_link_num,
							parent_to_child, joint_axis_child_prismatic, quatRotate(parent_to_child , pos),settings.m_disableParentCollision);

				} else
				{
					bod->setupRevolute(child_link_num, mass, inertia, this_link_num,parent_to_child, joint_axis_child_hinge,
											joint_axis_position,quatRotate(parent_to_child , (pos - joint_axis_position)),settings.m_disableParentCollision);
				}
				bod->setJointPos(child_link_num, initial_joint_angle);
				this_link_num = i;
		
				if (0)//!useGroundShape && i==4)
				{
					btVector3 pivotInAworld(0,20,46);
					btVector3 pivotInAlocal = bod->worldPosToLocal(i, pivotInAworld);
					btVector3 pivotInBworld = pivotInAworld;
					btMultiBodyPoint2Point* p2p = new btMultiBodyPoint2Point(bod,i,&btTypedConstraint::getFixedBody(),pivotInAlocal,pivotInBworld);
					world->addMultiBodyConstraint(p2p);
				}
				//add some constraint limit
				if (settings.m_usePrismatic)
				{
		//			btMultiBodyConstraint* con = new btMultiBodyJointLimitConstraint(bod,n_links-1,2,3);
			
					if (settings.m_createConstraints)
					{	
						btMultiBodyConstraint* con = new btMultiBodyJointLimitConstraint(bod,i,-1,1);
						world->addMultiBodyConstraint(con);
					}
			
				} else
				{
					if (settings.m_createConstraints)
					{	
						if (1)
						{
							btMultiBodyJointMotor* con = new btMultiBodyJointMotor(bod,i,0,500000); 
							world->addMultiBodyConstraint(con);
						}

						btMultiBodyConstraint* con = new btMultiBodyJointLimitConstraint(bod,i,-1,1);
						world->addMultiBodyConstraint(con);
					}

				}
			}
		}

		//add a collider for the base
		{
			
			btAlignedObjectArray<btQuaternion> world_to_local;
			world_to_local.resize(n_links+1);

			btAlignedObjectArray<btVector3> local_origin;
			local_origin.resize(n_links+1);
			world_to_local[0] = bod->getWorldToBaseRot();
			local_origin[0] = bod->getBasePos();
			//float halfExtents[3]={7.5,0.05,4.5};
			float halfExtents[3]={7.5,0.45,4.5};
			{
			
				float pos[4]={local_origin[0].x(),local_origin[0].y(),local_origin[0].z(),1};
				float quat[4]={-world_to_local[0].x(),-world_to_local[0].y(),-world_to_local[0].z(),world_to_local[0].w()};

			
				if (1)
				{
					btCollisionShape* box = new btBoxShape(btVector3(halfExtents[0],halfExtents[1],halfExtents[2])*scaling);
					btRigidBody* body = new btRigidBody(mass,0,box,inertia);
					btMultiBodyLinkCollider* col= new btMultiBodyLinkCollider(bod,-1);




					body->setCollisionShape(box);
					col->setCollisionShape(box);
								
					btTransform tr;
					tr.setIdentity();
					tr.setOrigin(local_origin[0]);
					tr.setRotation(btQuaternion(quat[0],quat[1],quat[2],quat[3]));
					body->setWorldTransform(tr);
					col->setWorldTransform(tr);
				
					b3Vector4 color = colors[curColor++];
					curColor&=3;

					int index = m_glApp->m_instancingRenderer->registerGraphicsInstance(cubeShapeId,tr.getOrigin(),tr.getRotation(),color,halfExtents);
					col->setUserIndex(index);




					world->addCollisionObject(col,short(btBroadphaseProxy::DefaultFilter),short(btBroadphaseProxy::AllFilter));
					col->setFriction(friction);
					bod->setBaseCollider(col);
				
				}
			}


			for (int i=0;i<bod->getNumLinks();i++)
			{
				const int parent = bod->getParent(i);
				world_to_local[i+1] = bod->getParentToLocalRot(i) * world_to_local[parent+1];
				local_origin[i+1] = local_origin[parent+1] + (quatRotate(world_to_local[i+1].inverse() , bod->getRVector(i)));
			}

		
			for (int i=0;i<bod->getNumLinks();i++)
			{
		
				btVector3 posr = local_origin[i+1];
				float pos[4]={posr.x(),posr.y(),posr.z(),1};
			
				float quat[4]={-world_to_local[i+1].x(),-world_to_local[i+1].y(),-world_to_local[i+1].z(),world_to_local[i+1].w()};

				btCollisionShape* box = new btBoxShape(btVector3(halfExtents[0],halfExtents[1],halfExtents[2])*scaling);
				btMultiBodyLinkCollider* col = new btMultiBodyLinkCollider(bod,i);

				col->setCollisionShape(box);
				btTransform tr;
				tr.setIdentity();
				tr.setOrigin(posr);
				tr.setRotation(btQuaternion(quat[0],quat[1],quat[2],quat[3]));
				col->setWorldTransform(tr);
				col->setFriction(friction);

								
				b3Vector4 color = colors[curColor++];
				curColor&=3;

				int index = m_glApp->m_instancingRenderer->registerGraphicsInstance(cubeShapeId,tr.getOrigin(),tr.getRotation(),color,halfExtents);
				col->setUserIndex(index);



				world->addCollisionObject(col,short(btBroadphaseProxy::DefaultFilter),short(btBroadphaseProxy::AllFilter));
			
				bod->getLink(i).m_collider=col;
				//app->drawBox(halfExtents, pos,quat);
			}

		}
		world->addMultiBody(bod);

		return bod;
	}
	
	void addColliders_testMultiDof(btMultiBody *pMultiBody, btMultiBodyDynamicsWorld *pWorld, const btVector3 &baseHalfExtents, const btVector3 &linkHalfExtents)
	{
	}
	void addBoxes_testMultiDof()
	{
	}

	void	initPhysics()
	{

		Bullet2MultiBodyDemo::initPhysics();

		//create ground
		int cubeShapeId = m_glApp->registerCubeShape();
		float pos[]={0,0,0};
		float orn[]={0,0,0,1};
		

		{
			float color[]={0.3,0.3,1,1};
			float halfExtents[]={50,50,50,1};
			btTransform groundTransform;
			groundTransform.setIdentity();
			groundTransform.setOrigin(btVector3(0,-50,0));
			btBoxShape* groundShape = new btBoxShape(btVector3(btScalar(halfExtents[0]),btScalar(halfExtents[1]),btScalar(halfExtents[2])));
			//We can also use DemoApplication::localCreateRigidBody, but for clarity it is provided here:
			{
				btScalar mass(0.);
				//rigidbody is dynamic if and only if mass is non zero, otherwise static
				bool isDynamic = (mass != 0.f);
				btVector3 localInertia(0,0,0);
				if (isDynamic)
					groundShape->calculateLocalInertia(mass,localInertia);
				//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
				btDefaultMotionState* myMotionState = new btDefaultMotionState(groundTransform);
				btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,groundShape,localInertia);
				btRigidBody* body = new btRigidBody(rbInfo);

				int index = m_glApp->m_instancingRenderer->registerGraphicsInstance(cubeShapeId,groundTransform.getOrigin(),groundTransform.getRotation(),color,halfExtents);
				body ->setUserIndex(index);

				//add the body to the dynamics world
				m_dynamicsWorld->addRigidBody(body);
			}
		}
#if 0

		{
			float halfExtents[]={1,1,1,1};
		


			btTransform startTransform;
			startTransform.setIdentity();
			btScalar mass = 1.f;
			btVector3 localInertia;
			btBoxShape* colShape = new btBoxShape(btVector3(halfExtents[0],halfExtents[1],halfExtents[2]));
			colShape ->calculateLocalInertia(mass,localInertia);

			for (int k=0;k<ARRAY_SIZE_Y;k++)
			{
				for (int i=0;i<ARRAY_SIZE_X;i++)
				{
					for(int j = 0;j<ARRAY_SIZE_Z;j++)
					{
						static int curColor=0;
						b3Vector4 color = colors[curColor];
						curColor++;
						startTransform.setOrigin(btVector3(
											btScalar(2.0*i),
											btScalar(20+2.0*k),
											btScalar(2.0*j)));

						int index = m_glApp->m_instancingRenderer->registerGraphicsInstance(cubeShapeId,startTransform.getOrigin(),startTransform.getRotation(),color,halfExtents);
			
						//using motionstate is recommended, it provides interpolation capabilities, and only synchronizes 'active' objects
						btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
						btRigidBody::btRigidBodyConstructionInfo rbInfo(mass,myMotionState,colShape,localInertia);
						btRigidBody* body = new btRigidBody(rbInfo);
						body->setUserIndex(index);

						m_dynamicsWorld->addRigidBody(body);
					}
				}
			}
		}
#endif
		btMultiBodySettings settings;
		settings.m_isFixedBase = false;
		settings.m_basePosition.setValue(0,10,0);
		settings.m_numLinks = 10;
		btMultiBody* mb = createFeatherstoneMultiBody(m_dynamicsWorld,settings);

		m_glApp->m_instancingRenderer->writeTransforms();
	}





	void	exitPhysics()
	{
		Bullet2MultiBodyDemo::exitPhysics();
	}
	void	drawObjects()
	{
		//sync graphics -> physics world transforms
		{
			for (int i=0;i<m_dynamicsWorld->getNumCollisionObjects();i++)
			{
				btCollisionObject* col = m_dynamicsWorld->getCollisionObjectArray()[i];

				btVector3 pos = col->getWorldTransform().getOrigin();
				btQuaternion orn = col->getWorldTransform().getRotation();
				int index = col->getUserIndex();
				m_glApp->m_instancingRenderer->writeSingleInstanceTransformToCPU(pos,orn,index);
			}
			m_glApp->m_instancingRenderer->writeTransforms();
		}

		m_glApp->m_instancingRenderer->renderScene();
	}

	btVector3	getRayTo(int x,int y)
	{
		if (!m_glApp->m_instancingRenderer)
		{
			btAssert(0);
			return btVector3(0,0,0);
		}

		float top = 1.f;
		float bottom = -1.f;
		float nearPlane = 1.f;
		float tanFov = (top-bottom)*0.5f / nearPlane;
		float fov = b3Scalar(2.0) * b3Atan(tanFov);

		btVector3 camPos,camTarget;
		m_glApp->m_instancingRenderer->getCameraPosition(camPos);
		m_glApp->m_instancingRenderer->getCameraTargetPosition(camTarget);

		btVector3	rayFrom = camPos;
		btVector3 rayForward = (camTarget-camPos);
		rayForward.normalize();
		float farPlane = 10000.f;
		rayForward*= farPlane;

		btVector3 rightOffset;
		btVector3 m_cameraUp=btVector3(0,1,0);
		btVector3 vertical = m_cameraUp;

		btVector3 hor;
		hor = rayForward.cross(vertical);
		hor.normalize();
		vertical = hor.cross(rayForward);
		vertical.normalize();

		float tanfov = tanf(0.5f*fov);


		hor *= 2.f * farPlane * tanfov;
		vertical *= 2.f * farPlane * tanfov;

		b3Scalar aspect;
		float width = m_glApp->m_instancingRenderer->getScreenWidth();
		float height = m_glApp->m_instancingRenderer->getScreenHeight();

		aspect =  width / height;
	
		hor*=aspect;


		btVector3 rayToCenter = rayFrom + rayForward;
		btVector3 dHor = hor * 1.f/width;
		btVector3 dVert = vertical * 1.f/height;


		btVector3 rayTo = rayToCenter - 0.5f * hor + 0.5f * vertical;
		rayTo += btScalar(x) * dHor;
		rayTo -= btScalar(y) * dVert;
		return rayTo;
	}

	
	bool	mouseMoveCallback(float x,float y)
	{
//		if (m_data->m_altPressed!=0 || m_data->m_controlPressed!=0)
	//		return false;
		
		if (m_pickedBody  && m_pickedConstraint)
		{
			btPoint2PointConstraint* pickCon = static_cast<btPoint2PointConstraint*>(m_pickedConstraint);
			if (pickCon)
			{
				//keep it at the same picking distance
				btVector3 newRayTo = getRayTo(x,y);
				btVector3 rayFrom;
				btVector3 oldPivotInB = pickCon->getPivotInB();
				btVector3 newPivotB;
				m_glApp->m_instancingRenderer->getCameraPosition(rayFrom);
				btVector3 dir = newRayTo-rayFrom;
				dir.normalize();
				dir *= m_oldPickingDist;

				newPivotB = rayFrom + dir;
				pickCon->setPivotB(newPivotB);
			}
		}
		if (m_pickingMultiBodyPoint2Point)
		{
			//keep it at the same picking distance

			btVector3 newRayTo = getRayTo(x,y);
			btVector3 rayFrom;
			btVector3 oldPivotInB = m_pickingMultiBodyPoint2Point->getPivotInB();
			btVector3 newPivotB;
			btVector3 camPos;
			m_glApp->m_instancingRenderer->getCameraPosition(camPos);
			rayFrom = camPos;
			btVector3 dir = newRayTo-rayFrom;
			dir.normalize();
			dir *= m_oldPickingDist;

			newPivotB = rayFrom + dir;
			
			m_pickingMultiBodyPoint2Point->setPivotInB(newPivotB);
		}
		
		return false;
	}
	bool	mouseButtonCallback(int button, int state, float x, float y)
	{

		if (state==1)
		{
			if(button==0)// && (m_data->m_altPressed==0 && m_data->m_controlPressed==0))
			{
				btVector3 camPos;
				m_glApp->m_instancingRenderer->getCameraPosition(camPos);

				btVector3 rayFrom = camPos;
				btVector3 rayTo = getRayTo(x,y);

				btCollisionWorld::ClosestRayResultCallback rayCallback(rayFrom,rayTo);
				m_dynamicsWorld->rayTest(rayFrom,rayTo,rayCallback);
				if (rayCallback.hasHit())
				{

					btVector3 pickPos = rayCallback.m_hitPointWorld;
					btRigidBody* body = (btRigidBody*)btRigidBody::upcast(rayCallback.m_collisionObject);
					if (body)
					{
						//other exclusions?
						if (!(body->isStaticObject() || body->isKinematicObject()))
						{
							m_pickedBody = body;
							m_pickedBody->setActivationState(DISABLE_DEACTIVATION);
							//printf("pickPos=%f,%f,%f\n",pickPos.getX(),pickPos.getY(),pickPos.getZ());
							btVector3 localPivot = body->getCenterOfMassTransform().inverse() * pickPos;
							btPoint2PointConstraint* p2p = new btPoint2PointConstraint(*body,localPivot);
							m_dynamicsWorld->addConstraint(p2p,true);
							m_pickedConstraint = p2p;
							btScalar mousePickClamping = 30.f;
							p2p->m_setting.m_impulseClamp = mousePickClamping;
							//very weak constraint for picking
							p2p->m_setting.m_tau = 0.001f;
						}
					} else
					{
						btMultiBodyLinkCollider* multiCol = (btMultiBodyLinkCollider*)btMultiBodyLinkCollider::upcast(rayCallback.m_collisionObject);
						if (multiCol && multiCol->m_multiBody)
						{
							multiCol->m_multiBody->setCanSleep(false);

							btVector3 pivotInA = multiCol->m_multiBody->worldPosToLocal(multiCol->m_link, pickPos);

							btMultiBodyPoint2Point* p2p = new btMultiBodyPoint2Point(multiCol->m_multiBody,multiCol->m_link,0,pivotInA,pickPos);
							//if you add too much energy to the system, causing high angular velocities, simulation 'explodes'
							//see also http://www.bulletphysics.org/Bullet/phpBB3/viewtopic.php?f=4&t=949
							//so we try to avoid it by clamping the maximum impulse (force) that the mouse pick can apply
							//it is not satisfying, hopefully we find a better solution (higher order integrator, using joint friction using a zero-velocity target motor with limited force etc?)

							p2p->setMaxAppliedImpulse(20*scaling);
		
							btMultiBodyDynamicsWorld* world = (btMultiBodyDynamicsWorld*) m_dynamicsWorld;
							world->addMultiBodyConstraint(p2p);
							m_pickingMultiBodyPoint2Point =p2p; 
						}
					}


//					pickObject(pickPos, rayCallback.m_collisionObject);
					m_oldPickingPos = rayTo;
					m_hitPos = pickPos;
					m_oldPickingDist  = (pickPos-rayFrom).length();
//					printf("hit !\n");
				//add p2p
				}
				
			}
		} else
		{
			if (button==0)
			{
				if (m_pickedConstraint)
				{
					m_dynamicsWorld->removeConstraint(m_pickedConstraint);
					delete m_pickedConstraint;
					m_pickedConstraint=0;
					m_pickedBody = 0;
				}

				if (m_pickingMultiBodyPoint2Point)
				{
					m_pickingMultiBodyPoint2Point->getMultiBodyA()->setCanSleep(true);
					btMultiBodyDynamicsWorld* world = (btMultiBodyDynamicsWorld*) m_dynamicsWorld;
					world->removeMultiBodyConstraint(m_pickingMultiBodyPoint2Point);
					delete m_pickingMultiBodyPoint2Point;
					m_pickingMultiBodyPoint2Point = 0;
				}
				//remove p2p
			}
		}

		//printf("button=%d, state=%d\n",button,state);
		return false;
	}

	void	stepSimulation()
	{
		m_dynamicsWorld->stepSimulation(1./60,0);
//		CProfileManager::dumpAll();
	}
};



BasicDemo* sDemo = 0;

static void MyMouseMoveCallback( float x, float y)
{
	bool handled = false;
	if (sDemo)
		handled = sDemo->mouseMoveCallback(x,y);
	if (!handled)
		b3DefaultMouseMoveCallback(x,y);
}
static void MyMouseButtonCallback(int button, int state, float x, float y)
{
	bool handled = false;
	//try picking first
	if (sDemo)
		handled = sDemo->mouseButtonCallback(button,state,x,y);

	if (!handled)
		b3DefaultMouseButtonCallback(button,state,x,y);
}


int main(int argc, char* argv[])
{
	
	float dt = 1./120.f;
#ifdef BT_DEBUG
	char* name = "Bullet 2 CPU FeatherstoneMultiBodyDemo (Debug build=SLOW)";
#else
	char* name = "Bullet 2 CPU FeatherstoneMultiBodyDemo";
#endif

	
	SimpleOpenGL3App* app = new SimpleOpenGL3App(name,1024,768);
	app->m_instancingRenderer->setCameraDistance(40);
	app->m_instancingRenderer->setCameraPitch(0);
	app->m_instancingRenderer->setCameraTargetPosition(b3MakeVector3(0,0,0));

	app->m_window->setMouseMoveCallback(MyMouseMoveCallback);
	app->m_window->setMouseButtonCallback(MyMouseButtonCallback);

	BasicDemo* demo = new BasicDemo(app);
	demo->initPhysics();
	sDemo = demo;

	GLint err = glGetError();
    assert(err==GL_NO_ERROR);
	
	do
	{
		GLint err = glGetError();
		assert(err==GL_NO_ERROR);
		app->m_instancingRenderer->init();
		app->m_instancingRenderer->updateCamera();
		
		demo->stepSimulation();
		demo->drawObjects();
		app->drawGrid(10,0.01);
		char bla[1024];
		static int frameCount = 0;
		frameCount++;
		sprintf(bla,"Simulation frame %d", frameCount);
		
		app->drawText(bla,10,10);
		app->swapBuffer();
	} while (!app->m_window->requestedExit());


	demo->exitPhysics();
	delete demo;

	delete app;
	return 0;
}
