#pragma once

#pragma warning(push)

#include "../../../CSEL/source/RE/Skyrim.h"
#include "../../../CSEL/source/REL/Relocation.h"
#include "../../../CSEL/EXTERNAL/SKSE/SKSE.h"

// #include <RE/Skyrim.h>
// #include <REL/Relocation.h>
// #include <SKSE/SKSE.h>

#include "../../../OrbisUtil/Third-Party/brofield/1.0/SimpleIni.h"

// #include <SimpleIni.h>

#include "ModAPI.h"

#define SMOOTHCAM_API_COMMONLIB

#ifdef NDEBUG
#else
#endif
#pragma warning(pop)

#define DLLEXPORT __declspec(dllexport)

#define RELOCATION_ID(a_se, a_vr) REL::Offset(a_se)
#define RELOCATION_OFFSET(SE, AE) 0 // REL::VariantOffset(SE, AE, 0).offset()
#define RELOCATION_OFFSET2(SE, AE, AE629) 0 // REL::VariantOffset(SE, REL::Module::get().version().compare(SKSE::RUNTIME_SSE_1_6_629) == std::strong_ordering::less ? AE : AE629, 0).offset()

// #include "Plugin.h"
