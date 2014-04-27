//
//  LDSpawner.cpp
//  LD29
//
//  Created by Nils Daumann on 27.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#include "LDSpawner.h"
#include "LDWorld.h"

namespace LD
{
	RNDefineMeta(Spawner, RN::SceneNode)
	
	Spawner::Spawner() :
	_time(0.0f),
	_player(nullptr)
	{
		_rng = new RN::RandomNumberGenerator(RN::RandomNumberGenerator::Type::MersenneTwister);
	}
	
	Spawner::~Spawner()
	{
		_player->Release();
	}
	
	void Spawner::Update(float delta)
	{
		_time -= delta;
		
		if(_time <= 0.0f)
		{
			Dirt *dirt = new Dirt();
			dirt->SetPosition(GetWorldPosition()+RN::Vector3(0.0f, 0.0f, _rng->RandomFloatRange(-60.0f, 10.0f)));
			dirt->Throw(RN::Vector3(-5.0f, 10.0f, 0.0f));
			if(_player)
				dirt->SetPlayer(_player);
			dirt->Release();
			
			if(!static_cast<World*>(RN::World::GetActiveWorld())->IsRunning())
				_time = _rng->RandomFloatRange(0.5f, 1.0f);
			else
				_time = _rng->RandomFloatRange(2.0f, 5.0f);
		}
	}
	
	void Spawner::SetPlayer(Player *player)
	{
		_player = player;
		_player->Retain();
	}
}