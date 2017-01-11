
#pragma once

#include <memory>
#include "ITask.h"

namespace Nvidia
{
	class ITask;

	class IActor
	{
	public:
		IActor() {};

		virtual ~IActor() {};

		virtual void addTask(std::unique_ptr<ITask> task) = 0;

		virtual void setupActor(DWORD currentTick) = 0;

		virtual void onTick(DWORD currentTick) = 0;

		virtual void cleanUpActor() = 0;

		virtual bool isEntityOurActor(Entity e) const = 0;

		virtual int id() = 0;

	protected:
		
	};
}