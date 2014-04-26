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
#include "RBRigidBody.h"

namespace LD
{
	class Player : public RN::Entity
	{
	public:
		Player();
		~Player();
		
		void Update(float delta) override;
		
	private:
		RN::Skeleton *_skeleton;
		RN::bullet::RigidBody *_body;
	};
}

#endif /* defined(__LD29__LDPlayer__) */
