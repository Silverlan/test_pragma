/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_shared.h"
#include "pragma/lua/libraries/lents.h"
#include <pragma/engine.h>
#include <pragma/game/game.h>
#include "pragma/entities/baseentity.h"
#include "pragma/entities/baseworld.h"
#include "luasystem.h"
#include "pragma/lua/classes/ldef_vector.h"
#include "pragma/lua/classes/ldef_angle.h"
#include "pragma/lua/classes/lphysics.h"
#include "pragma/physics/environment.hpp"
#include "pragma/entities/entity_iterator.hpp"
#include "pragma/entities/entity_component_manager_t.hpp"
#include "pragma/entities/components/base_player_component.hpp"
#include "pragma/entities/components/base_ai_component.hpp"
#include "pragma/entities/components/base_vehicle_component.hpp"
#include "pragma/entities/components/base_weapon_component.hpp"
#include "pragma/entities/components/base_transform_component.hpp"
#include "pragma/entities/components/base_physics_component.hpp"
#include "pragma/entities/components/component_member_flags.hpp"
#include "pragma/entities/attribute_specialization_type.hpp"
#include "pragma/lua/policies/default_parameter_policy.hpp"
#include "pragma/lua/converters/optional_converter_t.hpp"
#include "pragma/lua/lentity_type.hpp"
#include "pragma/lua/lua_entity_iterator.hpp"
#include "pragma/lua/sh_lua_component.hpp"
#include "pragma/lua/policies/core_policies.hpp"
#include "pragma/lua/types/udm.hpp"
#include <udm.hpp>
#include <sharedutils/magic_enum.hpp>

extern DLLNETWORK Engine *engine;

//void test_lua_policies(lua_State *l);
void Lua::ents::register_library(lua_State *l)
{
	//test_lua_policies(l);
	auto entsMod = luabind::module(l,"ents");
	entsMod[
		luabind::def("create",create),

		luabind::def("get_all",static_cast<tb<type<BaseEntity>>(*)(lua_State*)>(get_all)),
		luabind::def("get_all",static_cast<tb<type<BaseEntity>>(*)(lua_State*,func<type<BaseEntity>>)>(get_all)),
		luabind::def("get_all",static_cast<tb<type<BaseEntity>>(*)(lua_State*,EntityIterator::FilterFlags)>(get_all)),
		luabind::def("get_all",static_cast<tb<type<BaseEntity>>(*)(lua_State*,EntityIterator::FilterFlags,const tb<LuaEntityIteratorFilterBase>&)>(get_all)),
		luabind::def("get_all",static_cast<tb<type<BaseEntity>>(*)(lua_State*,const tb<LuaEntityIteratorFilterBase>&)>(get_all)),
		luabind::def("get_all_c",static_cast<tb<type<pragma::BaseEntityComponent>>(*)(lua_State*,func<type<pragma::BaseEntityComponent>>)>(get_all_c)),
		luabind::def("get_spawned",get_spawned),
		luabind::def("get_players",get_players),
		luabind::def("get_npcs",get_npcs),
		luabind::def("get_weapons",get_weapons),
		luabind::def("get_vehicles",get_vehicles),
		luabind::def("get_world",get_world),
		luabind::def("get_by_index",get_by_index),
		luabind::def("get_by_local_index",get_by_local_index),
		luabind::def("find_by_uuid",find_by_unique_index),
		luabind::def("get_null",get_null),
		luabind::def("find_by_filter",find_by_filter),
		luabind::def("find_by_class",find_by_class),
		luabind::def("find_by_name",find_by_name),
		luabind::def("find_by_component",find_by_component),
		luabind::def("find_in_sphere",find_in_sphere),
		luabind::def("find_in_aabb",find_in_aabb),
		luabind::def("find_in_cone",find_in_cone),
		luabind::def("create_trigger",static_cast<type<BaseEntity>(*)(lua_State*,const Vector3&,pragma::physics::IShape&)>(create_trigger)),
		luabind::def("create_trigger",static_cast<type<BaseEntity>(*)(lua_State*,const Vector3&,const Vector3&,const Vector3&,const EulerAngles&)>(create_trigger)),
		luabind::def("create_trigger",static_cast<type<BaseEntity>(*)(lua_State*,const Vector3&,float)>(create_trigger)),
		luabind::def("create_prop",&create_prop),
		luabind::def("create_prop",+[](lua_State *l,const std::string &mdl,const Vector3 *origin,const EulerAngles *angles) -> type<BaseEntity> {
			return create_prop(l,mdl,origin,angles,false);
		}),
		luabind::def("create_prop",+[](lua_State *l,const std::string &mdl,const Vector3 *origin) -> type<BaseEntity> {
			return create_prop(l,mdl,origin,nullptr,false);
		}),
		luabind::def("create_prop",+[](lua_State *l,const std::string &mdl) -> type<BaseEntity> {
			return create_prop(l,mdl,nullptr,nullptr,false);
		}),
		luabind::def("register",static_cast<void(*)(lua_State*,const std::string&,const Lua::classObject&)>(Lua::ents::register_class)),
		luabind::def("register",static_cast<void(*)(lua_State*,const std::string&,const luabind::tableT<luabind::variant<std::string,pragma::ComponentId>>&,LuaEntityType)>(Lua::ents::register_class)),
		luabind::def("register",+[](lua_State *l,const std::string &className,const luabind::tableT<luabind::variant<std::string,pragma::ComponentId>> &tComponents) {
			register_class(l,className,tComponents,LuaEntityType::Default);
		}),
		luabind::def("get_closest",get_closest),
		luabind::def("get_farthest",get_farthest),
		luabind::def("get_sorted_by_distance",get_sorted_by_distance),
		luabind::def("get_random",get_random),
		luabind::def("get_component_name",get_component_name),
		luabind::def("get_component_id",get_component_id),
		luabind::def("register_component_net_event",register_component_net_event),
		luabind::def("get_registered_component_types",+[](lua_State *l,Game &game) -> Lua::tb<pragma::ComponentId> {
			auto &manager = game.GetEntityComponentManager();
			auto t = luabind::newtable(l);
			for(uint32_t idx = 1;auto &componentInfo : manager.GetRegisteredComponentTypes())
			{
				if(componentInfo.id == pragma::INVALID_COMPONENT_ID)
					continue;
				t[idx++] = componentInfo.id;
			}
			return t;
		}),
		luabind::def("get_component_info",+[](lua_State *l,Game &game,pragma::ComponentId componentId) {
			auto &manager = game.GetEntityComponentManager();
			return manager.GetComponentInfo(componentId);
		}),
		luabind::def("find_component_id",+[](lua_State *l,Game &game,const std::string &name) -> std::optional<pragma::ComponentId> {
			auto &manager = game.GetEntityComponentManager();
			pragma::ComponentId componentId;
			if(manager.GetComponentTypeId(name,componentId) == false)
				return {};
			return componentId;
		}),
		luabind::def("find_installed_custom_components",+[](lua_State *l,Game &game) -> Lua::tb<std::string> {
			std::vector<std::string> dirs;
			std::string rootPath {"lua/entities/components/"};
			filemanager::find_files(rootPath +'*',nullptr,&dirs);
			auto customComponents = luabind::newtable(l);
			auto nwStateDirName = game.GetLuaNetworkDirectoryName();
			auto luaFileName = game.GetLuaNetworkFileName();

			auto nwStateDirNameC = nwStateDirName;
			auto luaFileNameC = luaFileName;
			ufile::remove_extension_from_filename(nwStateDirNameC,std::vector<std::string>{"lua"});
			ufile::remove_extension_from_filename(luaFileNameC,std::vector<std::string>{"lua"});
			nwStateDirNameC += ".clua";
			luaFileNameC += ".clua";
			for(uint32_t idx=1;auto &dir : dirs)
			{
				auto cdir = rootPath +dir +'/';
				if(
					filemanager::exists(cdir +luaFileName) || filemanager::exists(cdir +nwStateDirName +'/' +luaFileName) ||
					filemanager::exists(cdir +luaFileNameC) || filemanager::exists(cdir +nwStateDirName +'/' +luaFileNameC)
				)
					customComponents[idx++] = dir;
			}
			return customComponents;
		}),
		luabind::def("get_registered_entity_types",+[](Game &game) -> std::vector<std::string> {
			std::vector<std::string> entities;
			game.GetRegisteredEntities(entities,entities);
			return entities;
		}),
		luabind::def("add_component_creation_listener",+[](Game &game,pragma::ComponentId id,Lua::func<void,pragma::BaseEntityComponent> callback) {
			EntityIterator it {game};
			it.AttachFilter<EntityIteratorFilterComponent>(id);
			auto entIt = it.begin();
			if(entIt != it.end())
			{
				auto *ent = *entIt;
				callback(ent->FindComponent(id)->GetLuaObject());
			}

			return game.GetEntityComponentManager().AddCreationCallback(id,[callback](std::reference_wrapper<pragma::BaseEntityComponent> c) mutable {
				callback(c.get().GetLuaObject());
			});
		}),
		luabind::def("add_component_creation_listener",+[](Game &game,const std::string &componentName,Lua::func<void,pragma::BaseEntityComponent> callback) {
			EntityIterator it {game};
			it.AttachFilter<EntityIteratorFilterComponent>(componentName);
			auto entIt = it.begin();
			if(entIt != it.end())
			{
				auto *ent = *entIt;
				callback(ent->FindComponent(componentName)->GetLuaObject());
			}

			return game.GetEntityComponentManager().AddCreationCallback(componentName,[callback](std::reference_wrapper<pragma::BaseEntityComponent> c) mutable {
				callback(c.get().GetLuaObject());
			});
		})
	];
	Lua::RegisterLibraryEnums(l,"ents",{
		{"MEMBER_TYPE_STRING",umath::to_integral(pragma::ents::EntityMemberType::String)},
		{"MEMBER_TYPE_INT8",umath::to_integral(pragma::ents::EntityMemberType::Int8)},
		{"MEMBER_TYPE_UINT8",umath::to_integral(pragma::ents::EntityMemberType::UInt8)},
		{"MEMBER_TYPE_INT16",umath::to_integral(pragma::ents::EntityMemberType::Int16)},
		{"MEMBER_TYPE_UINT16",umath::to_integral(pragma::ents::EntityMemberType::UInt16)},
		{"MEMBER_TYPE_INT32",umath::to_integral(pragma::ents::EntityMemberType::Int32)},
		{"MEMBER_TYPE_UINT32",umath::to_integral(pragma::ents::EntityMemberType::UInt32)},
		{"MEMBER_TYPE_INT64",umath::to_integral(pragma::ents::EntityMemberType::Int64)},
		{"MEMBER_TYPE_UINT64",umath::to_integral(pragma::ents::EntityMemberType::UInt64)},
		{"MEMBER_TYPE_FLOAT",umath::to_integral(pragma::ents::EntityMemberType::Float)},
		{"MEMBER_TYPE_DOUBLE",umath::to_integral(pragma::ents::EntityMemberType::Double)},
		{"MEMBER_TYPE_BOOLEAN",umath::to_integral(pragma::ents::EntityMemberType::Boolean)},
		{"MEMBER_TYPE_VECTOR2",umath::to_integral(pragma::ents::EntityMemberType::Vector2)},
		{"MEMBER_TYPE_VECTOR3",umath::to_integral(pragma::ents::EntityMemberType::Vector3)},
		{"MEMBER_TYPE_VECTOR4",umath::to_integral(pragma::ents::EntityMemberType::Vector4)},
		{"MEMBER_TYPE_QUATERNION",umath::to_integral(pragma::ents::EntityMemberType::Quaternion)},
		{"MEMBER_TYPE_EULER_ANGLES",umath::to_integral(pragma::ents::EntityMemberType::EulerAngles)},
		{"MEMBER_TYPE_SRGBA",umath::to_integral(pragma::ents::EntityMemberType::Srgba)},
		{"MEMBER_TYPE_HDR_COLOR",umath::to_integral(pragma::ents::EntityMemberType::HdrColor)},
		{"MEMBER_TYPE_TRANSFORM",umath::to_integral(pragma::ents::EntityMemberType::Transform)},
		{"MEMBER_TYPE_SCALED_TRANSFORM",umath::to_integral(pragma::ents::EntityMemberType::ScaledTransform)},
		{"MEMBER_TYPE_MAT4",umath::to_integral(pragma::ents::EntityMemberType::Mat4)},
		{"MEMBER_TYPE_MAT3X4",umath::to_integral(pragma::ents::EntityMemberType::Mat3x4)},
		{"MEMBER_TYPE_HALF",umath::to_integral(pragma::ents::EntityMemberType::Half)},
		{"MEMBER_TYPE_VECTOR2I",umath::to_integral(pragma::ents::EntityMemberType::Vector2i)},
		{"MEMBER_TYPE_VECTOR3I",umath::to_integral(pragma::ents::EntityMemberType::Vector3i)},
		{"MEMBER_TYPE_VECTOR4I",umath::to_integral(pragma::ents::EntityMemberType::Vector4i)},
		{"MEMBER_TYPE_ENTITY",umath::to_integral(pragma::ents::EntityMemberType::Entity)},
		{"MEMBER_TYPE_MULTI_ENTITY",umath::to_integral(pragma::ents::EntityMemberType::MultiEntity)},
		{"MEMBER_TYPE_COUNT",umath::to_integral(pragma::ents::EntityMemberType::Count)},
		{"MEMBER_TYPE_LAST",umath::to_integral(pragma::ents::EntityMemberType::Last)},
		{"MEMBER_TYPE_INVALID",umath::to_integral(pragma::ents::EntityMemberType::Invalid)},
		
		{"INVALID_COMPONENT_ID",pragma::INVALID_COMPONENT_ID},
		{"INVALID_COMPONENT_MEMBER_INDEX",pragma::INVALID_COMPONENT_MEMBER_INDEX}
	});

	auto componentInfoDef = luabind::class_<pragma::ComponentInfo>("ComponentInfo");
	componentInfoDef.def_readonly("name",&pragma::ComponentInfo::name);
	componentInfoDef.def_readonly("id",&pragma::ComponentInfo::id);
	componentInfoDef.def_readonly("flags",&pragma::ComponentInfo::flags);
	componentInfoDef.def("GetMemberCount",+[](const pragma::ComponentInfo &componentInfo) {return componentInfo.members.size();});
	componentInfoDef.def("FindMemberIndex",+[](const pragma::ComponentInfo &componentInfo,const std::string &name) -> std::optional<pragma::ComponentMemberIndex> {
		auto it = componentInfo.memberNameToIndex.find(name);
		if(it == componentInfo.memberNameToIndex.end())
			return {};
		return it->second;
	});
	componentInfoDef.def("GetMemberInfo",+[](const pragma::ComponentInfo &componentInfo,uint32_t memberIdx) -> const pragma::ComponentMemberInfo* {
		if(memberIdx >= componentInfo.members.size())
			return nullptr;
		return &componentInfo.members[memberIdx];
	});
	componentInfoDef.def("GetMemberInfo",+[](Game &game,const pragma::ComponentInfo &componentInfo,const std::string &name) -> const pragma::ComponentMemberInfo* {
		auto lname = name;
		if(umath::is_flag_set(componentInfo.flags,pragma::ComponentFlags::LuaBased))
		{
			auto &manager = game.GetLuaEntityManager();
			auto *o = manager.GetClassObject(componentInfo.name);
			if(!o)
				return nullptr;
			auto *infos = pragma::BaseLuaBaseEntityComponent::GetMemberInfos(*o);
			if(!infos)
				return nullptr;
			auto it = std::find_if(infos->begin(),infos->end(),[&lname](const pragma::BaseLuaBaseEntityComponent::MemberInfo &memberInfo) {
				return memberInfo.memberName == lname;
			});
			if(it == infos->end() || !it->componentMemberInfo.has_value())
				return nullptr;
			return &*it->componentMemberInfo;
		}
		ustring::to_lower(lname);
		auto it = componentInfo.memberNameToIndex.find(lname);
		if(it == componentInfo.memberNameToIndex.end())
			return nullptr;
		auto memberIdx = it->second;
		if(memberIdx >= componentInfo.members.size())
			return nullptr;
		return &componentInfo.members[memberIdx];
	});
	
	auto memberInfoDef = luabind::class_<pragma::ComponentMemberInfo>("MemberInfo");
	memberInfoDef.add_static_constant("FLAG_NONE",umath::to_integral(pragma::ComponentMemberFlags::None));
	memberInfoDef.add_static_constant("FLAG_HIDE_IN_INTERFACE_BIT",umath::to_integral(pragma::ComponentMemberFlags::HideInInterface));
	memberInfoDef.def("__tostring",+[](const pragma::ComponentMemberInfo &memberInfo) -> std::string {
		std::stringstream ss;
		ss<<"MemberInfo";
		ss<<"["<<memberInfo.GetName()<<"]";
		ss<<"[Type:"<<magic_enum::enum_name(memberInfo.type)<<"]";
		ss<<"[Spec:"<<magic_enum::enum_name(memberInfo.GetSpecializationType());
		auto *c = memberInfo.GetCustomSpecializationType();
		if(c)
			ss<<"("<<*c<<")";
		ss<<"]";
		if(pragma::ents::is_udm_member_type(memberInfo.type))
		{
			udm::visit_ng(pragma::ents::member_type_to_udm_type(memberInfo.type),[&memberInfo,&ss](auto tag) {
				using T = decltype(tag)::type;
				if constexpr(udm::is_convertible<T,std::string>())
				{
					T def;
					if(memberInfo.GetDefault<T>(def))
					{
						auto defStr = udm::convert<T,std::string>(def);
						ss<<"[Def:"<<defStr<<"]";
					}
				}
			});
		}
		return ss.str();
	});
	
	memberInfoDef.def("SetFlags",&pragma::ComponentMemberInfo::SetFlags);
	memberInfoDef.def("GetFlags",&pragma::ComponentMemberInfo::GetFlags);
	memberInfoDef.def("HasFlag",&pragma::ComponentMemberInfo::HasFlag);
	memberInfoDef.def("SetFlag",&pragma::ComponentMemberInfo::SetFlag);
	memberInfoDef.def("SetFlag",&pragma::ComponentMemberInfo::SetFlag,luabind::default_parameter_policy<3,true>{});

	memberInfoDef.def("IsEnum",&pragma::ComponentMemberInfo::IsEnum);
	memberInfoDef.def("ValueToEnumName",&pragma::ComponentMemberInfo::ValueToEnumName);
	memberInfoDef.def("EnumNameToValue",&pragma::ComponentMemberInfo::EnumNameToValue);
	memberInfoDef.def("GetEnumValues",+[](const pragma::ComponentMemberInfo &memberInfo) -> std::optional<std::vector<int64_t>> {
		std::vector<int64_t> values;
		if(!memberInfo.GetEnumValues(values))
			return {};
		return values;
	});
	memberInfoDef.def_readonly("type",&pragma::ComponentMemberInfo::type);
	memberInfoDef.property("name",+[](lua_State *l,const pragma::ComponentMemberInfo &memInfo) {
		return memInfo.GetName();
	});
	memberInfoDef.property("nameHash",+[](lua_State *l,const pragma::ComponentMemberInfo &memInfo) {
		return memInfo.GetNameHash();
	});
	memberInfoDef.property("specializationType",+[](lua_State *l,const pragma::ComponentMemberInfo &memInfo) {
		return memInfo.GetSpecializationType();
	});
	memberInfoDef.property("customSpecializationType",+[](lua_State *l,const pragma::ComponentMemberInfo &memInfo) {
		return memInfo.GetCustomSpecializationType();
	});
	memberInfoDef.property("minValue",+[](lua_State *l,const pragma::ComponentMemberInfo &memInfo) {
		return memInfo.GetMin();
	});
	memberInfoDef.property("maxValue",+[](lua_State *l,const pragma::ComponentMemberInfo &memInfo) {
		return memInfo.GetMax();
	});
	memberInfoDef.property("stepSize",+[](lua_State *l,const pragma::ComponentMemberInfo &memInfo) {
		return memInfo.GetStepSize();
	});
	memberInfoDef.property("metaData",+[](lua_State *l,const pragma::ComponentMemberInfo &memInfo) -> udm::PProperty {
		return memInfo.GetMetaData();
	});
	memberInfoDef.property("default",+[](lua_State *l,const pragma::ComponentMemberInfo &memInfo) -> udm_type {
		// Default value is currently only allowed for UDM types. Tag: component-member-udm-default
		if(!pragma::ents::is_udm_member_type(memInfo.type))
			return nil;
		return udm::visit(pragma::ents::member_type_to_udm_type(memInfo.type),[&memInfo,l](auto tag) {
			using T = decltype(tag)::type;
			constexpr auto type = udm::type_to_enum<T>();
			if constexpr(type != udm::Type::Element && !udm::is_array_type(type))
			{
				T val;
				if(!memInfo.GetDefault(val))
					return nil;
				return luabind::object{l,std::move(val)};
			}
			else
				return nil;
		});
	});
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_NONE",umath::to_integral(pragma::AttributeSpecializationType::None));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_COLOR",umath::to_integral(pragma::AttributeSpecializationType::Color));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_DISTANCE",umath::to_integral(pragma::AttributeSpecializationType::Distance));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_LIGHT_INTENSITY",umath::to_integral(pragma::AttributeSpecializationType::LightIntensity));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_CUSTOM",umath::to_integral(pragma::AttributeSpecializationType::Custom));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_ANGLE",umath::to_integral(pragma::AttributeSpecializationType::Angle));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_FILE",umath::to_integral(pragma::AttributeSpecializationType::File));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_DIRECTORY",umath::to_integral(pragma::AttributeSpecializationType::Directory));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_PLANE",umath::to_integral(pragma::AttributeSpecializationType::Plane));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_NORMAL",umath::to_integral(pragma::AttributeSpecializationType::Normal));
	memberInfoDef.add_static_constant("SPECIALIZATION_TYPE_FRACTION",umath::to_integral(pragma::AttributeSpecializationType::Fraction));
	static_assert(umath::to_integral(pragma::AttributeSpecializationType::Count) == 11u);
	componentInfoDef.scope[memberInfoDef];

	entsMod[componentInfoDef];
}

Lua::type<BaseEntity> Lua::ents::create(lua_State *l,const std::string &classname)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();

	auto *ent = game->CreateEntity(classname);
	if(ent == NULL)
		return nil;
	return ent->GetLuaObject();
}

Lua::type<BaseEntity> Lua::ents::create_prop(lua_State *l,const std::string &mdl,const Vector3 *origin,const EulerAngles *angles,bool physicsProp=false)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	auto *ent = game->CreateEntity(physicsProp ? "prop_physics" : "prop_dynamic");
	if(ent == nullptr)
		return nil;
	if(origin)
		ent->SetPosition(*origin);
	if(angles)
		ent->SetRotation(uquat::create(*angles));
	ent->SetModel(mdl);
	ent->Spawn();
	return ent->GetLuaObject();
}

namespace Lua::ents
{
	Lua::type<BaseEntity> create_trigger(lua_State *l,const Vector3 &origin,const EulerAngles *angles,pragma::physics::IConvexShape *shape)
	{
		auto *state = engine->GetNetworkState(l);
		auto *game = state->GetGameState();
		auto *ent = game->CreateEntity("trigger_touch");
		if(ent == nullptr)
			return nil;
		auto pTrComponent = ent->GetTransformComponent();
		if(pTrComponent != nullptr)
		{
			if(angles != nullptr)
				pTrComponent->SetAngles(*angles);
			pTrComponent->SetPosition(origin);
		}
		ent->SetKeyValue("spawnflags","1144"); // TODO: Use enums
		ent->Spawn();
		if(shape != nullptr)
		{
			auto pPhysComponent = ent->GetPhysicsComponent();
			if(pPhysComponent != nullptr)
				pPhysComponent->InitializePhysics(*shape);
		}
		return ent->GetLuaObject();
	}
};
Lua::type<BaseEntity> Lua::ents::create_trigger(lua_State *l,const Vector3 &origin,float radius)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *phys = game->GetPhysicsEnvironment();
	auto shape = phys->CreateSphereShape(radius,phys->GetGenericMaterial());
	return Lua::ents::create_trigger(l,origin,nullptr,dynamic_cast<pragma::physics::IConvexShape*>(shape.get()));
}
Lua::type<BaseEntity> Lua::ents::create_trigger(lua_State *l,const Vector3 &origin,pragma::physics::IShape &shape)
{
	if(shape.IsConvex() == false)
	{
		Con::cwar<<"WARNING: Cannot create trigger_touch entity with non-convex physics shape!"<<Con::endl;
		return nil;
	}
	auto cvShape = std::dynamic_pointer_cast<pragma::physics::IConvexShape>(shape.shared_from_this());
	return Lua::ents::create_trigger(l,origin,nullptr,cvShape.get());
}

Lua::type<BaseEntity> Lua::ents::create_trigger(lua_State *l,const Vector3 &origin,const Vector3 &min,const Vector3 &max,const EulerAngles &angles)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto *phys = game->GetPhysicsEnvironment();

	auto extents = max -min;
	auto center = (max +min) *0.5f;
	origin += center;
	auto shape = phys->CreateBoxShape(extents *0.5f,phys->GetGenericMaterial());
	return Lua::ents::create_trigger(l,origin,&angles,shape.get());
}

static Lua::tb<Lua::type<BaseEntity>> entities_to_table(lua_State *l,std::vector<BaseEntity*> &ents)
{
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(auto *ent : ents)
	{
		if(!ent)
			continue;
		t[idx++] = ent->GetLuaObject();
	}
	return t;
}

static Lua::tb<Lua::type<BaseEntity>> entities_to_table(lua_State *l,EntityIterator &entIt)
{
	auto t = luabind::newtable(l);
	uint32_t idx = 1;
	for(auto *ent : entIt)
		t[idx++] = ent->GetLuaObject();
	return t;
}

static void iterate_entities(lua_State *l,const std::function<void(BaseEntity*)> &fCallback)
{
	auto fcIterator = 1;
	Lua::CheckFunction(l,fcIterator);
	BaseEntity *ent = nullptr;
	do
	{
		Lua::PushValue(l,fcIterator); /* 1 */
		Lua::Call(l,0,1); /* 1 */
		if(Lua::IsSet(l,-1))
		{
			ent = luabind::object_cast<BaseEntity*>(luabind::object{luabind::from_stack(l,-1)});
			if(ent != nullptr)
				fCallback(ent);
		}
		else
			ent = nullptr;
		Lua::Pop(l,1); /* 0 */
	}
	while(ent != nullptr);
}

Lua::opt<Lua::mult<Lua::type<BaseEntity>,double>> Lua::ents::get_closest(lua_State *l,const Vector3 &origin)
{
	auto dClosest = std::numeric_limits<float>::max();
	BaseEntity *entClosest = nullptr;
	iterate_entities(l,[&dClosest,&entClosest,&origin](BaseEntity *ent) {
		auto pTransformComponent = ent->GetTransformComponent();
		if(!pTransformComponent)
			return;
		auto d = uvec::distance_sqr(pTransformComponent->GetPosition(),origin);
		if(d >= dClosest)
			return;
		dClosest = d;
		entClosest = ent;
	});
	if(entClosest == nullptr)
		return nil;
	return Lua::mult<Lua::type<BaseEntity>,double>{l,entClosest->GetLuaObject(),dClosest};
}
Lua::opt<Lua::mult<Lua::type<BaseEntity>,double>> Lua::ents::get_farthest(lua_State *l,const Vector3 &origin)
{
	auto dFarthest = -1.f;
	BaseEntity *entClosest = nullptr;
	iterate_entities(l,[&dFarthest,&entClosest,&origin](BaseEntity *ent) {
		auto pTransformComponent = ent->GetTransformComponent();
		if(!pTransformComponent)
			return;
		auto d = uvec::distance_sqr(pTransformComponent->GetPosition(),origin);
		if(d <= dFarthest)
			return;
		dFarthest = d;
		entClosest = ent;
	});
	if(entClosest == nullptr)
		return nil;
	return Lua::mult<Lua::type<BaseEntity>,double>{l,entClosest->GetLuaObject(),dFarthest};
}
Lua::tb<Lua::type<BaseEntity>> Lua::ents::get_sorted_by_distance(lua_State *l,const Vector3 &origin)
{
	std::vector<std::pair<BaseEntity*,float>> ents {};
	iterate_entities(l,[&ents,&origin](BaseEntity *ent) {
		auto pTransformComponent = ent->GetTransformComponent();
		if(!pTransformComponent)
			return;
		auto d = uvec::distance_sqr(pTransformComponent->GetPosition(),origin);
		ents.push_back({ent,d});
	});
	std::sort(ents.begin(),ents.end(),[](const std::pair<BaseEntity*,float> &a,const std::pair<BaseEntity*,float> &b) {
		return a.second < b.second;
	});
	auto t = luabind::newtable(l);
	auto idx = 1;
	for(auto &pair : ents)
		t[idx++] = pair.first->GetLuaObject();
	return t;
}
Lua::type<BaseEntity> Lua::ents::get_random(lua_State *l)
{
	std::vector<BaseEntity*> ents {};
	iterate_entities(l,[&ents](BaseEntity *ent) {ents.push_back(ent);});
	if(ents.empty())
		return nil;
	auto r = umath::random(0,ents.size() -1);
	auto *ent = ents.at(r);
	return ent->GetLuaObject();
}

Lua::opt<std::string> Lua::ents::get_component_name(lua_State *l,pragma::ComponentId componentId)
{
	auto *info = engine->GetNetworkState(l)->GetGameState()->GetEntityComponentManager().GetComponentInfo(componentId);
	if(info == nullptr)
		return nil;
	return {l,info->name};
}
Lua::opt<uint32_t> Lua::ents::get_component_id(lua_State *l,const std::string &componentName)
{
	pragma::ComponentId componentId;
	if(engine->GetNetworkState(l)->GetGameState()->GetEntityComponentManager().GetComponentTypeId(componentName,componentId) == false)
		return nil;
	return {l,componentId};
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::get_all(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	std::vector<BaseEntity*> *ents;
	game->GetEntities(&ents);
	return entities_to_table(l,*ents);
}
Lua::tb<Lua::type<BaseEntity>> Lua::ents::get_all(lua_State *l,func<type<BaseEntity>> func)
{
	std::vector<BaseEntity*> ents {};
	iterate_entities(l,[&ents](BaseEntity *ent) {ents.push_back(ent);});
	if(ents.empty())
		return luabind::newtable(l);
	auto t = luabind::newtable(l);
	auto idx = 1;
	for(auto *ent : ents)
	{
		auto &o = ent->GetLuaObject();
		//if(!luabind::object_cast<bool>(func(o)))
		//	continue;
		t[idx++] = o;
	}
	return t;
}
static void iterate_entity_components(lua_State *l,const std::function<void(pragma::BaseEntityComponent*)> &fCallback)
{
	auto fcIterator = 1;
	Lua::CheckFunction(l,fcIterator);
	pragma::BaseEntityComponent *ent = nullptr;
	do
	{
		Lua::PushValue(l,fcIterator); /* 1 */
		Lua::Call(l,0,2); /* 1 */
		if(Lua::IsSet(l,-1))
		{
			ent = luabind::object_cast<pragma::BaseEntityComponent*>(luabind::object{luabind::from_stack(l,-1)});
			if(ent != nullptr)
				fCallback(ent);
		}
		else
			ent = nullptr;
		Lua::Pop(l,2); /* 0 */
	}
	while(ent != nullptr);
}
Lua::tb<Lua::type<pragma::BaseEntityComponent>> Lua::ents::get_all_c(lua_State *l,func<type<pragma::BaseEntityComponent>> func)
{
	std::vector<pragma::BaseEntityComponent*> ents {};
	iterate_entity_components(l,[&ents](pragma::BaseEntityComponent *ent) {ents.push_back(ent);});
	if(ents.empty())
		return luabind::newtable(l);
	auto t = luabind::newtable(l);
	auto idx = 1;
	for(auto *ent : ents)
	{
		auto &o = ent->GetLuaObject();
		//if(!luabind::object_cast<bool>(func(o)))
		//	continue;
		t[idx++] = o;
	}
	return t;
}
Lua::tb<Lua::type<BaseEntity>> Lua::ents::get_all(lua_State *l,EntityIterator::FilterFlags filterFlags)
{
	auto it = create_lua_entity_iterator(l,nil,filterFlags);

	auto t = luabind::newtable(l);
	auto idx = 1u;
	for(auto o : it)
		t[idx++] = o;
	return t;
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::get_all(lua_State *l,EntityIterator::FilterFlags filterFlags,const tb<LuaEntityIteratorFilterBase> &filters)
{
	auto it = create_lua_entity_iterator(l,filters,filterFlags);
	auto t = luabind::newtable(l);
	auto idx = 1u;
	for(auto o : it)
		t[idx++] = o;
	return t;
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::get_all(lua_State *l,const tb<LuaEntityIteratorFilterBase> &filters)
{
	return get_all(l,EntityIterator::FilterFlags::Default,filters);
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::get_spawned(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	std::vector<BaseEntity*> ents;
	game->GetSpawnedEntities(&ents);
	return entities_to_table(l,ents);
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::get_players(lua_State *l)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	auto t = luabind::newtable(l);
	auto idx = 1u;
	EntityIterator it {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending | EntityIterator::FilterFlags::Player};
	for(auto *ent : it)
		t[idx++] = ent->GetPlayerComponent().get()->GetLuaObject();
	return t;
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::get_npcs(lua_State *l)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	auto t = luabind::newtable(l);
	auto idx = 1u;
	EntityIterator it {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending | EntityIterator::FilterFlags::NPC};
	for(auto *ent : it)
		t[idx++] = ent->GetAIComponent().get()->GetLuaObject();
	return t;
}
Lua::tb<Lua::type<BaseEntity>> Lua::ents::get_weapons(lua_State *l)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	auto t = luabind::newtable(l);
	auto idx = 1u;
	EntityIterator it {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending | EntityIterator::FilterFlags::Weapon};
	for(auto *ent : it)
		t[idx++] = ent->GetWeaponComponent().get()->GetLuaObject();
	return t;
}
Lua::tb<Lua::type<pragma::BaseVehicleComponent>> Lua::ents::get_vehicles(lua_State *l)
{
	auto &game = *engine->GetNetworkState(l)->GetGameState();
	auto t = luabind::newtable(l);
	auto idx = 1u;
	EntityIterator it {game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending | EntityIterator::FilterFlags::Vehicle};
	for(auto *ent : it)
		t[idx++] = ent->GetVehicleComponent().get()->GetLuaObject();
	return t;
}

Lua::opt<Lua::type<BaseEntity>> Lua::ents::get_world(lua_State *l)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	auto *pWorld = game->GetWorld();
	if(pWorld == nullptr)
		return nil;
	BaseEntity &world = pWorld->GetEntity();
	return world.GetLuaObject();
}

Lua::opt<Lua::type<BaseEntity>> Lua::ents::get_by_index(lua_State *l,uint32_t idx)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	BaseEntity *ent = game->GetEntity(idx);
	if(ent == NULL)
		return nil;
	return ent->GetLuaObject();
}

Lua::opt<Lua::type<BaseEntity>> Lua::ents::get_by_local_index(lua_State *l,uint32_t idx)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();
	BaseEntity *ent = game->GetEntityByLocalIndex(idx);
	if(ent == NULL)
		return nil;
	return ent->GetLuaObject();
}

Lua::opt<Lua::type<BaseEntity>> Lua::ents::find_by_unique_index(lua_State *l,const std::string &uuid)
{
	auto uniqueIndex = util::uuid_string_to_bytes(uuid);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	EntityIterator entIt {*game,EntityIterator::FilterFlags::Default | EntityIterator::FilterFlags::Pending};
	auto it = std::find_if(entIt.begin(),entIt.end(),[uniqueIndex](const BaseEntity *ent) {
		return ent->GetUuid() == uniqueIndex;
	});
	if(it == entIt.end())
		return nil;
	return it->GetLuaObject();
}

namespace luabind::detail
{
	template< typename ValueType,typename BaseType >
	void make_null_value(lua_State* L, ValueType&& val)
	{
		// See luabind/detail/make_instance.hpp
		detail::class_registry* registry = luabind::detail::class_registry::get_registry(L);
		auto typeId = type_id{typeid(BaseType)};
		auto *cls = registry->find_class(typeId);
		if(!cls)
		{
			lua_pushnil(L);
			return;
		}
		auto &classIdMap = cls->classes();
		auto classId = classIdMap.get(typeId);
		if(classId == unknown_class)
		{
			lua_pushnil(L);
			return;
		}

		object_rep* instance = push_new_instance(L, cls);

		using value_type = typename std::remove_reference<ValueType>::type;
		using holder_type = pointer_like_holder<value_type>;

		void* storage = instance->allocate(sizeof(holder_type));

		try {
			new (storage) holder_type(L, std::forward<ValueType>(val), classId, nullptr);
		}
		catch(...) {
			instance->deallocate(storage);
			lua_pop(L, 1);
			throw;
		}

		instance->set_instance(static_cast<holder_type*>(storage));
	}
};

Lua::type<EntityHandle> Lua::ents::get_null(lua_State *l)
{
	luabind::detail::make_null_value<EntityHandle,BaseEntity>(l,EntityHandle{});
	luabind::object o {luabind::from_stack(l,-1)};
	Lua::Pop(l,1);
	return o;
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::find_by_filter(lua_State *l,const std::string &name)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();

	EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterEntity>(name);
	return entities_to_table(l,entIt);
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::find_by_class(lua_State *l,const std::string &className)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();

	EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterClass>(className);
	return entities_to_table(l,entIt);
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::find_by_name(lua_State *l,const std::string &name)
{
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();

	EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterName>(name);
	return entities_to_table(l,entIt);
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::find_by_component(lua_State *l,const std::string &componentName)
{
	auto *nw = engine->GetNetworkState(l);
	auto *game = nw->GetGameState();

	auto t = luabind::newtable(l);
	auto idx = 1u;
	for(auto *ent : EntityIterator{*game,componentName})
		t[idx++] = ent->GetLuaObject();
	return t;
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::find_in_aabb(lua_State *l,const Vector3 &min,const Vector3 &max)
{
	return find_in_box(l,min,max);
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::find_in_sphere(lua_State *l,const Vector3 &origin,float radius)
{
	std::vector<BaseEntity*> ents;
	NetworkState *state = engine->GetNetworkState(l);

	EntityIterator entIt {*state->GetGameState()};
	entIt.AttachFilter<EntityIteratorFilterSphere>(origin,radius);
	return entities_to_table(l,entIt);
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::find_in_box(lua_State *l,const Vector3 &min,const Vector3 &max)
{
	std::vector<BaseEntity*> ents;
	NetworkState *state = engine->GetNetworkState(l);
	Game *game = state->GetGameState();

	EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterBox>(min,max);
	return entities_to_table(l,entIt);
}

Lua::tb<Lua::type<BaseEntity>> Lua::ents::find_in_cone(lua_State *l,const Vector3 &origin,const Vector3 &dir,float radius,float angle)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();

	EntityIterator entIt {*game};
	entIt.AttachFilter<EntityIteratorFilterCone>(origin,dir,radius,angle);
	return entities_to_table(l,entIt);
}

Lua::opt<pragma::ComponentEventId> Lua::ents::get_event_id(lua_State *l,const std::string &name)
{
	pragma::ComponentEventId eventId;
	if(engine->GetNetworkState(l)->GetGameState()->GetEntityComponentManager().GetEventId(name,eventId) == false)
		return nil;
	return {l,eventId};
}

void Lua::ents::register_class(lua_State *l,const std::string &className,const Lua::classObject &classObject)
{
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &manager = game->GetLuaEntityManager();
	manager.RegisterEntity(className,const_cast<Lua::classObject&>(classObject),{});
}
void Lua::ents::register_class(lua_State *l,const std::string &className,const luabind::tableT<luabind::variant<std::string,pragma::ComponentId>> &tComponents,LuaEntityType type)
{
	std::vector<pragma::ComponentId> components;
	auto numComponents = Lua::GetObjectLength(l,tComponents);
	components.reserve(numComponents);
	auto *state = engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	for(auto i=decltype(numComponents){0};i<numComponents;++i)
	{
		auto o = tComponents[i +1];
		if(luabind::type(o) == LUA_TSTRING)
		{
			std::string name = luabind::object_cast<std::string>(o);
			auto id = pragma::INVALID_COMPONENT_ID;
			if(componentManager.GetComponentTypeId(name,id,false))
				components.push_back(id);
			else
			{
				if(game->LoadLuaComponentByName(name) == true && componentManager.GetComponentTypeId(name,id,false) == true)
					components.push_back(id);
				else
					Con::cwar<<"WARNING: Attempted to add unknown entity component '"<<name<<"' to registration of entity '"<<className<<"'! Skipping..."<<Con::endl;
			}
		}
		else
		{
			auto id = luabind::object_cast<uint32_t>(o);
			if(id != pragma::INVALID_COMPONENT_ID)
				components.push_back(id);
			else
				Con::cwar<<"WARNING: Attempted to add unknown entity component to registration of entity '"<<className<<"'! Skipping..."<<Con::endl;
		}
	}

	const char *cLuaClassName = "__TMP_ENTITY_CLASS";
	std::string luaClassName = cLuaClassName;
	std::stringstream ss;
	ss<<"util.register_class(\""<<luaClassName<<"\",BaseEntity);\n";
	ss<<"function "<<luaClassName<<":__init()\n";
	ss<<"    BaseEntity.__init(self)\n";
	ss<<"end\n";

	auto r = Lua::RunString(l,ss.str(),"internal");
	if(r == Lua::StatusCode::Ok)
	{
		auto o = luabind::object(luabind::globals(l)[cLuaClassName]);
		if(o)
		{
			o["Type"] = umath::to_integral(type);

			auto &manager = game->GetLuaEntityManager();
			manager.RegisterEntity(className,o,components);
		}
	}
	else
		Lua::HandleLuaError(l);

	Lua::PushNil(l);
	Lua::SetGlobal(l,luaClassName);
}

Lua::opt<pragma::NetEventId> Lua::ents::register_component_net_event(lua_State *l,pragma::ComponentId componentId,const std::string &name)
{
	auto *state = ::engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	auto *componentInfo = componentManager.GetComponentInfo(componentId);
	if(componentInfo == nullptr)
	{
		Con::cwar<<"WARNING: Attempted to register component net event '"<<name<<"' to unknown component type "<<componentId<<"!"<<Con::endl;
		return nil;
	}

	auto netName = componentInfo->name +'_' +std::string{name};
	return {l,game->SetupNetEvent(netName)};
}

Lua::opt<pragma::ComponentEventId> Lua::ents::register_component_event(lua_State *l,pragma::ComponentId componentId,const std::string &name)
{
	auto *state = ::engine->GetNetworkState(l);
	auto *game = state->GetGameState();
	auto &componentManager = game->GetEntityComponentManager();
	auto *componentInfo = componentManager.GetComponentInfo(componentId);
	if(componentInfo == nullptr)
	{
		Con::cwar<<"WARNING: Attempted to register component event '"<<name<<"' to unknown component type "<<componentId<<"!"<<Con::endl;
		return nil;
	}

	auto netName = componentInfo->name +'_' +std::string{name};
	auto eventId = componentManager.RegisterEventById(netName,componentId,pragma::EntityComponentManager::EventInfo::Type::Broadcast);
	return {l,eventId};
}
