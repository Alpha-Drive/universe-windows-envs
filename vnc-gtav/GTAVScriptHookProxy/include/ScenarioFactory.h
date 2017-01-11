
#pragma once
#include <boost\property_tree\ptree.hpp>
#include "Scenario.h"

namespace Nvidia
{
	class ScenarioFactory
	{
	public:
		static Scenario createScenario(const boost::property_tree::basic_ptree<std::string, std::string> & item);

	private:

		static IActor* createPlayerActor(const boost::property_tree::basic_ptree<std::string, std::string> & item);

		static IActor* createPedestrianActor(const boost::property_tree::basic_ptree<std::string, std::string> & item);

		static IActor* createVehicleActor(const boost::property_tree::basic_ptree<std::string, std::string> & item);

		static IActor* createLightActor(const boost::property_tree::basic_ptree<std::string, std::string> & item);

		static std::string loadRewardFunction(const std::string & fileName);

		static AbstractVehicleActor::VehicleType findVehicleType(const std::string & type);

		ScenarioFactory() = delete;
		~ScenarioFactory() = delete;
		ScenarioFactory(const ScenarioFactory &) = delete;
		

	};
}