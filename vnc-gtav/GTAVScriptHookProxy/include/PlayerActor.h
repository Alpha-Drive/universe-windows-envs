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