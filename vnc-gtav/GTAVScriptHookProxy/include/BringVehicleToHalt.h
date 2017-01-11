
#pragma once

#include "ITask.h"
#include "natives.h"

namespace Nvidia
{
	class BringVehicleToHalt : public ITask
	{
	public:
		BringVehicleToHalt(float distance) : m_distance(distance){}

		~BringVehicleToHalt() {};

		virtual void startTask(Ped ped, DWORD currentTick) override
		{
			Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(ped, FALSE);
			VEHICLE::_TASK_BRING_VEHICLE_TO_HALT(veh, m_distance, 0, TRUE);
		}

		virtual bool isTaskComplete(Ped ped, DWORD currentTick) override
		{
			Vehicle veh = PED::GET_VEHICLE_PED_IS_IN(ped, FALSE);
			auto velocity = ENTITY::GET_ENTITY_VELOCITY(veh);
			return velocity.x < 0.1f && velocity.y < 0.1f && velocity.z < 0.1f;

		}

		virtual void stopTask(Ped ped) override
		{
			AI::CLEAR_PED_TASKS(ped);
		}

		virtual Vector3 destination() const override
		{
			Vector3 ret = { 0.0f,0,0.0f,0,0.0f,0 };
			return ret;
		}

	private:
		float m_distance;
	};
}
