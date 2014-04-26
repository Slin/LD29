//
//  LDWorld.cpp
//  LD29
//
//  Created by Nils Daumann on 26.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#include "LDWorld.h"

namespace LD
{
	World::World() :
		RN::World("OctreeSceneManager")
	{
		RN::MessageCenter::GetSharedInstance()->AddObserver(kRNInputEventMessage, [&](RN::Message *message) {
			
			RN::Event *event = static_cast<RN::Event *>(message);
			HandleInputEvent(event);
			
		}, this);
		
		_physicsWorld = new RN::bullet::PhysicsWorld();
		
		AddAttachment(_physicsWorld);
	}
	
	World::~World()
	{
		_player->Release();
		_camera->Release();
		_physicsWorld->Release();
		RN::MessageCenter::GetSharedInstance()->RemoveObserver(this);
	}
	
	void World::HandleInputEvent(RN::Event *event)
	{
		if(event->GetType() == RN::Event::Type::KeyDown)
		{
			switch(event->GetCharacter())
			{
				case '0':
				{
					RN::MessageCenter::GetSharedInstance()->PostMessage(RNCSTR("DPToggle"), nullptr, nullptr);
					break;
				}
					
				default:
					break;
			}
		}
	}
			
	
	void World::LoadOnThread(RN::Thread *thread, RN::Deserializer *deserializer)
	{
		RN::Model *sky = RN::Model::WithSkyCube();
		for(int i = 0; i < 6; i++)
		{
			sky->GetMaterialAtIndex(0, i)->SetAmbientColor(RN::Color(6.0f, 6.0f, 6.0f, 1.0f));
			sky->GetMaterialAtIndex(0, i)->SetOverride(RN::Material::Override::Shader);
			sky->GetMaterialAtIndex(0, i)->Define("RN_ATMOSPHERE");
		}
		
		_camera = new RN::Camera(RN::Vector2(), RN::Texture::Format::RGB16F, RN::Camera::Flags::Defaults);
		_camera->SetSky(sky);
		_camera->SetBlitShader(RN::Shader::WithFile("shader/rn_DrawFramebufferTonemap"));
		_camera->SetPosition(RN::Vector3(0.0f, 8.0f, 5.0f));
		_camera->SetRotation(RN::Vector3(0.0f, -30.0f, 0.0f));
		
		RN::Renderer::GetSharedInstance()->SetHDRExposure(1.0f);
		RN::Renderer::GetSharedInstance()->SetHDRWhitePoint(5.0f);
		
		_sun = new RN::Light(RN::Light::Type::DirectionalLight);
		_sun->ActivateShadows(RN::ShadowParameter(_camera));
		_sun->SetRotation(RN::Vector3(45.0f, -60.0f, 0.0f));
		
		// Create static level objects
		RN::Model *model = RN::Model::WithFile("Models/Ground/Ground.sgm");
		RN::Entity *ground = new RN::Entity(model);
		ground->GetModel()->GetMaterialAtIndex(0, 0)->SetShader(RN::Shader::WithFile("Shaders/Ground"));
		ground->GetModel()->GetMaterialAtIndex(0, 0)->Define("RN_TEXTURE_TILING 100.0");
		ground->GetModel()->GetMaterialAtIndex(0, 0)->SetDiffuseColor(RN::Color(0.6f, 0.6f, 0.5f));
		ground->Release();
		RN::bullet::Shape *shape = RN::bullet::TriangleMeshShape::WithModel(model);
		RN::bullet::RigidBody *body = new RN::bullet::RigidBody(shape, 0.0f);
		ground->AddAttachment(body);
		body->Release();
		
		RN::Entity *factory = new RN::Entity(RN::Model::WithFile("Models/Factory/Factory.sgm"));
		factory->Release();
		
		//Create player
		_player = new Player();
		_player->AddChild(_camera);
		_player->SetPosition(RN::Vector3(-100.0f, 0.0f, 0.0f));
	}

	void World::Update(float delta)
	{
		// Do something...
	}
}
