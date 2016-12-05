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
