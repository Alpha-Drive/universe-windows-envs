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
