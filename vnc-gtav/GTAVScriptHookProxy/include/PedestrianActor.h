
#pragma once

#include "AbstractActor.h"

namespace Nvidia
{
	class PedestrianActor : public AbstractActor
	{
	public:
		PedestrianActor(const std::string & name, const Vector3 & pos, float heading) : AbstractActor(name,pos,heading){}

		virtual void setupActor(DWORD currentTick) override;

		virtual void cleanUpActor() override;

		virtual bool isEntityOurActor(Entity e) const override { return e == m_ped; };

		virtual int id() override { return m_ped; }

	protected:

		static Ped createPedestrian(Hash modelHash, float x, float y, float z, float heading);

		static Ped createRandomPedestrian(float x, float y, float z, float heading);
	};
}