/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_DEBUG_TEXT_HPP__
#define __C_SHADER_DEBUG_TEXT_HPP__

#include "pragma/rendering/shaders/debug/c_shader_debug.hpp"

namespace pragma
{
	class DLLCLIENT ShaderDebugText
		: public ShaderDebug
	{
	public:
		ShaderDebugText(prosper::Context &context,const std::string &identifier);

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_TEXTURE;

		bool Draw(prosper::IBuffer &vertexBuffer,uint32_t vertexCount,prosper::IDescriptorSet &descSetTexture,const Mat4 &mvp=umat::identity(),const Vector4 &color=Vector4(1.f,1.f,1.f,1.f));
	protected:
		virtual void InitializeGfxPipeline(Anvil::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};

#endif
