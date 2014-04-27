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
		RN::World("OctreeSceneManager"),
		_player(nullptr)
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
		_water->Release();
		_waterCamera->Release();
		_refractCamera->Release();
		
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
		
		RN::RenderStorage *storage = new RN::RenderStorage(RN::RenderStorage::BufferFormatComplete);
		storage->AddRenderTarget(RN::Texture::Format::RGB16F);
		storage->SetDepthTarget(RN::Texture::Format::Depth24I);
		
		_camera = new RN::Camera(RN::Vector2(), storage, RN::Camera::Flags::Defaults);
		_camera->SetDebugName("Main");
		_camera->SetFlags(_camera->GetFlags() | RN::Camera::Flags::NoFlush);
		_camera->SceneNode::SetFlags(_camera->SceneNode::GetFlags() | RN::SceneNode::Flags::NoSave);
		_camera->SetSky(sky);
		_camera->SetClipFar(10000.0f);
		_camera->SetClipNear(0.01f);
		_camera->SetFogFar(1500.0f);
		_camera->SetFogNear(0.0f);
		
		RN::PostProcessingPipeline *waterPipeline = _camera->AddPostProcessingPipeline("water", 0);
		
		RN::Material *refractCopy = new RN::Material(RN::Shader::WithFile("shader/rn_PPCopy"));
		refractCopy->Define("RN_COPYDEPTH");
		_refractCamera = new RN::Camera(_camera->GetFrame().Size(), RN::Texture::Format::RGBA32F, RN::Camera::Flags::UpdateStorageFrame, RN::RenderStorage::BufferFormatColor);
		_refractCamera->SetMaterial(refractCopy);
		_refractCamera->SceneNode::SetFlags(_camera->SceneNode::GetFlags() | RN::SceneNode::Flags::NoSave);
		waterPipeline->AddStage(_refractCamera, RN::RenderStage::Mode::ReUsePreviousStage);
		
		_waterCamera = new RN::Camera(RN::Vector2(_camera->GetFrame().Size()), storage, RN::Camera::Flags::Defaults);
		_waterCamera->SetClearMask(0);
		_waterCamera->SetRenderGroups(RN::Camera::RenderGroups::Group2);
		_waterCamera->SetDebugName("Water");
		_waterCamera->SceneNode::SetFlags(_waterCamera->SceneNode::GetFlags() | RN::SceneNode::Flags::NoSave);
		_waterCamera->SetClipFar(_camera->GetClipFar());
		_waterCamera->SetClipNear(_camera->GetClipNear());
		_waterCamera->SetFogFar(_camera->GetFogFar());
		_waterCamera->SetFogNear(_camera->GetFogNear());
		
		_waterCamera->SetBlitShader(RN::Shader::WithFile("shader/rn_DrawFramebufferTonemap"));
		_waterCamera->SetPriority(-100);
		_camera->AddChild(_waterCamera);
		
		RN::Renderer::GetSharedInstance()->SetHDRExposure(1.0f);
		RN::Renderer::GetSharedInstance()->SetHDRWhitePoint(5.0f);
		
		_sun = new RN::Light(RN::Light::Type::DirectionalLight);
		_sun->ActivateShadows(RN::ShadowParameter(_camera, 2048));
		_sun->SetRotation(RN::Vector3(45.0f, -60.0f, 0.0f));
		
		// Create static level objects
		RN::Model *model = RN::Model::WithFile("Models/Ground/Ground.sgm");
		RN::Entity *ground = new RN::Entity(model);
		model->GetMaterialAtIndex(0, 0)->SetShader(RN::Shader::WithFile("Shaders/Ground"));
		model->GetMaterialAtIndex(0, 0)->Define("RN_TEXTURE_TILING 100.0");
		model->GetMaterialAtIndex(0, 0)->SetDiffuseColor(RN::Color(0.6f, 0.6f, 0.5f));
		model->GetMaterialAtIndex(0, 0)->SetOverride(model->GetMaterialAtIndex(0, 0)->GetOverride() | RN::Material::Override::Shader);
		ground->SetPosition(RN::Vector3(0.0f, 2.0f, 0.0f));
		ground->Release();
		RN::bullet::Shape *shape = RN::bullet::TriangleMeshShape::WithModel(model);
		RN::bullet::RigidBody *body = new RN::bullet::RigidBody(shape, 0.0f);
		ground->AddAttachment(body);
		body->Release();
		
		_water = new RN::Water(_camera, RN::Texture::WithFile("Textures/waterbump.png", true), _refractCamera->GetStorage()->GetRenderTarget());
		_water->SetWorldPosition(RN::Vector3(0.0f, 0.0f, 0.0f));
		_water->SetScale(RN::Vector3(50.0f));
		_water->SetFlags(_water->GetFlags() | RN::SceneNode::Flags::NoSave);
	
		RN::Entity *factory = new RN::Entity(RN::Model::WithFile("Models/Factory/Factory.sgm"));
		factory->SetPosition(RN::Vector3(0.0f, 2.0f, 0.0f));
		factory->Release();
		
		//Create spawner
		Spawner *spawner = new Spawner();
		spawner->SetWorldPosition(RN::Vector3(0.0f, 10.0f, 0.0f));
		spawner->Release();
		
		//Create player
		_player = new Player(_camera);
		_player->SetPosition(RN::Vector3(-100.0f, 0.0f, 0.0f));
		spawner->SetPlayer(_player);
	}

	void World::Update(float delta)
	{
		if(_camera->GetWorldPosition().y < _water->GetWorldPosition().y)
		{
			_camera->SetFogFar(50.0f);
			_waterCamera->SetFogFar(50.0f);
			_camera->SetFogColor(RN::Color(0.5f, 0.5f, 0.6f) * 0.5f);
			_waterCamera->SetFogColor(RN::Color(0.5f, 0.5f, 0.6f) * 0.5f);
		}
		else
		{
			_camera->SetFogFar(1500.0f);
			_waterCamera->SetFogFar(1500.0f);
			_camera->SetFogColor(RN::Color(1.0f, 1.0f, 1.0f));
			_waterCamera->SetFogColor(RN::Color(1.0f, 1.0f, 1.0f));
		}
		
		if(!_player)
		{
			RN::Input *input = RN::Input::GetSharedInstance();
			
			RN::Vector3 translation;
			RN::Vector3 rotation;
			
			const RN::Vector2& mouseDelta = input->GetMouseDelta();
			
			if(!(input->GetModifierKeys() & RN::KeyModifier::KeyControl))
			{
				rotation.x = mouseDelta.x;
				rotation.y = mouseDelta.y;
			}
			
			translation.x = (input->IsKeyPressed('d') - input->IsKeyPressed('a')) * 30.0f;
			translation.z = (input->IsKeyPressed('s') - input->IsKeyPressed('w')) * 30.0f;
			
			translation *= (input->GetModifierKeys() & RN::KeyModifier::KeyShift) ? 2.0f : 1.0f;
			_camera->Rotate(rotation);
			_camera->TranslateLocal(translation * delta);
		}
	}
}
