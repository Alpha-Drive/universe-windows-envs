
#pragma once

#include "AbstractVehicleActor.h"

namespace Nvidia
{
	class VehicleActor : public AbstractVehicleActor
	{
	public:
		VehicleActor(const std::string & vehicleName, const Vector3 & pos, float heading,VehicleType type, const std::string & subVehicleName,int primaryColor, int secondaryColor) : AbstractVehicleActor(vehicleName,pos,heading,type,primaryColor,secondaryColor), m_subVehicleName(subVehicleName){}

		virtual void setupActor(DWORD currentTick) override;

		virtual void cleanUpActor() override;

		virtual bool isEntityOurActor(Entity e) const override;

	protected:

		Vehicle m_subVeh;

		std::string m_subVehicleName;
	};
}
