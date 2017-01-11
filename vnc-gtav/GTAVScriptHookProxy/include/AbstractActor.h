
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