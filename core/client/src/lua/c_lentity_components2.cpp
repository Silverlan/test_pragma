/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/entities/components/liquid/c_liquid_control_component.hpp"
#include "pragma/entities/components/liquid/c_liquid_surface_simulation_component.hpp"
#include "pragma/entities/components/c_bvh_component.hpp"
#include "pragma/entities/components/c_animated_bvh_component.hpp"
#include "pragma/entities/components/c_static_bvh_cache_component.hpp"
#include "pragma/entities/components/c_static_bvh_user_component.hpp"
#include "pragma/entities/components/c_optical_camera_component.hpp"
#include "pragma/entities/components/c_motion_blur_data_component.hpp"
#include "pragma/entities/components/c_hitbox_bvh_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_fog_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_dof_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_bloom_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_glow_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_tone_mapping_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_fxaa_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_motion_blur_component.hpp"
#include "pragma/entities/components/renderers/c_renderer_pp_volumetric_component.hpp"
#include "pragma/entities/components/intersection_handler_component.hpp"
#include "pragma/entities/components/c_debug_hitbox_component.hpp"
#include "pragma/model/c_modelmesh.h"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/lua/converters/optional_converter_t.hpp>
#include <pragma/lua/lua_util_component.hpp>
#include <pragma/lua/lua_util_component_stream.hpp>
#include <pragma/lua/lua_util_class.hpp>
// --template-include-location

void RegisterLuaEntityComponents2_cl(lua_State *l, luabind::module_ &entsMod)
{
	auto defLiquidControl = pragma::lua::create_entity_component_class<pragma::CLiquidControlComponent, pragma::BaseLiquidControlComponent>("LiquidControlComponent");
	entsMod[defLiquidControl];

	auto defLiquidSurfaceSimulation = pragma::lua::create_entity_component_class<pragma::CLiquidSurfaceSimulationComponent, pragma::BaseLiquidSurfaceSimulationComponent>("LiquidSurfaceSimulationComponent");
	entsMod[defLiquidSurfaceSimulation];

	auto defBvh = pragma::lua::create_entity_component_class<pragma::CBvhComponent, pragma::BaseBvhComponent>("BvhComponent");
	entsMod[defBvh];

	auto defAnimatedBvh = pragma::lua::create_entity_component_class<pragma::CAnimatedBvhComponent, pragma::BaseEntityComponent>("AnimatedBvhComponent");
	defAnimatedBvh.def("SetUpdateLazily", &pragma::CAnimatedBvhComponent::SetUpdateLazily);
	defAnimatedBvh.def("RebuildAnimatedBvh", static_cast<void (pragma::CAnimatedBvhComponent::*)(bool)>(&pragma::CAnimatedBvhComponent::RebuildAnimatedBvh));
	entsMod[defAnimatedBvh];

	auto defStaticBvh = pragma::lua::create_entity_component_class<pragma::CStaticBvhCacheComponent, pragma::BaseStaticBvhCacheComponent>("StaticBvhCacheComponent");
	entsMod[defStaticBvh];

	auto defStaticBvhUser = pragma::lua::create_entity_component_class<pragma::CStaticBvhUserComponent, pragma::BaseStaticBvhUserComponent>("StaticBvhUserComponent");
	entsMod[defStaticBvhUser];

	auto defOpticalCamera = pragma::lua::create_entity_component_class<pragma::COpticalCameraComponent, pragma::BaseEntityComponent>("OpticalCameraComponent");
	entsMod[defOpticalCamera];

	auto defHitboxBvh = pragma::lua::create_entity_component_class<pragma::CHitboxBvhComponent, pragma::BaseEntityComponent>("HitboxBvhComponent");

	auto defDebugDrawInfo = luabind::class_<pragma::bvh::DebugDrawInfo>("DebugDrawInfo");
	defDebugDrawInfo.add_static_constant("FLAG_NONE", umath::to_integral(pragma::bvh::DebugDrawInfo::Flags::None));
	defDebugDrawInfo.add_static_constant("FLAG_DRAW_TRAVERSED_NODES_BIT", umath::to_integral(pragma::bvh::DebugDrawInfo::Flags::DrawTraversedNodesBit));
	defDebugDrawInfo.add_static_constant("FLAG_DRAW_TRAVERSED_LEAVES_BIT", umath::to_integral(pragma::bvh::DebugDrawInfo::Flags::DrawTraversedLeavesBit));
	defDebugDrawInfo.add_static_constant("FLAG_DRAW_HIT_LEAVES_BIT", umath::to_integral(pragma::bvh::DebugDrawInfo::Flags::DrawHitLeavesBit));
	defDebugDrawInfo.add_static_constant("FLAG_DRAW_TRAVERSED_MESHES_BIT", umath::to_integral(pragma::bvh::DebugDrawInfo::Flags::DrawTraversedMeshesBit));
	defDebugDrawInfo.add_static_constant("FLAG_DRAW_HIT_MESHES_BIT", umath::to_integral(pragma::bvh::DebugDrawInfo::Flags::DrawHitMeshesBit));
	defDebugDrawInfo.def(luabind::constructor<>());
	defDebugDrawInfo.def_readwrite("flags", &pragma::bvh::DebugDrawInfo::flags);
	defDebugDrawInfo.def_readwrite("pose", &pragma::bvh::DebugDrawInfo::basePose);
	defDebugDrawInfo.def_readwrite("duration", &pragma::bvh::DebugDrawInfo::duration);
	defHitboxBvh.scope[defDebugDrawInfo];

	defHitboxBvh.def("DebugDrawHitboxMeshes", &pragma::CHitboxBvhComponent::DebugDrawHitboxMeshes);
	defHitboxBvh.def(
	  "IntersectionTest", +[](pragma::CHitboxBvhComponent &bvhC, const Vector3 &origin, const Vector3 &dir, float minDist, float maxDist, const pragma::bvh::DebugDrawInfo &debugDrawInfo) -> std::optional<float> {
		  pragma::HitInfo hitInfo {};
		  if(!bvhC.IntersectionTest(origin, dir, minDist, maxDist, hitInfo, &debugDrawInfo))
			  return {};
		  return hitInfo.distance;
	  });

	entsMod[defHitboxBvh];

	auto defDebugHitbox = pragma::lua::create_entity_component_class<pragma::CDebugHitboxComponent, pragma::BaseEntityComponent>("DebugHitboxComponent");
	defDebugHitbox.def("SetHitboxColor", &pragma::CDebugHitboxComponent::SetHitboxColor);
	entsMod[defDebugHitbox];

	auto defPpFog = pragma::lua::create_entity_component_class<pragma::CRendererPpFogComponent, pragma::BaseEntityComponent>("RendererPpFogComponent");
	entsMod[defPpFog];

	auto defPpDof = pragma::lua::create_entity_component_class<pragma::CRendererPpDoFComponent, pragma::BaseEntityComponent>("RendererPpDoFComponent");
	entsMod[defPpDof];

	auto defPpBloom = pragma::lua::create_entity_component_class<pragma::CRendererPpBloomComponent, pragma::BaseEntityComponent>("RendererPpBloomComponent");
	defPpBloom.def("SetBlurRadius", &pragma::CRendererPpBloomComponent::SetBlurRadius);
	defPpBloom.def("SetBlurSigma", &pragma::CRendererPpBloomComponent::SetBlurSigma);
	defPpBloom.def("GetBlurRadius", &pragma::CRendererPpBloomComponent::GetBlurRadius);
	defPpBloom.def("GetBlurSigma", &pragma::CRendererPpBloomComponent::GetBlurSigma);
	defPpBloom.def("GetBloomThreshold", &pragma::CRendererPpBloomComponent::GetBloomThreshold);
	defPpBloom.def("SetBloomThreshold", &pragma::CRendererPpBloomComponent::SetBloomThreshold);
	defPpBloom.def("GetBlurAmount", &pragma::CRendererPpBloomComponent::GetBlurAmount);
	defPpBloom.def("SetBlurAmount", &pragma::CRendererPpBloomComponent::SetBlurAmount);
	entsMod[defPpBloom];

	auto defPpGlow = pragma::lua::create_entity_component_class<pragma::CRendererPpGlowComponent, pragma::BaseEntityComponent>("RendererPpGlowComponent");
	defPpGlow.def("SetBlurRadius", &pragma::CRendererPpGlowComponent::SetBlurRadius);
	defPpGlow.def("SetBlurSigma", &pragma::CRendererPpGlowComponent::SetBlurSigma);
	defPpGlow.def("GetBlurRadius", &pragma::CRendererPpGlowComponent::GetBlurRadius);
	defPpGlow.def("GetBlurSigma", &pragma::CRendererPpGlowComponent::GetBlurSigma);
	defPpGlow.def("GetBlurAmount", &pragma::CRendererPpGlowComponent::GetBlurAmount);
	defPpGlow.def("SetBlurAmount", &pragma::CRendererPpGlowComponent::SetBlurAmount);
	entsMod[defPpGlow];

	auto defPpToneMapping = pragma::lua::create_entity_component_class<pragma::CRendererPpToneMappingComponent, pragma::BaseEntityComponent>("RendererPpToneMappingComponent");
	defPpToneMapping.def("SetApplyToHdrImage", &pragma::CRendererPpToneMappingComponent::SetApplyToHdrImage);
	entsMod[defPpToneMapping];

	auto defPpFxaa = pragma::lua::create_entity_component_class<pragma::CRendererPpFxaaComponent, pragma::BaseEntityComponent>("RendererPpFxaaComponent");
	entsMod[defPpFxaa];

	auto defPpMotionBlur = pragma::lua::create_entity_component_class<pragma::CRendererPpMotionBlurComponent, pragma::BaseEntityComponent>("RendererPpMotionBlurComponent");
	defPpMotionBlur.add_static_constant("MOTION_BLUR_QUALITY_LOW", umath::to_integral(pragma::MotionBlurQuality::Low));
	defPpMotionBlur.add_static_constant("MOTION_BLUR_QUALITY_MEDIUM", umath::to_integral(pragma::MotionBlurQuality::Medium));
	defPpMotionBlur.add_static_constant("MOTION_BLUR_QUALITY_HIGH", umath::to_integral(pragma::MotionBlurQuality::High));
	defPpMotionBlur.def("SetAutoUpdateMotionData", &pragma::CRendererPpMotionBlurComponent::SetAutoUpdateMotionData);
	defPpMotionBlur.def("SetMotionBlurIntensity", &pragma::CRendererPpMotionBlurComponent::SetMotionBlurIntensity);
	defPpMotionBlur.def("GetMotionBlurIntensity", &pragma::CRendererPpMotionBlurComponent::GetMotionBlurIntensity);
	defPpMotionBlur.def("SetMotionBlurQuality", &pragma::CRendererPpMotionBlurComponent::SetMotionBlurQuality);
	defPpMotionBlur.def("GetMotionBlurQuality", &pragma::CRendererPpMotionBlurComponent::GetMotionBlurQuality);
	defPpMotionBlur.def("UpdateMotionBlurData", &pragma::CRendererPpMotionBlurComponent::UpdateMotionBlurData);
	entsMod[defPpMotionBlur];

	auto defPpVol = pragma::lua::create_entity_component_class<pragma::CRendererPpVolumetricComponent, pragma::BaseEntityComponent>("RendererPpVolumetricComponent");
	entsMod[defPpVol];

	auto defMotionBlurData = pragma::lua::create_entity_component_class<pragma::CMotionBlurDataComponent, pragma::BaseEntityComponent>("MotionBlurDataComponent");
	defMotionBlurData.def("UpdatePoses", &pragma::CMotionBlurDataComponent::UpdateEntityPoses);
	entsMod[defMotionBlurData];

	// --template-component-register-location
}
