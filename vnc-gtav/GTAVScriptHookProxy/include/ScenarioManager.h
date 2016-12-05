/*
* Copyright (c) 2016 NVIDIA CORPORATION.  All Rights Reserved.
*
* NVIDIA CORPORATION and its licensors retain all intellectual property
* and proprietary rights in and to this software, related documentation
* and any modifications thereto.  Any use, reproduction, disclosure or
* distribution of this software and related documentation without an express
* license agreement from NVIDIA CORPORATION is strictly prohibited.
*
*/

#pragma once

#include <string>
#include "Scenario.h"

namespace Nvidia
{

	class ScenarioManager
	{
	public:
		using ScenarioCollection = std::vector<Scenario>;
		using ScenarioIterator = ScenarioCollection::iterator;

		ScenarioManager();

		Scenario::Status onGameLoop();

		void load(const std::string & scenarioPath);

		void run(const std::string & scenarioName);

		void stop();

		void reset(DWORD waitTime = 1000);

		const Scenario & currentScenario() const;

	private:

		void removeOtherVehiclesAndPedestrians();

		bool m_isCurrentlyRunning;

		ScenarioCollection m_scenarios;

		ScenarioIterator m_currentScenario;

		ScenarioManager(const ScenarioManager&) = delete;
		ScenarioManager & operator=(const ScenarioManager &) = delete;
		
	};
}