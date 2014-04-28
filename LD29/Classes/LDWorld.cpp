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
		RN::World("GenericSceneManager"),
		_player(nullptr),
		_dirtCounter(0),
		_time(0.0f),
		_isIntro(true),
		_isRunning(false),
		_clockWidget(nullptr)
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
		_clockLabel->Release();
		_clockWidget->Release();
		_loseWidget->Release();
		_startWidget->Release();
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
		_camera->SetClipNear(0.1f);
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
		RN::ShadowParameter shadowParam(_camera, 2048);
		shadowParam.distanceBlendFactor = 0.01f;
		shadowParam.splits[1].updateInterval = 1;
		shadowParam.splits[2].updateInterval = 1;
		shadowParam.splits[3].updateInterval = 1;
		_sun->ActivateShadows(shadowParam);
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
		
		_water = new RN::Water(_camera, RN::Texture::WithFile("Textures_/waterbump.png", true), _refractCamera->GetStorage()->GetRenderTarget());
		_water->SetWorldPosition(RN::Vector3(0.0f, 0.0f, 0.0f));
		_water->SetScale(RN::Vector3(30.0f));
		_water->SetFlags(_water->GetFlags() | RN::SceneNode::Flags::NoSave);
	
		RN::Entity *factory = new RN::Entity(RN::Model::WithFile("Models/Factory/Factory.sgm"));
		factory->SetPosition(RN::Vector3(0.0f, 2.0f, 0.0f));
		factory->Release();
		
		factory = new RN::Entity(RN::Model::WithFile("Models/Factory/FactoryThingy.sgm"));
		factory->SetPosition(RN::Vector3(0.0f, 2.0f, 0.0f));
		factory->Release();
		
		factory = new RN::Entity(RN::Model::WithFile("Models/Robot/Robot.sgm"));
		factory->SetPosition(RN::Vector3(21.0f, 17.0f, 12.0f));
		factory->SetRotation(RN::Vector3(-45.0f, 0.0f, 0.0f));
		factory->SetScale(RN::Vector3(350.0f));
		factory->GetSkeleton()->SetAnimation("eat");
		factory->SetAction([](RN::SceneNode *ent, float delta){
			ent->Downcast<RN::Entity>()->GetSkeleton()->Update(delta * 24.0f);
		});
		factory->Release();
		
		//Create smoke particles
		RN::GenericParticleEmitter *emitter = new RN::GenericParticleEmitter();
		emitter->SetSpawnRate(0.5f);
		emitter->SetStartColor(RN::Color(3.0f, 3.0f, 3.0f, 1.0f));
		emitter->SetEndColor(RN::Color(3.0f, 3.0f, 3.0f, 0.0f));
		emitter->SetEndSize(RN::Vector2(5.0f, 10.0f));
		emitter->SetLifeSpan(RN::Vector2(10.0f, 15.0f));
		emitter->SetGravity(RN::Vector3());
		emitter->SetVelocity(RN::Vector3(0.0f, 1.0f, 0.0f));
		emitter->SetVelocityRandomizeMax(RN::Vector3(0.5f, 0.5f, 0.5f));
		emitter->SetVelocityRandomizeMin(RN::Vector3(-0.5f, 0.0f, -0.5f));
		emitter->SetPosition(RN::Vector3(-6.0f, 48.0f, 8.5f));
		
		RN::GenericParticleEmitter *otherEmitter = new RN::GenericParticleEmitter(emitter);
		otherEmitter->SetPosition(RN::Vector3(-6.0f, 48.0f, -19.5f));
		otherEmitter->Release();
		
		otherEmitter = new RN::GenericParticleEmitter(emitter);
		otherEmitter->SetPosition(RN::Vector3(-6.0f, 48.0f, -44.5f));
		otherEmitter->Release();
		
		otherEmitter = new RN::GenericParticleEmitter(emitter);
		otherEmitter->SetMaxParticles(50);
		otherEmitter->SetPosition(RN::Vector3(-6.0f, 48.0f, -65.0f));
		otherEmitter->Release();
		
		emitter->Release();
		
		//Create houses and trees
		RN::Random::MersenneTwister random;
		random.Seed(0x1024);
		
		RN::InstancingNode *treeNode = new RN::InstancingNode();
		treeNode->SetModels(RN::Array::WithObjects(RN::Model::WithFile("Models/Tree/Tree.sgm"), nullptr));
		treeNode->SetPivot(_camera);
		treeNode->SetFlags(treeNode->GetFlags() | RN::SceneNode::Flags::NoSave);
		treeNode->Autorelease();
		
		for(int i = 0; i < 500; i++)
		{
			RN::Vector3 pos(random.RandomFloatRange(-300.0f, -500.0f), 10.0f, random.RandomFloatRange(-500.0f, 300.0f));
			
			RN::Hit hit = _physicsWorld->CastRay(pos, RN::Vector3(pos.x, -500.0f, pos.z));
			if(hit.distance <= 0.0f || hit.position.y < 0.0f)
				continue;
			
			pos = hit.position;
			
			RN::Entity *entity = new RN::Entity(RN::Model::WithFile("Models/Tree/Tree.sgm"), pos);
			entity->SetFlags(entity->GetFlags() | RN::SceneNode::Flags::Static | RN::SceneNode::Flags::NoSave);
			entity->SetScale(RN::Vector3(random.RandomFloatRange(0.89f, 1.12f)));
			treeNode->AddChild(entity);
			entity->Release();
		}
		
		RN::InstancingNode *houseNode = new RN::InstancingNode();
		houseNode->SetModels(RN::Array::WithObjects(RN::Model::WithFile("Models/House/House.sgm"), nullptr));
		houseNode->SetPivot(_camera);
		houseNode->SetFlags(houseNode->GetFlags() | RN::SceneNode::Flags::NoSave);
		houseNode->Autorelease();
		
		for(int i = 0; i < 500; i++)
		{
			RN::Vector3 pos(random.RandomFloatRange(0.0f, 500.0f), 2.0f, random.RandomFloatRange(-500.0f, 500.0f));
			
			if(pos.GetLength() < 200.0f)
				continue;
			
			RN::Entity *entity = new RN::Entity(RN::Model::WithFile("Models/House/House.sgm"), pos);
			entity->SetFlags(entity->GetFlags() | RN::SceneNode::Flags::Static | RN::SceneNode::Flags::NoSave);
			entity->SetScale(RN::Vector3(1.2f));
			houseNode->AddChild(entity);
			entity->Release();
		}
		
		RN::InstancingNode *algaeNode = new RN::InstancingNode();
		RN::Model *algaeModels[2];
		algaeModels[0] = RN::Model::WithFile("Models/Algae/Algae.sgm");
		algaeModels[1] = RN::Model::WithFile("Models/Stone/Stone.sgm");
		algaeModels[0]->GetMaterialAtIndex(0, 0)->SetCullMode(RN::Material::CullMode::None);
		algaeNode->SetModels(RN::Array::WithObjects(algaeModels[0], algaeModels[1], nullptr));
		algaeNode->SetPivot(_camera);
		algaeNode->SetFlags(algaeNode->GetFlags() | RN::SceneNode::Flags::NoSave);
		algaeNode->Autorelease();
		
		for(int i = 0; i < 1000; i++)
		{
			RN::Vector3 pos(random.RandomFloatRange(-100.0f, -250.0f), 2.0f, random.RandomFloatRange(-500.0f, 500.0f));
			
			RN::Hit hit = _physicsWorld->CastRay(pos, RN::Vector3(pos.x, -500.0f, pos.z));
			if(hit.distance <= 10.0f)
				continue;
			
			pos = hit.position;
			
			RN::Entity *entity = new RN::Entity(algaeModels[random.RandomInt32Range(0, 2)], pos);
			entity->SetFlags(entity->GetFlags() | RN::SceneNode::Flags::Static | RN::SceneNode::Flags::NoSave);
			entity->SetScale(RN::Vector3(random.RandomFloatRange(0.89f, 1.12f)));
			entity->SetRotation(RN::Vector3(random.RandomFloatRange(0.0f, 360.0f), 0.0f, 0.0f));
			algaeNode->AddChild(entity);
			entity->Release();
		}
		
		//Create spawner
		Spawner *spawner = new Spawner();
		spawner->SetWorldPosition(RN::Vector3(0.0f, 10.0f, 0.0f));
		spawner->Release();
		
		//Create player
		_player = new Player(_camera);
		_player->SetPosition(RN::Vector3(-100.0f, -10.0f, 0.0f));
		_camera->SetPosition(_player->GetWorldPosition());
		spawner->SetPlayer(_player);
		
		//Create UI
		RN::Kernel::GetSharedInstance()->ScheduleFunction([&](){
			if(!_clockWidget)
			{
				_clockWidget = new RN::UI::Widget(RN::UI::Widget::Style::Borderless, RN::Rect(10.0f, 50.0f, 180.0f, 100.0f));
				_clockLabel = new RN::UI::Label();
				_clockLabel->SetAutoresizingMask(RN::UI::View::AutoresizingMask::FlexibleHeight | RN::UI::View::AutoresizingMask::FlexibleWidth);
				_clockLabel->SetNumberOfLines(1);
				_clockLabel->SetFrame(RN::Rect(0.0f, 0.0f, _clockWidget->GetFrame().Size().x, _clockWidget->GetFrame().Size().y).Inset(5.0f, 5.0f));
				_clockWidget->GetContentView()->AddSubview(_clockLabel);
				_clockWidget->SetCanBecomeKeyWidget(false);
				_clockWidget->SetWidgetLevel(-10000);
				
				_loseWidget = new RN::UI::Widget(RN::UI::Widget::Style::Borderless, RN::Rect(0.0f, 0.0f, 512, 512));
				_loseWidget->Center();
				RN::UI::Label *label = new RN::UI::Label();
				label->SetText(RNSTR("The factory won!"));
				label->SetFrame(_clockWidget->GetContentView()->GetBounds());
				label->SetAlignment(RN::UI::TextAlignment::Center);
				label->SetTextColor(RN::Color::Yellow());
				RN::UI::FontDescriptor fontDescriptor;
				fontDescriptor.style = RN::UI::FontDescriptor::FontStyleBold;
				RN::UI::Font *font = RN::UI::Font::WithNameAndDescriptor("Helvetica", 20, fontDescriptor);
				label->SetFont(font);
				_loseWidget->GetContentView()->AddSubview(label);
				_loseWidget->SetCanBecomeKeyWidget(false);
				_loseWidget->SetWidgetLevel(-10000);
				 label->Release();
				
				_startWidget = new RN::UI::Widget(RN::UI::Widget::Style::Borderless, RN::Rect(0.0f, 0.0f, 512, 512));
				_startWidget->Center();
				label = new RN::UI::Label();
				label->SetNumberOfLines(5);
				label->SetText(RNSTR("You are a cleaning robot\ndo not let the\nwaste cross the river!\n\nPress space to start!"));
				label->SetFrame(_clockWidget->GetContentView()->GetBounds());
				label->SetAlignment(RN::UI::TextAlignment::Center);
				label->SetTextColor(RN::Color::Yellow());
				font = RN::UI::Font::WithNameAndDescriptor("Helvetica", 15, fontDescriptor);
				label->SetFont(font);
				_startWidget->GetContentView()->AddSubview(label);
				_startWidget->SetCanBecomeKeyWidget(false);
				_startWidget->SetWidgetLevel(-10000);
				 label->Release();
			}
			
			_startWidget->Open();
		});
	}

	void World::Update(float delta)
	{
		if(_isIntro)
		{
			RN::Vector3 position(100.0f, 20.0f, 0.0f);
			RN::Quaternion rotation = RN::Quaternion(RN::Vector3(_time * 20.0f, 0.0f, 0.0f));
			position = rotation.GetRotatedVector(position);
			_camera->SetWorldPosition(position);
			_camera->SetRotation(RN::Quaternion::WithLookAt(position));
		}
		
		RN::Input *input = RN::Input::GetSharedInstance();
		if(!_isRunning && input->IsKeyPressed(' ') && _dirtCounter == 0)
		{
			_isRunning = true;
			_isIntro = false;
			
			RN::ShadowParameter shadowParam = _sun->GetShadowParameters();
			shadowParam.distanceBlendFactor = 0.0001f;
			_sun->UpdateShadowParameters(shadowParam);
			
			RN::Kernel::GetSharedInstance()->ScheduleFunction([&](){
				_startWidget->Close();
				_clockWidget->Open();
			});
			
			_time = 0.0f;
			_camera->SetWorldPosition(_player->GetWorldPosition());
		}
		
		if(_camera->GetWorldPosition().y < 0.0f)
		{
			_camera->SetFogFar(50.0f);
			_waterCamera->SetFogFar(50.0f);
			_camera->SetFogNear(-20.0f);
			_waterCamera->SetFogNear(-20.0f);
			
			float factor = _dirtCounter / 4.0f;
			RN::Color fogcolor = RN::Color(0.25f, 0.25f, 0.3f) * (1.0f-factor) + RN::Color(0.25f, 0.0f, 0.0f) * factor;
			
			_camera->SetFogColor(fogcolor);
			_waterCamera->SetFogColor(fogcolor);
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
		
		_time += delta;
		
		if(_dirtCounter < 4 && _isRunning)
		{
			RN::Kernel::GetSharedInstance()->ScheduleFunction([&](){
				_clockLabel->SetText(RNSTR("Time: %4.2f", _time));
			});
		}
	}
	
	void World::MakeDirty()
	{
		_dirtCounter += 1;
		_dirtCounter = std::min(_dirtCounter, 4);
		
		if(_dirtCounter == 4)
		{
			_loseWidget->Open();
			_isRunning = false;
			
			RN::Timer::ScheduledTimerWithDuration(std::chrono::milliseconds(3000), [&]() {
				_loseWidget->Close();
				_dirtCounter = 0;
				RN::World::GetActiveWorld()->DropSceneNodes();
				RN::World::GetActiveWorld()->LoadOnThread(RN::Thread::GetCurrentThread(), nullptr);
				_startWidget->Open();
				_clockWidget->Close();
				_isIntro = true;
			}, false);
		}
	}
}
