//
//  LDDirt.cpp
//  LD29
//
//  Created by Nils Daumann on 27.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#include "LDDirt.h"
#include "LDWorld.h"

namespace LD
{
	RNDefineMeta(Dirt, RN::Entity)
	
	Dirt::Dirt() :
	_player(nullptr),
	_didCount(false),
	_isIntro(false)
	{
		RN::Model *model = RN::Model::WithFile("Models/Dirt/Dirt.sgm");
		SetModel(model);
		SetScale(RN::Vector3(5.0f));
		
		_body = new RN::bullet::RigidBody(RN::bullet::SphereShape::WithRadius(0.5f), 0.5f);
		AddAttachment(_body);
		
		if(!static_cast<World*>(RN::World::GetActiveWorld())->IsRunning())
		{
			_isIntro = true;
		}
	}
	
	Dirt::~Dirt()
	{
		SafeRelease(_player);
		_body->Release();
	}
	
	void Dirt::Update(float delta)
	{
		if(GetWorldPosition().y < 0.0f)
		{
			_body->SetDamping(0.2f, 0.1f);
			_body->SetGravity(RN::Vector3(-0.8f, -0.3f, 0.0f));
			
			if(_isIntro)
			{
				RemoveFromWorld();
				return;
			}
		}
		
		if(_player)
		{
			if(_player->GetWorldPosition().GetDistance(GetWorldPosition()) < 1.0f)
			{
				RemoveFromWorld();
			}
		}
		
		if(!_didCount && GetWorldPosition().x < -290.0f)
		{
			static_cast<World*>(RN::World::GetActiveWorld())->MakeDirty();
			_didCount = true;
		}
	}
	
	void Dirt::Throw(RN::Vector3 impulse)
	{
		_body->ApplyImpulse(impulse);
	}
	
	void Dirt::SetPlayer(LD::Player *player)
	{
		_player = player->Retain();
	}
}