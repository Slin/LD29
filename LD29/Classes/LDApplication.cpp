//
//  LDApplication.cpp
//  LD29
//
//  Created by Nils Daumann on 26.04.14.
//  Copyright (c) 2014 Überpixel. All rights reserved.
//

#include "LDApplication.h"

namespace LD
{
	Application::Application()
	{}
	Application::~Application()
	{}
	
	
	void Application::Start()
	{
		SetTitle("LD29");

		RN::World *world = new World();
		RN::WorldCoordinator::GetSharedInstance()->LoadWorld(world->Autorelease());
	}
	
	void Application::WillExit()
	{}
}
