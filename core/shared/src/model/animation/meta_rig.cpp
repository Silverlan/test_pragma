/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2024 Silverlan
 */

#include "pragma/model/animation/meta_rig.hpp"
#include "pragma/model/animation/skeleton.hpp"

pragma::animation::BoneId pragma::animation::MetaRig::GetBoneId(const pragma::GString &type) const
{
	auto eType = get_meta_rig_bone_type_enum(type);
	if(!eType)
		return pragma::animation::INVALID_BONE_INDEX;
	return GetBoneId(*eType);
}

pragma::animation::BoneId pragma::animation::MetaRig::GetBoneId(pragma::animation::MetaRigBoneType &type) const
{
	auto i = umath::to_integral(type);
	if(i >= bones.size())
		return pragma::animation::INVALID_BONE_INDEX;
	return bones[i].boneId;
}

void pragma::animation::MetaRig::DebugPrint(Skeleton &skeleton)
{
	std::stringstream ss;
	ss << "MetaRig\n";
	auto printBone = [&ss, &skeleton](MetaRigBoneType boneType, const MetaRigBone &rigBone) {
		ss << "\t" << magic_enum::enum_name(boneType) << "\n";
		auto bone = skeleton.GetBone(rigBone.boneId).lock();
		ss << "\t\tBone: " << (bone ? bone->name : "NULL") << "\n";
		EulerAngles ang {rigBone.normalizedRotationOffset};
		ss << "\t\tNormalized rotation offset: " << ang.p << "," << ang.y << "," << ang.r << "\n";
		auto &[min, max] = rigBone.bounds;
		ss << "\t\tBounds: (" << min.x << "," << min.y << "," << min.z << ") (" << max.x << "," << max.y << "," << max.z << ")\n";
		ss << "\n";
	};
	for(size_t i = 0; i < bones.size(); ++i)
		printBone(static_cast<MetaRigBoneType>(i), bones[i]);
	Con::cout << ss.str() << Con::endl;
}
const pragma::animation::MetaRigBone *pragma::animation::MetaRig::GetBone(pragma::animation::MetaRigBoneType type) const
{
	auto i = umath::to_integral(type);
	if(i >= bones.size() || bones[i].boneId == pragma::animation::INVALID_BONE_INDEX)
		return nullptr;
	return &bones[i];
}
