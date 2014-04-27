//
//  LDPlayer.cpp
//  LD29
//
//  Created by Nils Daumann on 26.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#include "LDPlayer.h"
#include "LDWorld.h"

namespace LD
{
	RNDefineMeta(Player, RN::Entity)
	
	Player::Player(RN::Camera *camera) :
	_smoothDelta(0.016f)
	{
		_camera = camera->Retain();
		
		RN::Model *model = RN::Model::WithFile("Models/Robot/Robot.sgm");
		model->GetMaterialAtIndex(0, 2)->SetCullMode(RN::Material::CullMode::None);
		
		for(int i = 0; i < model->GetMeshCount(0); i++)
		{
			model->GetMaterialAtIndex(0, i)->Define("RN_SPECULARITY");
			model->GetMaterialAtIndex(0, i)->SetSpecularColor(RN::Color(0.5f, 0.5f, 0.5f, 120.0f));
			model->GetMaterialAtIndex(0, i)->SetAmbientColor(RN::Color::Black());
		}
		
		SetModel(model);
		_skeleton = GetSkeleton();
		_skeleton->Retain();
		_skeleton->SetAnimation("eat");
		
		SetScale(RN::Vector3(3.0f));
		
		RN::bullet::Shape *shape = RN::bullet::SphereShape::WithRadius(0.3f);
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
		if(!static_cast<World*>(RN::World::GetActiveWorld())->IsRunning())
			return;
		
		Entity::Update(delta);
		
		_smoothDelta = _smoothDelta * 0.9f + delta * 0.1f;
		
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
		Rotate((rotationX+rotationY) * 50.0f * _smoothDelta);
		
		_speed *= 1.0f - _smoothDelta;
		
		if(GetWorldPosition().y < 0.0f)
		{
			_controller->SetGravity(0.0f);
			
			_speed += GetForward() * accelerate * _smoothDelta;
			
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
		
		
		RN::Vector3 cameraTarget = GetWorldPosition() - GetForward() * 0.3f + GetUp() * 0.2f;
		RN::Vector3 targetDirection = cameraTarget - _camera->GetWorldPosition();
		_camera->Translate(targetDirection * std::max(std::min(_smoothDelta * 10.0f, 1.0f), 0.0f));
		
		_camera->SetWorldRotation(RN::Quaternion::WithLookAt(_camera->GetWorldPosition() - GetWorldPosition() - GetUp() * 0.1f, GetUp()));
		//_camera->Rotate(RN::Vector3(0.0f, 15.0f, 0.0f));
	}
}
