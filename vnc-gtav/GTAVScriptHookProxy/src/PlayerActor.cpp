
#include "PlayerActor.h"
#include "natives.h"
#include "Keyboard.h"

void Nvidia::PlayerActor::setupActor(DWORD currentTick)
{
	auto player = PLAYER::PLAYER_PED_ID();

	if (m_name.empty() == false)
	{
		Hash hash = GAMEPLAY::GET_HASH_KEY(const_cast<char*>(m_name.c_str()));
		m_veh = createVehicle(hash, m_position.x, m_position.y, m_position.z, m_heading);
	}
	else
	{
		m_veh = createRandomVehicle(m_type, m_position.x, m_position.y, m_position.z, m_heading);
	}
	
	PED::SET_PED_INTO_VEHICLE(player, m_veh, -1);

	m_ped = player;

	m_startPosition = { m_position.x,0,m_position.y,0,m_position.z,0 };

	if (m_tasks.empty() == false)
	{
		m_currentTask = m_tasks.begin();

		if (m_isManualControl == false)
			(*m_currentTask)->startTask(m_ped, currentTick);
	}
	else
	{
		m_currentTask = m_tasks.end();
	}
}

void Nvidia::PlayerActor::cleanUpActor()
{
	AI::TASK_LEAVE_VEHICLE(PLAYER::PLAYER_PED_ID(), m_veh, 16);

	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(ENTITY::GET_ENTITY_MODEL(m_veh));

	VEHICLE::DELETE_VEHICLE(&m_veh);
}

void Nvidia::PlayerActor::onTick(DWORD currentTick)
{
	auto savedTask = m_currentTask;

	if (m_currentTask != m_tasks.end())
	{
		if ((*m_currentTask)->isTaskComplete(m_ped, currentTick))
		{
			if(m_isManualControl == false)
				(*m_currentTask)->stopTask(m_ped);

			++m_currentTask;

			if (m_isManualControl == false && m_currentTask != m_tasks.end())
				(*m_currentTask)->startTask(m_ped, currentTick);
		}
	}

	if (savedTask != m_currentTask)
	{
		m_startPosition = ENTITY::GET_ENTITY_COORDS(m_veh, TRUE);
	}

	if (m_isManualControl == false)
	{
		bool isNowSendingInput = isKeyDown(VkKeyScan('a')) || isKeyDown(VkKeyScan('d')) || isKeyDown((VkKeyScan('w'))) || isKeyDown(VkKeyScan('s')) || isKeyDown(VK_SPACE);

		if (isNowSendingInput)
		{
			AI::CLEAR_PED_TASKS_IMMEDIATELY(PLAYER::PLAYER_PED_ID());

			PED::SET_PED_INTO_VEHICLE(m_ped, m_veh, -1);
		}
	}
}

Vector3 Nvidia::PlayerActor::destination() const
{
	if (m_currentTask != m_tasks.end())
	{
		return (*m_currentTask)->destination();
	}
	else
	{
		Vector3 ret = { 0.0f,0,0.0f,0,0.0f,0 };
		return ret;
	}
}
