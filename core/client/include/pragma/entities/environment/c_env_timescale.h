/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#ifndef __C_ENV_TIMESCALE_H__
#define __C_ENV_TIMESCALE_H__
#include "pragma/clientdefinitions.h"
#include "pragma/entities/c_baseentity.h"
#include "pragma/entities/environment/env_timescale.h"
#include "pragma/entities/components/c_entity_component.hpp"
#include <memory>

namespace pragma
{
	class DLLCLIENT CEnvTimescaleComponent final
		: public BaseEnvTimescaleComponent,
		public CBaseNetComponent
	{
	public:
		CEnvTimescaleComponent(BaseEntity &ent) : BaseEnvTimescaleComponent(ent) {}
		virtual void ReceiveData(NetPacket &packet) override;
		virtual void InitializeLuaObject(lua_State *l) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	};
};

class DLLCLIENT CEnvTimescale
	: public CBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif