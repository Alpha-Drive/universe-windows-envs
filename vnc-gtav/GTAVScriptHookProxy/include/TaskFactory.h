
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