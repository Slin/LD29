//
//  LDDirt.cpp
//  LD29
//
//  Created by Nils Daumann on 27.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#include "LDDirt.h"

namespace LD
{
	RNDefineMeta(Dirt, RN::Entity)
	
	Dirt::Dirt() :
	_player(nullptr)
	{
		RN::Model *model = RN::Model::WithFile("Models/Dirt/Dirt.sgm");
		SetModel(model);
		SetScale(RN::Vector3(5.0f));
		
		_body = new RN::bullet::RigidBody(RN::bullet::SphereShape::WithRadius(0.5f), 0.5f);
		AddAttachment(_body);
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
			_body->ApplyImpulse(RN::Vector3(0.0f, 4.9f, 0.0f) * delta);
		}
		
		if(_player)
		{
			if(_player->GetWorldPosition().GetDistance(GetWorldPosition()) < 1.0f)
			{
				RemoveFromWorld();
			}
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