
#include "Scenario.h"
#include "natives.h"

Nvidia::Scenario::Scenario() : m_lengthInMilliseconds(0), m_playerActor(nullptr), m_startTick(0), m_status(NotStarted), m_camera(0)
{

}

Nvidia::Scenario::~Scenario()
{
	for each (auto actor in m_actors)
	{
		actor->cleanUpActor();
		delete actor;
	}

	m_actors.clear();
}

void Nvidia::Scenario::setupScenario(DWORD currentTick)
{
	m_startTick = currentTick;

	m_actorIds.resize(m_actors.size());

	for each (auto & actor in m_actors)
	{
		actor->setupActor(currentTick);
		m_actorIds.push_back(actor->id());
	}

	GAMEPLAY::SET_TIME_SCALE(m_timeScale);

	TIME::ADVANCE_CLOCK_TIME_TO(m_hour, m_minute, m_second);

	GAMEPLAY::SET_WEATHER_TYPE_NOW(const_cast<char*>(m_weather.c_str()));

	if (m_playerActor != nullptr)
	{
		auto rotation = ENTITY::GET_ENTITY_ROTATION(m_playerActor->playerVehicle(), FALSE);

		auto position = ENTITY::GET_ENTITY_COORDS(m_playerActor->playerVehicle(), FALSE);

		m_camera = CAM::CREATE_CAM_WITH_PARAMS("DEFAULT_SCRIPTED_FLY_CAMERA", position.x, position.y, position.z,
			0.0, 0.0, 0.0, 50.0, 0, 2);

		CAM::ATTACH_CAM_TO_ENTITY(m_camera, m_playerActor->playerVehicle(), m_cameraPosition.x, m_cameraPosition.y, m_cameraPosition.z, TRUE);

		CAM::SET_CAM_ROT(m_camera, rotation.x, rotation.y, rotation.z, 2);

		CAM::SET_CAM_ACTIVE(m_camera, TRUE);

		CAM::RENDER_SCRIPT_CAMS(TRUE, FALSE, m_camera, TRUE, FALSE);
	}
	

	m_status = Running;
}

void Nvidia::Scenario::onTick(DWORD currentTick)
{
	for each (auto & actor in m_actors)
	{
		actor->onTick(currentTick);
	}

	auto rotation = ENTITY::GET_ENTITY_ROTATION(m_playerActor->playerVehicle(), FALSE);

	CAM::SET_CAM_ROT(m_camera, rotation.x, rotation.y, rotation.z, 2);

	if (m_status == Running && ((currentTick - m_startTick) > m_lengthInMilliseconds))
	{
		m_status = Completed;
	}
}

void Nvidia::Scenario::cleanUpScenario()
{
	for each (auto actor in m_actors)
	{
		actor->cleanUpActor();
		delete actor;
	}

	m_actors.clear();

	m_actorIds.clear();

	GAMEPLAY::SET_TIME_SCALE(1.0f);

	CAM::SET_CAM_ACTIVE(m_camera, FALSE);

	CAM::RENDER_SCRIPT_CAMS(FALSE, FALSE, m_camera, TRUE, FALSE);

	CAM::DESTROY_CAM(m_camera, TRUE);

	m_status = NotStarted;
	
}

bool Nvidia::Scenario::isEntityInOurScenario(Entity e) const
{
	bool ret = false;

	for each (auto & actor  in m_actors)
	{
		if (actor->isEntityOurActor(e))
		{
			ret = true;
			break;
		}
	}

	return ret;
}


