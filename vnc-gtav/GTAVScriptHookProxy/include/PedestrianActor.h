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