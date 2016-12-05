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

#include <vector>
#include <memory>
#include "IActor.h"
#include "PlayerActor.h"
#include "types.h"

namespace Nvidia
{
	class Scenario
	{
	public:
		using Actors = std::vector<IActor*>;
		using ActorIds = std::vector<int>;

		enum Status {NotStarted,Running,Completed};

		friend class ScenarioFactory;

		Scenario();

		~Scenario();

		void setupScenario(DWORD currentTick);

		void onTick(DWORD currentTick);

		void cleanUpScenario();

		bool isEntityInOurScenario(Entity e) const;

		Status status() const {return m_status;}

		const std::string & name() const { return m_name; }

		bool removeOtherEntities() const { return m_removeOtherEntities; }

		const std::string & rewardFunction() const { return m_rewardFunction; }

		const PlayerActor * playerActor() const { return m_playerActor; }

		const std::vector<int> & actorIDs() const { return m_actorIds; }
	private:

		std::string m_name;

		std::string m_rewardFunction;

		unsigned int m_lengthInMilliseconds;

		PlayerActor * m_playerActor;

		Actors m_actors;

		ActorIds m_actorIds;

		DWORD m_startTick;

		Status m_status;
		
		Cam m_camera;

		Vector3 m_cameraPosition;

		bool m_removeOtherEntities;

		float m_timeScale;

		int m_hour;

		int m_minute;
		
		int m_second;

		std::string m_weather;
	};
}