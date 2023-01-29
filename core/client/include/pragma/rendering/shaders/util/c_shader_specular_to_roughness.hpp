/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_SHADER_SPECULAR_TO_ROUGHNESS_HPP__
#define __C_SHADER_SPECULAR_TO_ROUGHNESS_HPP__

#include "pragma/clientdefinitions.h"
#include <shader/prosper_shader_base_image_processing.hpp>

namespace prosper {
	class Texture;
};
namespace pragma {
	class DLLCLIENT ShaderSpecularToRoughness : public prosper::ShaderBaseImageProcessing {
	  public:
		ShaderSpecularToRoughness(prosper::IPrContext &context, const std::string &identifier);
	};
};

#endif
