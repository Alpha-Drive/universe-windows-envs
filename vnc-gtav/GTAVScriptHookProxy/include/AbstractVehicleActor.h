
#pragma once

#include "AbstractActor.h"

namespace Nvidia
{
	class AbstractVehicleActor : public AbstractActor
	{
	public:

		enum VehicleType { Car, Pickup, QuadBike, Motorcylce, Van, Truck, Cyclist, Bus, Trailer };

		AbstractVehicleActor(const std::string & name, const Vector3 & pos, float heading, VehicleType type, int primaryColor, int secondaryColor) : AbstractActor(name, pos,heading), m_veh(0), m_type(type), m_primaryColor(primaryColor), m_secondaryColor(secondaryColor) {}

		virtual bool isEntityOurActor(Entity e) const override { return e == m_ped || e == m_veh; };

		virtual int id() override { return m_veh; }

	protected:
		
		Vehicle createRandomVehicle(VehicleType type, float x, float y, float z, float heading);

		Vehicle createVehicle(Hash hash, float x, float y, float z, float heading);

		Vehicle m_veh;

		VehicleType m_type;

		int m_primaryColor;

		int m_secondaryColor;

	private:

		Vehicle createRandomVehicle(const Hash * vehicles, int count, float x, float y, float z, float heading);

	};
}
