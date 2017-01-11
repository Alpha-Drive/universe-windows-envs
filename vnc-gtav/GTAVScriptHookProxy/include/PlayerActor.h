

#pragma once

#include "AbstractVehicleActor.h"

namespace Nvidia
{
	class PlayerActor : public AbstractVehicleActor
	{
	public:
		PlayerActor(const std::string & vehicleName, const Vector3 & pos, float heading, VehicleType type,bool isManualControl, int primaryColor, int secondaryColor) : AbstractVehicleActor(vehicleName,pos,heading,type,primaryColor,secondaryColor), m_isManualControl(isManualControl){}

		virtual void setupActor(DWORD currentTick) override;

		virtual void cleanUpActor() override;

		Vehicle playerVehicle() const { return m_veh; }

		virtual void onTick(DWORD currentTick) override;

		Vector3 startPosition() const { return m_startPosition; }

		Vector3 destination() const;

	protected:

		bool m_isManualControl;

		Vector3 m_startPosition;
	};
}