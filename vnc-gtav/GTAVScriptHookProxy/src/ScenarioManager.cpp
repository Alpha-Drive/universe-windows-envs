
#include <boost\property_tree\ptree_fwd.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "ScenarioManager.h"
#include "ScenarioFactory.h"
#include <boost\log\trivial.hpp>
#include "natives.h"


using namespace Nvidia;

ScenarioManager::ScenarioManager() : m_isCurrentlyRunning(false)
{

}

void ScenarioManager::load(const std::string & scenarioPath)
{
	using boost::property_tree::ptree;

	ptree pt;

	try
	{
		read_json(scenarioPath, pt);

		for (auto & item : pt.get_child("scenarios"))
		{
			m_scenarios.push_back(ScenarioFactory::createScenario(item.second));
		}
	}
	catch (const std::exception& ex)
	{
		BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " Exception loading scenario json: " << ex.what();
		m_currentScenario = m_scenarios.end();
	}
	catch (...)
	{
		BOOST_LOG_TRIVIAL(error) << __FUNCTION__ <<  "Unknown exception loading scenario json.";
		m_currentScenario = m_scenarios.end();
	}
}

Scenario::Status Nvidia::ScenarioManager::onGameLoop()
{
	if (m_isCurrentlyRunning)
	{
		auto tick = GetTickCount();

		auto status = m_currentScenario->status();

		switch (status)
		{
		case Scenario::NotStarted:
			m_currentScenario->setupScenario(tick);
			if(m_currentScenario->removeOtherEntities())
				removeOtherVehiclesAndPedestrians();
			break;
		case Scenario::Running:
			m_currentScenario->onTick(tick);
			if (m_currentScenario->removeOtherEntities())
				removeOtherVehiclesAndPedestrians();
			break;
		}

		return status;
	}

	return Scenario::NotStarted;
}

void Nvidia::ScenarioManager::run(const std::string & scenarioName)
{
	m_currentScenario = std::find_if(m_scenarios.begin(), m_scenarios.end(), [&](Scenario & scenario)
	{
		return scenario.name() == scenarioName;
	});

	if (m_currentScenario == m_scenarios.end())
	{
		BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " Unable to find a scenario name: " << scenarioName;
	}
	else
	{
		m_isCurrentlyRunning = true;
	}
	
}

void Nvidia::ScenarioManager::stop()
{
	reset(3000);

	m_currentScenario = m_scenarios.end();

	m_isCurrentlyRunning = false;
}

void Nvidia::ScenarioManager::reset(DWORD waitTime)
{
	if (m_currentScenario != m_scenarios.end() && m_currentScenario->status() != Scenario::NotStarted)
	{
		GAMEPLAY::SET_GAME_PAUSED(TRUE);

		WAIT(waitTime);

		m_currentScenario->cleanUpScenario();

		GAMEPLAY::SET_GAME_PAUSED(FALSE);
	}
	
}

const Scenario & Nvidia::ScenarioManager::currentScenario() const
{
	if (m_currentScenario != m_scenarios.end())
		return *m_currentScenario;

	BOOST_LOG_TRIVIAL(error) << __FUNCTION__ << " Fatal error.  Somebody called current scenario prior to calling run";

	throw std::runtime_error("Fatal error.  Somebody called current scenario prior to calling run");
}

void Nvidia::ScenarioManager::removeOtherVehiclesAndPedestrians()
{
	
	if (m_currentScenario == m_scenarios.end())
		return;

	auto currentScenario = m_currentScenario;

	auto removeEntities = [currentScenario](int(*fillFunction)(int * arr, int arrSize), void(*removeFunction)(int id))
	{
		const int NUMBER_OBJECTS = 1024;
		std::vector<int> entities(NUMBER_OBJECTS);

		fillFunction(&entities[0], NUMBER_OBJECTS);

		for each (int i in entities)
		{
			if (ENTITY::DOES_ENTITY_EXIST(i) && currentScenario->isEntityInOurScenario((Entity)i) == false)
			{
				removeFunction(i);
			}

		}
	};

	removeEntities(worldGetAllVehicles, [](int id) 
	{
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(ENTITY::GET_ENTITY_MODEL(id));

		Vehicle veh = (Vehicle)id;

		ENTITY::SET_ENTITY_AS_MISSION_ENTITY(veh, true, false);

		VEHICLE::DELETE_VEHICLE(&veh);
	});
	
	removeEntities(worldGetAllPeds, [](int id) 
	{
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(ENTITY::GET_ENTITY_MODEL(id));

		Ped ped = (Ped)id;

		ENTITY::SET_ENTITY_AS_MISSION_ENTITY(ped, true, false);

		PED::DELETE_PED(&ped);
	});
}


