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
		
		
		RN::Color start(3.0f, 3.0f, 3.0f, 0.3f);
		RN::Color end(3.0f, 3.0f, 3.0f, 0.0f);
		
		_forwardEmitter = new RN::GenericParticleEmitter();
		_forwardEmitter->SetRenderGroup(2);
		_forwardEmitter->SetSpawnRate(0.02f);
		_forwardEmitter->SetLifeSpan(RN::Vector2());
		_forwardEmitter->SetStartColor(start);
		_forwardEmitter->SetEndColor(end);
		_forwardEmitter->SetStartSize(RN::Vector2(0.0f, 0.0f));
		_forwardEmitter->SetEndSize(RN::Vector2(0.005f, 0.015f));
		_forwardEmitter->SetGravity(RN::Vector3());
		_forwardEmitter->SetVelocityRandomizeMax(RN::Vector3(0.01f, 0.01f, 0.01f));
		_forwardEmitter->SetVelocityRandomizeMin(RN::Vector3(-0.01f, 0.0f, -0.01f));
		_forwardEmitter->SetPositionRandomizeMax(RN::Vector3());
		_forwardEmitter->SetPositionRandomizeMin(RN::Vector3());
		_forwardEmitter->SetPosition(RN::Vector3(0.0f, 0.0f, 0.012f));
		_forwardEmitter->SetRotation(RN::Vector3(0.0f, 90.0f, 0.0f));
		//_forwardEmitter->SetIsRenderedInversed(true);
		
		_leftEmitter = new RN::GenericParticleEmitter();
		_leftEmitter->SetRenderGroup(2);
		_leftEmitter->SetSpawnRate(0.02f);
		_leftEmitter->SetLifeSpan(RN::Vector2());
		_leftEmitter->SetStartColor(start);
		_leftEmitter->SetEndColor(end);
		_leftEmitter->SetStartSize(RN::Vector2(0.0f, 0.0f));
		_leftEmitter->SetEndSize(RN::Vector2(0.005f, 0.015f));
		_leftEmitter->SetGravity(RN::Vector3());
		_leftEmitter->SetVelocityRandomizeMax(RN::Vector3(0.01f, 0.01f, 0.01f));
		_leftEmitter->SetVelocityRandomizeMin(RN::Vector3(-0.01f, 0.0f, -0.01f));
		_leftEmitter->SetPositionRandomizeMax(RN::Vector3());
		_leftEmitter->SetPositionRandomizeMin(RN::Vector3());
		_leftEmitter->SetPosition(RN::Vector3(0.012f, 0.001f, -0.001f));
		_leftEmitter->SetRotation(RN::Vector3(90.0f, 90.0f, 0.0f));
		
		_rightEmitter = new RN::GenericParticleEmitter();
		_rightEmitter->SetRenderGroup(2);
		_rightEmitter->SetSpawnRate(0.02f);
		_rightEmitter->SetLifeSpan(RN::Vector2());
		_rightEmitter->SetStartColor(start);
		_rightEmitter->SetEndColor(end);
		_rightEmitter->SetStartSize(RN::Vector2(0.0f, 0.0f));
		_rightEmitter->SetEndSize(RN::Vector2(0.005f, 0.015f));
		_rightEmitter->SetGravity(RN::Vector3());
		_rightEmitter->SetVelocityRandomizeMax(RN::Vector3(0.01f, 0.01f, 0.01f));
		_rightEmitter->SetVelocityRandomizeMin(RN::Vector3(-0.01f, 0.0f, -0.01f));
		_rightEmitter->SetPositionRandomizeMax(RN::Vector3());
		_rightEmitter->SetPositionRandomizeMin(RN::Vector3());
		_rightEmitter->SetPosition(RN::Vector3(-0.012f, 0.001f, -0.001f));
		_rightEmitter->SetRotation(RN::Vector3(-90.0f, 90.0f, 0.0f));
		
		_upEmitter = new RN::GenericParticleEmitter();
		_upEmitter->SetRenderGroup(2);
		_upEmitter->SetSpawnRate(0.02f);
		_upEmitter->SetLifeSpan(RN::Vector2());
		_upEmitter->SetStartColor(start);
		_upEmitter->SetEndColor(end);
		_upEmitter->SetStartSize(RN::Vector2(0.0f, 0.0f));
		_upEmitter->SetEndSize(RN::Vector2(0.005f, 0.015f));
		_upEmitter->SetGravity(RN::Vector3());
		_upEmitter->SetVelocityRandomizeMax(RN::Vector3(0.01f, 0.01f, 0.01f));
		_upEmitter->SetVelocityRandomizeMin(RN::Vector3(-0.01f, 0.0f, -0.01f));
		_upEmitter->SetPositionRandomizeMax(RN::Vector3());
		_upEmitter->SetPositionRandomizeMin(RN::Vector3());
		_upEmitter->SetPosition(RN::Vector3(0.0f, -0.012f, 0.0f));
		_upEmitter->SetRotation(RN::Vector3(0.0f, -180.0f, 0.0f));
		
		_downEmitter = new RN::GenericParticleEmitter();
		_downEmitter->SetRenderGroup(2);
		_downEmitter->SetSpawnRate(0.02f);
		_downEmitter->SetLifeSpan(RN::Vector2());
		_downEmitter->SetStartColor(start);
		_downEmitter->SetEndColor(end);
		_downEmitter->SetStartSize(RN::Vector2(0.0f, 0.0f));
		_downEmitter->SetEndSize(RN::Vector2(0.005f, 0.015f));
		_downEmitter->SetGravity(RN::Vector3());
		_downEmitter->SetVelocityRandomizeMax(RN::Vector3(0.01f, 0.01f, 0.01f));
		_downEmitter->SetVelocityRandomizeMin(RN::Vector3(-0.01f, 0.0f, -0.01f));
		_downEmitter->SetPositionRandomizeMax(RN::Vector3());
		_downEmitter->SetPositionRandomizeMin(RN::Vector3());
		_downEmitter->SetPosition(RN::Vector3(0.0f, 0.012f, 0.0f));
		_downEmitter->SetRotation(RN::Vector3(0.0f, 0.0f, 0.0f));
		
		AddChild(_forwardEmitter);
		AddChild(_leftEmitter);
		AddChild(_rightEmitter);
		AddChild(_upEmitter);
		AddChild(_downEmitter);
	}
	
	Player::~Player()
	{
		_camera->Release();
		_skeleton->Release();
		RemoveAttachment(_controller);
		_controller->Release();
		_forwardEmitter->Release();
		_leftEmitter->Release();
		_rightEmitter->Release();
		_upEmitter->Release();
		_downEmitter->Release();
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
		
		_rotationSpeed *= 1.0f - _smoothDelta*2.0f;
		_rotationSpeed += (rotationX+rotationY) * 200.0f * _smoothDelta;
		
		if(_rotationSpeed.GetLength() > 80.0f)
		{
			_rotationSpeed.Normalize();
			_rotationSpeed *= 80.0f;
		}
		
		Rotate(_rotationSpeed * _smoothDelta);
		
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
		
		//Set particles according to movement
		if(accelerate > RN::k::EpsilonFloat)
		{
			_forwardEmitter->SetLifeSpan(RN::Vector2(0.5f, 1.0f));
			if(accelerate < 1.0f + RN::k::EpsilonFloat)
			{
				_forwardEmitter->SetSpawnRate(0.02f);
				_forwardEmitter->SetVelocity(RN::Vector3(0.0f, 0.02f, 0.0f));
			}
			else
			{
				_forwardEmitter->SetSpawnRate(0.01f);
				_forwardEmitter->SetVelocity(RN::Vector3(0.0f, 0.04f, 0.0f));
			}
		}
		else
		{
			_forwardEmitter->SetLifeSpan(RN::Vector2(0.0f, 0.0f));
		}
		
		if(rotationX.x > RN::k::EpsilonFloat)
		{
			_rightEmitter->SetLifeSpan(RN::Vector2());
			_leftEmitter->SetLifeSpan(RN::Vector2(0.3f, 0.8f));
			_leftEmitter->SetSpawnRate(0.01f);
			_leftEmitter->SetVelocity(RN::Vector3(0.0f, 0.02f, 0.0f));
		}
		else
		{
			_leftEmitter->SetLifeSpan(RN::Vector2());
			
			if(rotationX.x < -RN::k::EpsilonFloat)
			{
				_rightEmitter->SetLifeSpan(RN::Vector2(0.3f, 0.8f));
				_rightEmitter->SetSpawnRate(0.01f);
				_rightEmitter->SetVelocity(RN::Vector3(0.0f, 0.02f, 0.0f));
			}
			else
			{
				_rightEmitter->SetLifeSpan(RN::Vector2());
			}
		}
		
		if(rotationY.y > RN::k::EpsilonFloat)
		{
			_downEmitter->SetLifeSpan(RN::Vector2());
			_upEmitter->SetLifeSpan(RN::Vector2(0.3f, 0.8f));
			_upEmitter->SetSpawnRate(0.01f);
			_upEmitter->SetVelocity(RN::Vector3(0.0f, 0.02f, 0.0f));
		}
		else
		{
			_upEmitter->SetLifeSpan(RN::Vector2());
			
			if(rotationY.y < -RN::k::EpsilonFloat)
			{
				_downEmitter->SetLifeSpan(RN::Vector2(0.3f, 0.8f));
				_downEmitter->SetSpawnRate(0.01f);
				_downEmitter->SetVelocity(RN::Vector3(0.0f, 0.02f, 0.0f));
			}
			else
			{
				_downEmitter->SetLifeSpan(RN::Vector2());
			}
		}
		
		RN::Vector3 cameraOffset = RN::Vector3(0.0f, 0.2f, 0.3f) * (1.0f + _speed.GetLength() * 8.0f);
		cameraOffset = (GetWorldRotation()-_rotationSpeed*0.1f).GetRotatedVector(cameraOffset);
		RN::Vector3 cameraTarget = GetWorldPosition() + cameraOffset;
		//RN::Vector3 targetDirection = cameraTarget - _camera->GetWorldPosition();
		_camera->SetPosition(cameraTarget);
		//_camera->Translate(targetDirection * std::max(std::min(_smoothDelta * 10.0f, 1.0f), 0.0f));
		
		_camera->SetWorldRotation(RN::Quaternion::WithLookAt(_camera->GetWorldPosition() - GetWorldPosition() - GetUp() * 0.1f, GetUp()));
		//_camera->Rotate(RN::Vector3(0.0f, 15.0f, 0.0f));
		
		//_forwardEmitter->SetPosition();
	}
}
