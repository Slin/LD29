//
//  LDSpawner.h
//  LD29
//
//  Created by Nils Daumann on 27.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#ifndef __LD29__LDSpawner__
#define __LD29__LDSpawner__

#include <Rayne/Rayne.h>
#include "LDDirt.h"
#include "LDPlayer.h"

namespace LD
{
	class Spawner : public RN::SceneNode
	{
	public:
		Spawner();
		~Spawner();
		
		void Update(float delta) override;
		void SetPlayer(Player *player);
		
	private:
		float _time;
		RN::RandomNumberGenerator *_rng;
		Player *_player;
		
		RNDeclareMeta(Spawner)
	};
}

#endif /* defined(__LD29__LDSpawner__) */
