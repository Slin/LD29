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
	RNDefineMeta(Player, RN::Entity)
	
	Player::Player(RN::Camera *camera)
	{
		_camera = camera->Retain();
		
		RN::Model *model = RN::Model::WithFile("Models/Robot/Robot.sgm");
		model->GetMaterialAtIndex(0, 2)->SetCullMode(RN::Material::CullMode::None);
		
		for(int i = 0; i < model->GetMeshCount(0); i++)
		{
			model->GetMaterialAtIndex(0, i)->Define("RN_SPECULARITY");
			model->GetMaterialAtIndex(0, i)->SetSpecularColor(RN::Color(0.5f, 0.5f, 0.5f, 120.0f));
		}
		
		SetModel(model);
		_skeleton = GetSkeleton();
		_skeleton->Retain();
		_skeleton->SetAnimation("eat");
		
		RN::bullet::Shape *shape = RN::bullet::SphereShape::WithRadius(0.2f);
		_controller = new RN::bullet::KinematicController(shape, 0.025f);
		_controller->SetJumpSpeed(0.0f);
		
		AddAttachment(_controller);
	}
	
	Player::~Player()
	{
		_camera->Release();
		_skeleton->Release();
		_controller->Release();
	}
	
	void Player::Update(float delta)
	{
		Entity::Update(delta);
		
		_skeleton->Update(delta*24.0f);
		
		RN::Input *input = RN::Input::GetSharedInstance();
		
		_turboCooldown -= delta;
		
		float accelerate = input->IsKeyPressed(' ');
		if(input->GetModifierKeys() & RN::KeyModifier::KeyShift)
		{
			if(_turboCooldown <= 0.0f)
				_turboCooldown = 6.0f;
			
			if(_turboCooldown > 2.0f)
				accelerate *= 2.0f;
		}
		
		RN::Vector3 rotationX(input->IsKeyPressed('a')-input->IsKeyPressed('d'), 0.0f, 0.0f);
		RN::Vector3 rotationY(0.0f, input->IsKeyPressed('s')-input->IsKeyPressed('w'), 0.0f);
		Rotate((rotationX+rotationY) * 50.0f * delta);
		
		_speed *= 1.0f - delta;
		
		if(GetWorldPosition().y < 0.0f)
		{
			_controller->SetGravity(0.0f);
			
			_speed += GetForward() * accelerate * delta;
			
			float currentMaxSpeed = 0.1f * std::max((_turboCooldown-2.0f)*0.75f, 1.0f);
			if(_speed.GetLength() > currentMaxSpeed)
			{
				_speed.Normalize();
				_speed *= currentMaxSpeed;
			}
		}
		else
		{
			_controller->SetGravity(9.81f);
		}
		
		_controller->SetWalkDirection(_speed);
		
		
		RN::Vector3 cameraTarget = GetWorldPosition() - GetForward() * 0.125f + GetUp() * 0.075f;
		RN::Vector3 targetDirection = cameraTarget - _camera->GetWorldPosition();
		_camera->Translate(targetDirection);//std::max(std::min(delta * 10.0f, 0.5f), 0.0f));
		
		_camera->SetWorldRotation(RN::Quaternion::WithLookAt(_camera->GetWorldPosition() - GetWorldPosition() - GetUp() * 0.05f, GetUp()));
		//_camera->Rotate(RN::Vector3(0.0f, 15.0f, 0.0f));
	}
}
