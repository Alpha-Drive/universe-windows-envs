
#include "TaskFactory.h"
#include "natives.h"
#include "ITask.h"
#include "WalkToCoordinate.h"
#include "DriveToCoordinate.h"
#include "BringVehicleToHalt.h"


std::unique_ptr<Nvidia::ITask> Nvidia::TaskFactory::createTask(const boost::property_tree::basic_ptree<std::string, std::string> & item)
{
	std::string name = item.get("name","");

	if (name == "walktocoordinate")
	{
		auto x = item.get("x", 0.0f);
		auto y = item.get("y", 0.0f);
		auto z = item.get("z", 0.0f);
		auto speed = item.get("speed", 2.0f);
		auto heading = item.get("heading", 0.0f);
		auto stoppingDistance = item.get("stoppingDistance", 0.0f);
		return std::unique_ptr<ITask>(new WalkToCoordinate(x, y, z, speed, heading, stoppingDistance));
	}
	else if(name == "drivetocoordinate")
	{
		auto x = item.get("x", 0.0f);
		auto y = item.get("y", 0.0f);
		auto z = item.get("z", 0.0f);
		auto speed = item.get("speed", 2.0f);
		auto drivingMode = findDrivingStyle(item.get<std::string>("drivingMode", ""));
		auto stoppingDistance = item.get("stoppingDistance", 0.0f);
		auto completeDistance = item.get("completeDistance", 1.0f);
		auto startingSpeed = item.get("startingSpeed", 0.0f);
		return std::unique_ptr<ITask>(new DriveToCoordinate(x, y, z, speed, drivingMode, stoppingDistance, completeDistance, startingSpeed));
	}
	else if (name == "bringvehicletohalt")
	{
		auto distance = item.get("stoppingDistance", 1.0f);
		return std::unique_ptr<ITask>(new BringVehicleToHalt(distance));
	}
	else
	{
		throw std::exception("Unknown task sent into create task");
	}
}

int Nvidia::TaskFactory::findDrivingStyle(const std::string & drivingMode)
{
	int ret = (int)DrivingStyleNormal;

	if (drivingMode == "ignorelights")
		ret = (int)DrivingStyleIgnoreLights;
	else if (drivingMode == "sometimesovertaketraffic")
		ret = (int)DrivingStyleSometimesOvertakeTraffic;
	else if (drivingMode == "rushed")
		ret = (int)DrivingStyleRushed;
	else if (drivingMode == "superrushed")
		ret = (int)(DrivingStyleRushed & (~(unsigned int)1));
	else if (drivingMode == "avoidtraffic")
		ret = (int)DrivingStyleAvoidTraffic;
	else if (drivingMode == "avoidtrafficextremely")
		ret = (int)DrivingStyleAvoidTrafficExtremely;

	return ret;
}
