
#include "VehicleActor.h"
#include "natives.h"

void Nvidia::VehicleActor::setupActor(DWORD currentTick)
{
	if (m_name.empty() == false)
	{
		Hash hash = GAMEPLAY::GET_HASH_KEY(const_cast<char*>(m_name.c_str()));
		m_veh = createVehicle(hash, m_position.x, m_position.y, m_position.z, m_heading);
	}
	else
	{
		m_veh = createRandomVehicle(m_type, m_position.x, m_position.y, m_position.z, m_heading);
	}
	
	if(m_tasks.empty() == false)
		m_ped = PED::CREATE_RANDOM_PED_AS_DRIVER(m_veh, TRUE);

	if (m_subVehicleName.empty() == false)
	{
		Hash subHash = GAMEPLAY::GET_HASH_KEY(const_cast<char*>(m_subVehicleName.c_str()));
		m_subVeh = createVehicle(subHash, m_position.x, m_position.y, m_position.z, m_heading);

		VEHICLE::ATTACH_VEHICLE_TO_TRAILER(m_veh, m_subVeh, 100.0f);
	}

	AbstractActor::setupActor(currentTick);
}

void Nvidia::VehicleActor::cleanUpActor()
{
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(ENTITY::GET_ENTITY_MODEL(m_veh));

	VEHICLE::DELETE_VEHICLE(&m_veh);

	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(ENTITY::GET_ENTITY_MODEL(m_ped));

	PED::DELETE_PED(&m_ped);

	if (m_subVeh != 0)
	{
		STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(ENTITY::GET_ENTITY_MODEL(m_subVeh));

		VEHICLE::DELETE_VEHICLE(&m_subVeh);
	}
}


bool Nvidia::VehicleActor::isEntityOurActor(Entity e) const
{
	return e == m_subVeh || __super::isEntityOurActor(e);
}
