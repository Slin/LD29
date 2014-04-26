//
//  LDWorld.h
//  LD29
//
//  Created by Nils Daumann on 26.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#ifndef __LD_WORLD_H__
#define __LD_WORLD_H__

#include <Rayne/Rayne.h>
#include "RBPhysicsWorld.h"

namespace LD
{
	class World : public RN::World
	{
	public:
		World();
		~World() override;
		
		void LoadOnThread(RN::Thread *thread, RN::Deserializer *deserializer) override;
		void Update(float delta) override;
		
	private:
		void HandleInputEvent(RN::Event *event);
		
		RN::bullet::PhysicsWorld *_physicsWorld;
		RN::Camera *_camera;
	};
}

#endif /* __LD_WORLD_H__ */
