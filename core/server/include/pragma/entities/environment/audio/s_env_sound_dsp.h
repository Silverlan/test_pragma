/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer */

#ifndef __S_ENV_SOUND_DSP_H__
#define __S_ENV_SOUND_DSP_H__
#include "pragma/serverdefinitions.h"
#include "pragma/entities/s_baseentity.h"
#include "pragma/entities/environment/audio/env_sound_dsp.h"
#include "pragma/entities/components/s_entity_component.hpp"

namespace pragma
{
	class DLLSERVER SBaseSoundDspComponent
		: public BaseEnvSoundDspComponent,
		public SBaseNetComponent
	{
	public:
		virtual void Initialize() override;
		virtual void SendData(NetPacket &packet,networking::ClientRecipientFilter &rp) override;
		virtual void SetGain(float gain) override;
		virtual bool ShouldTransmitNetData() const override {return true;}
	protected:
		using BaseEnvSoundDspComponent::BaseEnvSoundDspComponent;
	};

	class DLLSERVER SSoundDspComponent final
		: public SBaseSoundDspComponent
	{
	public:
		SSoundDspComponent(BaseEntity &ent) : SBaseSoundDspComponent(ent) {}
		virtual luabind::object InitializeLuaObject(lua_State *l) override;
	};
};

class DLLSERVER EnvSoundDsp
	: public SBaseEntity
{
public:
	virtual void Initialize() override;
};

#endif