//
//  LDPlayer.cpp
//  LD29
//
//  Created by Nils Daumann on 26.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#include "LDPlayer.h"

namespace LD
{
	Player::Player()
	{
		RN::Model *model = RN::Model::WithFile("Models/Robot/Robot.sgm");
		model->GetMaterialAtIndex(0, 2)->SetCullMode(RN::Material::CullMode::None);
		
		for(int i = 0; i < model->GetMeshCount(0); i++)
		{
			model->GetMaterialAtIndex(0, i)->Define("RN_SPECULARITY");
		}
		
		SetModel(model);
		_skeleton = GetSkeleton();
		_skeleton->Retain();
		_skeleton->SetAnimation("eat");
		
		RN::bullet::Shape *shape = RN::bullet::SphereShape::WithRadius(2.0f);
		_body = new RN::bullet::RigidBody(shape, 1.0f);
		AddAttachment(_body);
	}
	
	Player::~Player()
	{
		_skeleton->Release();
		_body->Release();
	}
	
	void Player::Update(float delta)
	{
		_skeleton->Update(delta*24.0f);
		_body->SetAngularVelocity(RN::Vector3(0.0f));
	}
}
