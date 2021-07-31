/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_TIME_SCALE_COMPONENT_HPP__
#define __C_TIME_SCALE_COMPONENT_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <pragma/entities/components/base_time_scale_component.hpp>

namespace pragma
{
	class DLLCLIENT CTimeScaleComponent final
		: public BaseTimeScaleComponent,
		public CBaseNetComponent
	{
	public:
		CTimeScaleComponent(BaseEntity &ent) : BaseTimeScaleComponent(ent) {}
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual void ReceiveData(NetPacket &packet) override {}
		virtual bool ShouldTransmitNetData() const override {return true;}
		virtual Bool ReceiveNetEvent(pragma::NetEventId eventId,NetPacket &packet) override;
	};
};

#endif
