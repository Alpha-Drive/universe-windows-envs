
#include "AbstractActor.h"
#include "natives.h"
#include "types.h"

void Nvidia::AbstractActor::setupActor(DWORD currentTick)
{
	if (m_tasks.empty() == false)
	{
		m_currentTask = m_tasks.begin();

		(*m_currentTask)->startTask(m_ped,currentTick);
	}
	else
	{
		m_currentTask = m_tasks.end();
	}
	
}

void Nvidia::AbstractActor::onTick(DWORD currentTick)
{
	if (m_currentTask != m_tasks.end())
	{
		if ((*m_currentTask)->isTaskComplete(m_ped, currentTick))
		{
			(*m_currentTask)->stopTask(m_ped);

			++m_currentTask;

			if (m_currentTask != m_tasks.end())
				(*m_currentTask)->startTask(m_ped, currentTick);
		}
	}
	
}


