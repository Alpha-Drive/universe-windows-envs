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

#include <boost\property_tree\ptree.hpp>
#include <memory>
#include "ITask.h"
#include "enums.h"

namespace Nvidia
{
	class TaskFactory
	{
	public:
		static std::unique_ptr<ITask> createTask(const boost::property_tree::basic_ptree<std::string, std::string> & item);

		

	private:

		static int findDrivingStyle(const std::string & drivingMode);

		TaskFactory() = delete;
		~TaskFactory() = delete;
		TaskFactory(const TaskFactory &) = delete;
		
	};
}