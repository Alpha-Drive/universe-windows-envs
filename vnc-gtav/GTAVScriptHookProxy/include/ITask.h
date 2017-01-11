
#pragma once
#include "types.h"

namespace Nvidia
{
	class ITask
	{
	public:
		virtual ~ITask() {};

		virtual void startTask(Ped ped,DWORD currentTick) = 0;

		virtual bool isTaskComplete(Ped ped, DWORD currentTick) = 0;

		virtual void stopTask(Ped ped) = 0;

		virtual Vector3 destination() const = 0;

	protected:
		ITask() {};

	};
}