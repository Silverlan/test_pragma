/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/rendering/shaders/c_shader_shadow.hpp"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include "pragma/entities/components/c_render_component.hpp"
#include "pragma/model/c_vertex_buffer_data.hpp"
#include "pragma/model/c_modelmesh.h"
#include <shader/prosper_pipeline_create_info.hpp>
#include <prosper_util.hpp>
#include <pragma/model/vertex.h>

using namespace pragma;

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

static auto SHADOW_DEPTH_BIAS_CONSTANT = 1.25f;
static auto SHADOW_DEPTH_BIAS_SLOPE = 1.75f;

decltype(ShaderShadow::RENDER_PASS_DEPTH_FORMAT) ShaderShadow::RENDER_PASS_DEPTH_FORMAT = prosper::Format::D32_SFloat;

decltype(ShaderShadow::VERTEX_BINDING_RENDER_BUFFER_INDEX) ShaderShadow::VERTEX_BINDING_RENDER_BUFFER_INDEX = {prosper::VertexInputRate::Instance};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX) ShaderShadow::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX = {ShaderGameWorld::VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX,VERTEX_BINDING_RENDER_BUFFER_INDEX};

decltype(ShaderShadow::VERTEX_BINDING_BONE_WEIGHT) ShaderShadow::VERTEX_BINDING_BONE_WEIGHT = {prosper::VertexInputRate::Vertex};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID) ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID = {ShaderGameWorld::VERTEX_ATTRIBUTE_BONE_WEIGHT_ID,VERTEX_BINDING_BONE_WEIGHT};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT) ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT = {ShaderGameWorld::VERTEX_ATTRIBUTE_BONE_WEIGHT,VERTEX_BINDING_BONE_WEIGHT};

decltype(ShaderShadow::VERTEX_BINDING_BONE_WEIGHT_EXT) ShaderShadow::VERTEX_BINDING_BONE_WEIGHT_EXT = {prosper::VertexInputRate::Vertex};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID) ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID = {ShaderGameWorld::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID,VERTEX_BINDING_BONE_WEIGHT_EXT};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT) ShaderShadow::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT = {ShaderGameWorld::VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT,VERTEX_BINDING_BONE_WEIGHT_EXT};

decltype(ShaderShadow::VERTEX_BINDING_VERTEX) ShaderShadow::VERTEX_BINDING_VERTEX = {prosper::VertexInputRate::Vertex,sizeof(VertexBufferData)};
decltype(ShaderShadow::VERTEX_ATTRIBUTE_POSITION) ShaderShadow::VERTEX_ATTRIBUTE_POSITION = {ShaderGameWorld::VERTEX_ATTRIBUTE_POSITION,VERTEX_BINDING_VERTEX};

decltype(ShaderShadow::DESCRIPTOR_SET_INSTANCE) ShaderShadow::DESCRIPTOR_SET_INSTANCE = {&ShaderGameWorld::DESCRIPTOR_SET_INSTANCE};
decltype(ShaderShadow::DESCRIPTOR_SET_SCENE) ShaderShadow::DESCRIPTOR_SET_SCENE = {&ShaderScene::DESCRIPTOR_SET_SCENE};
decltype(ShaderShadow::DESCRIPTOR_SET_MATERIAL) ShaderShadow::DESCRIPTOR_SET_MATERIAL = {&ShaderGameWorldLightingPass::DESCRIPTOR_SET_MATERIAL};
decltype(ShaderShadow::DESCRIPTOR_SET_RENDER_SETTINGS) ShaderShadow::DESCRIPTOR_SET_RENDER_SETTINGS = {&ShaderGameWorld::DESCRIPTOR_SET_RENDER_SETTINGS};
ShaderShadow::ShaderShadow(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader)
	: ShaderGameWorld(context,identifier,vsShader,fsShader)
{}

ShaderShadow::ShaderShadow(prosper::IPrContext &context,const std::string &identifier)
	: ShaderShadow(context,identifier,"shadow/vs_shadow","shadow/fs_shadow")
{}
void ShaderShadow::OnPipelinesInitialized()
{
	ShaderGameWorld::OnPipelinesInitialized();
	m_defaultMatDsg = c_engine->GetRenderContext().CreateDescriptorSetGroup(DESCRIPTOR_SET_MATERIAL);
}
bool ShaderShadow::BeginDraw(
	const std::shared_ptr<prosper::ICommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin,
	RecordFlags recordFlags
)
{
	return ShaderScene::BeginDraw(cmdBuffer,0u,recordFlags);
}
bool ShaderShadow::BindEntityDepthMatrix(const Mat4 &depthMVP)
{
	return RecordPushConstants(depthMVP);
}
bool ShaderShadow::BindMaterial(CMaterial &mat)
{
	/*auto &descTexture = mat.GetDescriptorSetGroup(*this);
	if(descTexture == nullptr)
		return false;
	auto *data = mat.GetDataBlock();
	auto alphaDiscardThreshold = pragma::DefaultAlphaDiscardThreshold;
	if(data != nullptr)
		data->GetFloat("alpha_discard_threshold",&alphaDiscardThreshold);
	cmdBuffer->PushConstants(layout,prosper::ShaderStageFlags::FragmentBit,20,1,&alphaDiscardThreshold);
	cmdBuffer->BindDescriptorSet(umath::to_integral(DescSet::DiffuseMap),layout,descTexture);
	return true;*/
	return true; // TODO
}

bool ShaderShadow::Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx,prosper::IBuffer &renderBufferIndexBuffer,uint32_t instanceCount)
{
	return true;
	/*auto flags = Flags::None;
	if(mesh.GetExtendedVertexWeights().empty() == false)
		flags |= Flags::UseExtendedVertexWeights;
	return RecordPushConstants(flags,offsetof(PushConstants,flags)) && ShaderGameWorld::Draw(mesh,meshIdx,renderBufferIndexBuffer,instanceCount);*/
}

bool ShaderShadow::BindLight(CLightComponent &light,uint32_t layerId)
{
	auto &ent = light.GetEntity();
	auto pTrComponent = ent.GetTransformComponent();
	auto pRadiusComponent = ent.GetComponent<CRadiusComponent>();
	auto pos = pTrComponent != nullptr ? pTrComponent->GetPosition() : Vector3{};
	auto lightPos = Vector4{pos.x,pos.y,pos.z,static_cast<float>(pRadiusComponent.valid() ? pRadiusComponent->GetRadius() : 0.f)};
	auto &depthMVP = light.GetTransformationMatrix(layerId);
	return RecordPushConstants(lightPos,offsetof(PushConstants,lightPos)) && RecordPushConstants(depthMVP);
}
void ShaderShadow::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderShadow>({{{
		{
			RENDER_PASS_DEPTH_FORMAT,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::AttachmentLoadOp::Clear,
			prosper::AttachmentStoreOp::Store,prosper::SampleCountFlags::e1Bit,prosper::ImageLayout::ShaderReadOnlyOptimal
		}
	}}},outRenderPass,pipelineIdx);
}
uint32_t ShaderShadow::GetRenderSettingsDescriptorSetIndex() const {return DESCRIPTOR_SET_RENDER_SETTINGS.setIndex;}
uint32_t ShaderShadow::GetCameraDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
uint32_t ShaderShadow::GetLightDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
uint32_t ShaderShadow::GetInstanceDescriptorSetIndex() const{return DESCRIPTOR_SET_INSTANCE.setIndex;}
uint32_t ShaderShadow::GetMaterialDescriptorSetIndex() const {return DESCRIPTOR_SET_MATERIAL.setIndex;}
bool ShaderShadow::BindScene(pragma::CSceneComponent &scene,rendering::RasterizationRenderer &renderer,bool bView) {return BindRenderSettings(c_game->GetGlobalRenderSettingsDescriptorSet());}
void ShaderShadow::GetVertexAnimationPushConstantInfo(uint32_t &offset) const {}
void ShaderShadow::InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx)
{
	prosper::ShaderGraphics::InitializeGfxPipeline(pipelineInfo,pipelineIdx);

	ToggleDynamicScissorState(pipelineInfo,true);
	pipelineInfo.ToggleDepthWrites(true);
	pipelineInfo.ToggleDepthTest(true,prosper::CompareOp::LessOrEqual);
	prosper::util::set_graphics_pipeline_cull_mode_flags(pipelineInfo,prosper::CullModeFlags::None);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_RENDER_BUFFER_INDEX);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_ID);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID);
	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT);

	AddVertexAttribute(pipelineInfo,VERTEX_ATTRIBUTE_POSITION);

	AttachPushConstantRange(pipelineInfo,0u,sizeof(PushConstants),prosper::ShaderStageFlags::VertexBit | prosper::ShaderStageFlags::FragmentBit);

	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_INSTANCE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_MATERIAL);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_SCENE);
	AddDescriptorSetGroup(pipelineInfo,DESCRIPTOR_SET_RENDER_SETTINGS);

	pipelineInfo.ToggleDepthBias(true,SHADOW_DEPTH_BIAS_CONSTANT,0.f,SHADOW_DEPTH_BIAS_SLOPE);
}

//////////////////

ShaderShadowSpot::ShaderShadowSpot(prosper::IPrContext &context,const std::string &identifier)
	: ShaderShadow(context,identifier,"shadow/vs_shadow","shadow/fs_shadow_spot")
{
	SetBaseShader<ShaderShadow>();
}

//////////////////

ShaderShadowCSM::ShaderShadowCSM(prosper::IPrContext &context,const std::string &identifier)
	: ShaderShadow(context,identifier,"shadow/vs_shadow_csm","")
{
	SetBaseShader<ShaderShadow>();
}
void ShaderShadowCSM::InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx)
{
	CreateCachedRenderPass<ShaderShadowCSM>({{{
		{
			RENDER_PASS_DEPTH_FORMAT,prosper::ImageLayout::DepthStencilAttachmentOptimal,prosper::AttachmentLoadOp::Load,
			prosper::AttachmentStoreOp::Store,prosper::SampleCountFlags::e1Bit,prosper::ImageLayout::ShaderReadOnlyOptimal
		}
	}}},outRenderPass,pipelineIdx);
}
