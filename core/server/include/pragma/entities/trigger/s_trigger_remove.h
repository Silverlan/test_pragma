/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_TRIGGER_REMOVE_H__
#define __S_TRIGGER_REMOVE_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include <pragma/entities/trigger/base_trigger_remove.h>

namespace pragma
{
	class DLLSERVER STriggerRemoveComponent final
		: public BaseTriggerRemoveComponent
	{
	public:
		STriggerRemoveComponent(BaseEntity &ent) : BaseTriggerRemoveComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER TriggerRemove
	: public SBaseEntity
{
protected:
public:
	virtual void Initialize() override;
};

#endif