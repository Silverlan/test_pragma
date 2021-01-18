/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/world/c_shader_wireframe.hpp"
#include <shader/prosper_pipeline_create_info.hpp>

using namespace pragma;

ShaderWireframe::ShaderWireframe(prosper::IPrContext &context,const std::string &identifier)
	: ShaderGameWorldLightingPass(context,identifier,"world/vs_wireframe","world/fs_wireframe")
{
	// SetBaseShader<ShaderTextured3DBase>();
}

void ShaderWireframe::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	ShaderGameWorldLightingPass::InitializeGfxPipeline(pipelineInfo,pipelineIdx);
	prosper::util::set_graphics_pipeline_polygon_mode(pipelineInfo,prosper::PolygonMode::Line);
	prosper::util::set_graphics_pipeline_line_width(pipelineInfo,1.f);
}
