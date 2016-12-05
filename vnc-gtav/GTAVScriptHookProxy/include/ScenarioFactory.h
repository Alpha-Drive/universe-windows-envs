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