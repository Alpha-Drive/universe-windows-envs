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