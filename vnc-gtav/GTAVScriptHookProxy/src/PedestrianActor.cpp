
#include "PedestrianActor.h"
#include "natives.h"
#include "PedList.h"

void Nvidia::PedestrianActor::setupActor(DWORD currentTick)
{
	if (m_name.empty() == false)
	{
		Hash hash = GAMEPLAY::GET_HASH_KEY(const_cast<char*>(m_name.c_str()));
		m_ped = createPedestrian(hash, m_position.x, m_position.y, m_position.z, m_heading);
	}
	else
	{
		m_ped = createRandomPedestrian(m_position.x, m_position.y, m_position.z, m_heading);
	}

	AbstractActor::setupActor(currentTick);
}

void Nvidia::PedestrianActor::cleanUpActor()
{
	STREAMING::SET_MODEL_AS_NO_LONGER_NEEDED(ENTITY::GET_ENTITY_MODEL(m_ped));

	PED::DELETE_PED(&m_ped);
}

Ped Nvidia::PedestrianActor::createPedestrian(Hash modelHash, float x, float y, float z, float heading)
{
	if (STREAMING::IS_MODEL_IN_CDIMAGE(modelHash))
	{
		STREAMING::REQUEST_MODEL(modelHash);
		while (!STREAMING::HAS_MODEL_LOADED(modelHash)) WAIT(0);
	}

	return PED::CREATE_PED(1, modelHash, x, y, z, heading, TRUE, TRUE);
}

Ped Nvidia::PedestrianActor::createRandomPedestrian(float x, float y, float z, float heading)
{
	int random = rand() % _countof(pedestrianList);

	Hash modelHash = pedestrianList[random];

	return createPedestrian(modelHash, x, y, z, heading);
}
