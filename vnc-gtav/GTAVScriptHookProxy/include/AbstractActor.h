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
#include "types.h"


namespace Nvidia
{
	class AbstractActor : public IActor
	{
	public:
		using Tasks = std::vector<std::unique_ptr<ITask>>;

		virtual void setupActor(DWORD currentTick) override;

		virtual void onTick(DWORD currentTick) override;

		virtual void addTask(std::unique_ptr<ITask> task) override
		{
			m_tasks.push_back(std::move(task));
		}

	protected:

		AbstractActor(const std::string & name, const Vector3 & position, float heading) : m_position(position),m_heading(heading), m_name(name), m_ped(0) {}

		Vector3 m_position;

		float m_heading;

		std::string m_name;

		Ped m_ped;

		Tasks m_tasks;

		Tasks::iterator m_currentTask;
		
	};
	
}