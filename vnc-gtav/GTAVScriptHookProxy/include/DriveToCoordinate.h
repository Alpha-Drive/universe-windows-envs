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

#include "ITask.h"
#include "natives.h"
#include "types.h"
#include "enums.h"

namespace Nvidia
{
	class DriveToCoordinate : public Nvidia::ITask
	{
	public:
		DriveToCoordinate(float x, float y, float z, float speed, int drivingMode, float stoppingRange, float completeDistance, float startingSpeed) :
			m_x(x), m_y(y), m_z(z), m_speed(speed), m_stopDistance(stoppingRange), m_completeDistance(completeDistance),m_drivingMode(drivingMode), m_startingSpeed(startingSpeed) {}

		~DriveToCoordinate() {};

		virtual void startTask(Ped ped, DWORD currentTick) override
		{
			Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(ped, FALSE);

			if (m_startingSpeed > 0.0f)
				VEHICLE::SET_VEHICLE_FORWARD_SPEED(veh, m_startingSpeed);

			AI::TASK_VEHICLE_DRIVE_TO_COORD(ped, veh, m_x, m_y, m_z, m_speed, 1, ENTITY::GET_ENTITY_MODEL(veh), m_drivingMode, m_stopDistance, -1);
		}

		virtual bool isTaskComplete(Ped ped, DWORD currentTick) override
		{
			return ENTITY::IS_ENTITY_AT_COORD(ped, m_x, m_y, m_z, m_completeDistance, m_completeDistance, m_completeDistance, 0, 1, 0) == TRUE;
		}

		virtual void stopTask(Ped ped) override
		{
			Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(ped, FALSE);

			AI::CLEAR_PED_TASKS_IMMEDIATELY(ped);

			PED::SET_PED_INTO_VEHICLE(ped, veh, -1);
		}

		virtual Vector3 destination() const override
		{
			Vector3 ret = { m_x,0,m_y,0,m_z,0 };
			return ret;
		}

	private:
		float m_x;
		float m_y;
		float m_z;
		float m_speed;
		float m_stopDistance;
		float m_completeDistance;
		int m_drivingMode;
		float m_startingSpeed;


	};
}

