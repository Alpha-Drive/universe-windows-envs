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