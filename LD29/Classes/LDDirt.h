//
//  LDDirt.h
//  LD29
//
//  Created by Nils Daumann on 27.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#ifndef __LD29__LDDirt__
#define __LD29__LDDirt__

#include <Rayne/Rayne.h>
#include "RBRigidBody.h"
#include "LDPlayer.h"

namespace LD
{
	class Dirt : public RN::Entity
	{
	public:
		Dirt();
		~Dirt();
		
		void SetPlayer(Player *player);
		void Update(float delta) override;
		void Throw(RN::Vector3 impulse);
		
	private:
		RN::bullet::RigidBody *_body;
		Player *_player;
		
		bool _didCount;
		bool _isIntro;
		
		RNDeclareMeta(Dirt)
	};
}

#endif /* defined(__LD29__LDDirt__) */
