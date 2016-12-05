
#include "AbstractVehicleActor.h"
#include "VehicleList.h"
#include "natives.h"
#include <boost\log\trivial.hpp>


Vehicle Nvidia::AbstractVehicleActor::createRandomVehicle(VehicleType type, float x, float y, float z, float heading)
{
	switch (type)
	{
	case Nvidia::AbstractVehicleActor::Car:
		return createRandomVehicle(carList, _countof(carList), x, y, z, heading);
		break;
	case Nvidia::AbstractVehicleActor::Pickup:
		return createRandomVehicle(pickupList, _countof(pickupList), x, y, z, heading);
		break;
	case Nvidia::AbstractVehicleActor::QuadBike:
		return createRandomVehicle(quadBikeList, _countof(quadBikeList), x, y, z, heading);
		break;
	case Nvidia::AbstractVehicleActor::Motorcylce:
		return createRandomVehicle(motorcycleList, _countof(motorcycleList), x, y, z, heading);
		break;
	case Nvidia::AbstractVehicleActor::Van:
		return createRandomVehicle(vanList, _countof(vanList), x, y, z, heading);
		break;
	case Nvidia::AbstractVehicleActor::Truck:
		return createRandomVehicle(truckList, _countof(truckList), x, y, z, heading);
		break;
	case Nvidia::AbstractVehicleActor::Cyclist:
		return createRandomVehicle(cyclistList, _countof(cyclistList), x, y, z, heading);
		break;
	case Nvidia::AbstractVehicleActor::Bus:
		return createRandomVehicle(busList, _countof(busList), x, y, z, heading);
		break;
	case Nvidia::AbstractVehicleActor::Trailer:
		return createRandomVehicle(trailerList, _countof(trailerList), x, y, z, heading);
	}

	return 0;
}

Vehicle Nvidia::AbstractVehicleActor::createRandomVehicle(const Hash * vehicles, int count, float x, float y, float z, float heading)
{
	//get a random number in our range
	int random = rand() % count;

	Hash modelHash = vehicles[random];

	return createVehicle(modelHash, x, y, z, heading);

}

Vehicle Nvidia::AbstractVehicleActor::createVehicle(Hash modelHash, float x, float y, float z, float heading)
{
	if (STREAMING::IS_MODEL_IN_CDIMAGE(modelHash))
	{
		STREAMING::REQUEST_MODEL(modelHash);
		while (!STREAMING::HAS_MODEL_LOADED(modelHash)) WAIT(0);
	}

	Vehicle ret = VEHICLE::CREATE_VEHICLE(modelHash, x, y, z, heading, 1, 1);

	VEHICLE::SET_VEHICLE_ON_GROUND_PROPERLY(ret);

	if (m_primaryColor >= 0 || m_primaryColor < 159)
	{
		if (m_secondaryColor >= 0 || m_secondaryColor < 159)
		{
			VEHICLE::SET_VEHICLE_COLOURS(ret, m_primaryColor, m_secondaryColor);
		}
		else
		{
			int originalPrimary, originalSecondary;

			VEHICLE::GET_VEHICLE_COLOURS(ret, &originalPrimary, &originalSecondary);

			VEHICLE::SET_VEHICLE_COLOURS(ret, m_primaryColor, originalSecondary);
		}
	}

	return ret;
}