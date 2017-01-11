
#pragma once

#include <vector>
#include <memory>
#include "IActor.h"
#include <types.h>
#include <natives.h>

namespace Nvidia
{
	class LightActor : public IActor
	{
	public:
		LightActor(const Vector3 & pos, int red, int green, int blue, float range, float intensity) :
			m_pos(pos), m_red(red), m_green(green), m_blue(blue), m_range(range), m_intensity(intensity) {}
			
		virtual void addTask(std::unique_ptr<ITask> task) override {}

		virtual void setupActor(DWORD currentTick) override {}

		virtual void onTick(DWORD currentTick) override
		{
			GRAPHICS::DRAW_LIGHT_WITH_RANGE(m_pos.x, m_pos.y, m_pos.z, m_red, m_green, m_blue, m_range, m_intensity);
		}

		virtual void cleanUpActor() override {}

		virtual bool isEntityOurActor(Entity e) const override { return false; }

		virtual int id() override { return 0; }

	protected:

		Vector3 m_pos;
		int m_red;
		int m_green;
		int m_blue;
		float m_range;
		float m_intensity;

	};
}

