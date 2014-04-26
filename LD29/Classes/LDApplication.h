//
//  LDApplication.h
//  LD29
//
//  Created by Nils Daumann on 26.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#ifndef __LD_APPLICATION_H__
#define __LD_APPLICATION_H__

#include <Rayne/Rayne.h>
#include "LDWorld.h"

namespace LD
{
	class Application : public RN::Application
	{
	public:
		Application();
		~Application() override;
		
		void Start() override;
		void WillExit() override;
	};
}

#endif /* __LD_APPLICATION_H__ */
