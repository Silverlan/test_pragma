/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_ALPHA_MODE_HPP__
#define __C_ALPHA_MODE_HPP__

#include "pragma/clientdefinitions.h"
#include <cinttypes>
#include <mathutil/uvec.h>

namespace pragma
{
	enum class AlphaMode : uint32_t
	{
		Additive = 0u,
		AdditiveFull,
		Opaque,
		Masked,
		Translucent,
		Premultiplied,

		Count
	};
	DLLCLIENT bool premultiply_alpha(Vector4 &color,pragma::AlphaMode alphaMode);
};

#endif
