/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#include "stdafx_server.h"
#include "pragma/entities/trigger/s_trigger_hurt.h"
#include "pragma/entities/s_entityfactories.h"
#include <pragma/physics/physobj.h>
#include <sharedutils/util_string.h>
#include "pragma/lua/s_lentity_handles.hpp"
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(trigger_hurt,TriggerHurt);

luabind::object STriggerHurtComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<STriggerHurtComponentHandleWrapper>(l);}

void TriggerHurt::Initialize()
{
	SBaseEntity::Initialize();
	AddComponent<STriggerHurtComponent>();
}
