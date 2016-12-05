
#include "ScenarioFactory.h"
#include "TaskFactory.h"
#include "PedestrianActor.h"
#include "PlayerActor.h"
#include "VehicleActor.h"
#include <fstream>
#include "AbstractVehicleActor.h"
#include <boost\log\trivial.hpp>
#include "LightActor.h"

using namespace Nvidia;

Scenario Nvidia::ScenarioFactory::createScenario(const boost::property_tree::basic_ptree<std::string, std::string> & item)
{
	Scenario ret; 

	ret.m_name = item.get("name", "Unnamed");

	ret.m_lengthInMilliseconds = item.get("length", 20000);

	ret.m_rewardFunction = loadRewardFunction(item.get("rewardFunction", ""));

	auto && cameraPosition = item.get_child("cameraPosition");

	ret.m_cameraPosition.x = cameraPosition.get("x", 0.0f);
	ret.m_cameraPosition.y = cameraPosition.get("y", 0.0f);
	ret.m_cameraPosition.z = cameraPosition.get("z", 0.0f);

	ret.m_removeOtherEntities = item.get("removeOtherEntities", true);

	ret.m_timeScale = item.get("timeScale", 1.0f);

	ret.m_hour = item.get("hour", 9);
	ret.m_minute = item.get("minute", 0);
	ret.m_second = item.get("second", 0);

	ret.m_weather = item.get("weather", "");

	for (auto & actor : item.get_child("actors"))
	{
		auto & data = actor.second;
		auto type = data.get("type", "");

		IActor * createdActor = nullptr;

		if (type == "player")
		{
			createdActor = createPlayerActor(data);

			ret.m_playerActor = (PlayerActor *)createdActor;

			ret.m_actors.push_back(createdActor);
		}
		else if (type == "pedestrian")
		{
			createdActor = createPedestrianActor(data);

			ret.m_actors.push_back(createdActor);
		}
		else if (type == "vehicle")
		{
			createdActor = createVehicleActor(data);

			ret.m_actors.push_back(createdActor);
		}
		else if (type == "light")
		{
			createdActor = createLightActor(data);

			ret.m_actors.push_back(createdActor);
		}
		else
			throw std::exception("Unknown actor type sent into method");

		if (createdActor != nullptr)
		{
			for (auto & task : data.get_child("tasks"))
			{
				createdActor->addTask(TaskFactory::createTask(task.second));
			}
		}
		
			
	}

	return ret;
}

IActor* ScenarioFactory::createLightActor(const boost::property_tree::basic_ptree<std::string, std::string> & item)
{
	Vector3 pos;

	pos.x = item.get("x", 0.0f);
	pos.y = item.get("y", 0.0f);
	pos.z = item.get("z", 0.0f);

	int red = item.get("red", 255);
	int blue = item.get("blue", 255);
	int green = item.get("green", 255);

	float range = item.get("range", 1.0f);
	float intensity = item.get("intensity", 1.0f);

	return new LightActor(pos,red,blue,green,range,intensity);
}

IActor* ScenarioFactory::createPlayerActor(const boost::property_tree::basic_ptree<std::string, std::string> & item)
{
	Vector3 pos;

	pos.x = item.get("x", 0.0f);
	pos.y = item.get("y", 0.0f);
	pos.z = item.get("z", 0.0f);

	float heading = item.get("heading", 0.0f);

	std::string vehicleName = item.get<std::string>("vehicleName", "");

	bool isManualControlled = item.get("manualControlled", false);

	std::string type = item.get<std::string>("vehicleType", "");

	auto vehicleType = findVehicleType(type);

	auto primaryColor = item.get<int>("primaryColor", -1);

	auto secondaryColor = item.get<int>("secondaryColor", -1);

	return new PlayerActor(vehicleName, pos, heading, vehicleType, isManualControlled, primaryColor, secondaryColor);

}


IActor* ScenarioFactory::createVehicleActor(const boost::property_tree::basic_ptree<std::string, std::string> & item)
{
	Vector3 pos;

	pos.x = item.get("x", 0.0f);
	pos.y = item.get("y", 0.0f);
	pos.z = item.get("z", 0.0f);

	float heading = item.get("heading", 0.0f);

	std::string vehicleName = item.get<std::string>("vehicleName", "");

	std::string type = item.get<std::string>("vehicleType", "");

	auto vehicleType = findVehicleType(type);

	std::string subVehicleName = item.get<std::string>("subVehicleName", "");

	auto primaryColor = item.get<int>("primaryColor", -1);

	auto secondaryColor = item.get<int>("secondaryColor", -1);

	return new VehicleActor(vehicleName, pos, heading, vehicleType, subVehicleName, primaryColor, secondaryColor);
}

IActor* ScenarioFactory::createPedestrianActor(const boost::property_tree::basic_ptree<std::string, std::string> & item)
{
	Vector3 pos;

	pos.x = item.get("x", 0.0f);
	pos.y = item.get("y", 0.0f);
	pos.z = item.get("z", 0.0f);

	float heading = item.get("heading", 0.0f);

	std::string pedName = item.get<std::string>("pedestrianName", "");

	return new PedestrianActor(pedName,pos,heading);
}



std::string ScenarioFactory::loadRewardFunction(const std::string & fileName)
{
	std::ifstream infile(fileName.c_str(), std::ios::in | std::ios::ate | std::ios::binary);

	if (infile.is_open())
	{
		const auto size = infile.tellg();
		infile.seekg(0, std::ios::beg);

		assert(size >= 0);

		if (size == std::streampos(0))
		{
			return std::string();
		}
		else 
		{
			std::vector<char> v(static_cast<size_t>(size));
			infile.read(&v[0], size);
			return std::string(v.begin(), v.end());
		}
	}

	return std::string();
}

AbstractVehicleActor::VehicleType ScenarioFactory::findVehicleType(const std::string & type)
{
	auto ret = AbstractVehicleActor::Car;


	if (type == "Pickup")
		ret = AbstractVehicleActor::Pickup;
	else if (type == "QuadBike")
		ret = AbstractVehicleActor::QuadBike;
	else if (type == "Motorcycle")
		ret = AbstractVehicleActor::Motorcylce;
	else if (type == "Van")
		ret = AbstractVehicleActor::Van;
	else if (type == "Truck")
		ret = AbstractVehicleActor::Truck;
	else if (type == "Cyclist")
		ret = AbstractVehicleActor::Cyclist;
	else if (type == "Bus")
		ret = AbstractVehicleActor::Bus;
	else if (type == "Trailer")
		ret = AbstractVehicleActor::Trailer;
	else
	{
		if (!type.empty() && type != "Car")
			BOOST_LOG_TRIVIAL(error) << "Unknown vehicle type " << type << " sent into method.";
	}

	return ret;
}
