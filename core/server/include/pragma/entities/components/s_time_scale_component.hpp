/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_TIME_SCALE_COMPONENT_HPP__
#define __S_TIME_SCALE_COMPONENT_HPP__

#include "pragma/serverdefinitions.h"
#include "pragma/entities/components/s_entity_component.hpp"
#include <pragma/entities/components/base_time_scale_component.hpp>

namespace pragma
{
	class DLLSERVER STimeScaleComponent final
		: public BaseTimeScaleComponent
	{
	public:
		STimeScaleComponent(BaseEntity &ent) : BaseTimeScaleComponent(ent) {}
		virtual void Initialize() override;
		virtual void SetTimeScale(float timeScale) override;

		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

#endif
