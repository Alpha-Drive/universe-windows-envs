
#pragma once

#include "ITask.h"
#include "natives.h"
#include "types.h"

namespace Nvidia
{
	class WalkToCoordinate : public ITask
	{
	public:
		WalkToCoordinate(float x, float y, float z, float speed, float heading, float stoppingDistance) : m_x(x), m_y(y), m_z(z), m_speed(speed), m_heading(heading), m_stopDistance(stoppingDistance) {}

		virtual ~WalkToCoordinate() {};

		virtual void startTask(Ped ped, DWORD currentTick) override
		{
			AI::TASK_GO_STRAIGHT_TO_COORD(ped, m_x, m_y, m_z, m_speed, -1, m_heading, 0.0f);
		}

		virtual bool isTaskComplete(Ped ped, DWORD currentTick) override
		{
			return ENTITY::IS_ENTITY_AT_COORD(ped, m_x, m_y, m_z, m_stopDistance, m_stopDistance, m_stopDistance, 0, 1, 0) == TRUE;
		}

		virtual void stopTask(Ped ped) override
		{
			AI::CLEAR_PED_TASKS_IMMEDIATELY(ped);
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
		float m_heading;
		float m_stopDistance;
	};
}
