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
		_camera = new RN::Camera(RN::Vector2(), RN::Texture::Format::RGB888, RN::Camera::Flags::Defaults);
	}

	void World::Update(float delta)
	{
		// Do something...
	}
}
