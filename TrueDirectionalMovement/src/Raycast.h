#pragma once

#include "Offsets.h"

namespace Raycast
{
	struct RayHitCollector : public ConsoleRE::hkpRayHitCollector
	{
	public:
		RayHitCollector();

		virtual void AddRayHit(const ConsoleRE::hkpCdBody& cdBody, const ConsoleRE::hkpShapeRayCastCollectorOutput& hitInfo) override;

		void Reset();

		ConsoleRE::hkpShapeRayCastCollectorOutput closestHitInfo;
		bool doesHitExist = false;
	};

	/*struct AllRayHitCollector : public ConsoleRE::hkpRayHitCollector
	{
	public:
		AllRayHitCollector();

		virtual void AddRayHit(const ConsoleRE::hkpCdBody& cdBody, const ConsoleRE::hkpShapeRayCastCollectorOutput& hitInfo) override;

		inline void Reset();

		std::vector<std::pair<ConsoleRE::hkpCdBody*, ConsoleRE::hkpShapeRayCastCollectorOutput>> hits;
	};*/
}
