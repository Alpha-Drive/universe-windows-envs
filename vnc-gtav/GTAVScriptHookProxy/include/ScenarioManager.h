
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