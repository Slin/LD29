//
//  LDPlayer.h
//  LD29
//
//  Created by Nils Daumann on 26.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#ifndef __LD29__LDPlayer__
#define __LD29__LDPlayer__

#include <Rayne/Rayne.h>
#include "RBKinematicController.h"
#include "RBPhysicsWorld.h"

namespace LD
{
	class Player : public RN::Entity
	{
	public:
		Player(RN::Camera *camera);
		~Player();
		
		void Update(float delta) override;
		
	private:
		RN::Skeleton *_skeleton;
		RN::bullet::KinematicController *_controller;
		RN::Camera *_camera;
		
		RN::GenericParticleEmitter *_forwardEmitter;
		RN::GenericParticleEmitter *_leftEmitter;
		RN::GenericParticleEmitter *_rightEmitter;
		RN::GenericParticleEmitter *_upEmitter;
		RN::GenericParticleEmitter *_downEmitter;
		
		RN::Vector3 _speed;
		RN::Vector3 _rotationSpeed;
		float _turboCooldown;
		
		float _smoothDelta;
		
		RNDeclareMeta(Player);
	};
}

#endif /* defined(__LD29__LDPlayer__) */
