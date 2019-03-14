#include "stdafx_client.h"
#include "pragma/rendering/c_prepass.hpp"
#include "pragma/rendering/shaders/world/c_shader_prepass.hpp"
#include "pragma/console/c_cvar_global_functions.h"
#include "pragma/debug/c_debug_game_gui.h"
#include "pragma/gui/widebugdepthtexture.h"
#include "pragma/gui/widebugmsaatexture.hpp"
#include "pragma/rendering/shaders/world/c_shader_textured.hpp"
#include <wgui/types/wirect.h>
#include <image/prosper_render_target.hpp>
#include <prosper_util.hpp>
#include <image/prosper_msaa_texture.hpp>
#include <image/prosper_sampler.hpp>
#include <prosper_command_buffer.hpp>

extern DLLCENGINE CEngine *c_engine;
extern DLLCLIENT CGame *c_game;

bool pragma::rendering::Prepass::Initialize(prosper::Context &context,uint32_t width,uint32_t height,Anvil::SampleCountFlagBits samples,bool bExtended)
{
	auto &dev = context.GetDevice();

	m_shaderPrepass = context.GetShader("prepass");
	m_shaderPrepassDepth = context.GetShader("prepass_depth");

	prosper::util::ImageCreateInfo imgCreateInfo {};
	imgCreateInfo.format = ShaderTextured3DBase::RENDER_PASS_DEPTH_FORMAT;
	imgCreateInfo.width = width;
	imgCreateInfo.height = height;
	imgCreateInfo.samples = samples;
	imgCreateInfo.usage = Anvil::ImageUsageFlagBits::DEPTH_STENCIL_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
	imgCreateInfo.postCreateLayout = Anvil::ImageLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
	auto imgDepth = prosper::util::create_image(dev,imgCreateInfo);

	prosper::util::TextureCreateInfo texCreateInfo {};
	texCreateInfo.flags = {};
	prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
	prosper::util::SamplerCreateInfo samplerCreateInfo {};
	samplerCreateInfo.addressModeU = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeV = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	samplerCreateInfo.addressModeW = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
	textureDepth = prosper::util::create_texture(dev,texCreateInfo,imgDepth,&imgViewCreateInfo,&samplerCreateInfo);

	if(textureDepth->IsMSAATexture())
		textureDepthSampled = static_cast<prosper::MSAATexture&>(*textureDepth).GetResolvedTexture();
	else
	{
		imgCreateInfo.usage = Anvil::ImageUsageFlagBits::TRANSFER_DST_BIT | Anvil::ImageUsageFlagBits::SAMPLED_BIT;
		imgCreateInfo.postCreateLayout = Anvil::ImageLayout::TRANSFER_DST_OPTIMAL;
		imgCreateInfo.samples = Anvil::SampleCountFlagBits::_1_BIT;
		auto imgDepthSampled = prosper::util::create_image(dev,imgCreateInfo);

		texCreateInfo.flags = {};
		textureDepthSampled = prosper::util::create_texture(dev,texCreateInfo,imgDepthSampled,&imgViewCreateInfo,&samplerCreateInfo);
	}

	SetUseExtendedPrepass(bExtended,true);
	return true;
}

pragma::ShaderPrepassBase &pragma::rendering::Prepass::GetShader() const
{
	return (m_bExtended == true) ? static_cast<pragma::ShaderPrepassBase&>(*m_shaderPrepass.get()) : static_cast<pragma::ShaderPrepassBase&>(*m_shaderPrepassDepth.get());
}

bool pragma::rendering::Prepass::IsExtended() const {return m_bExtended;}
void pragma::rendering::Prepass::SetUseExtendedPrepass(bool b,bool bForceReload)
{
	if(m_bExtended == b && bForceReload == false)
		return;
	m_bExtended = b;

	auto &context = *c_engine;
	context.WaitIdle();

	auto &imgDepth = textureDepth->GetImage();
	auto extents = imgDepth->GetExtents();
	auto width = extents.width;
	auto height = extents.height;

	auto whShaderPrepassDepth = c_engine->GetShader("prepass_depth");
	auto whShaderPrepass = c_engine->GetShader("prepass");
	if(whShaderPrepass.expired() || whShaderPrepassDepth.expired())
		return;

	auto *shaderPrepassDepth = static_cast<pragma::ShaderPrepassDepth*>(whShaderPrepassDepth.get());
	auto *shaderPrepass = static_cast<pragma::ShaderPrepass*>(whShaderPrepass.get());
	auto sampleCount = imgDepth->GetSampleCount();
	auto pipelineType = pragma::ShaderPrepassBase::GetPipelineIndex(sampleCount);
	if(b == true)
	{
		auto &dev = context.GetDevice();
		prosper::util::ImageCreateInfo imgCreateInfo {};
		imgCreateInfo.samples = imgDepth->GetSampleCount();
		imgCreateInfo.format = ShaderPrepass::RENDER_PASS_NORMAL_FORMAT;
		imgCreateInfo.width = width;
		imgCreateInfo.height = height;
		imgCreateInfo.usage = Anvil::ImageUsageFlagBits::SAMPLED_BIT | Anvil::ImageUsageFlagBits::COLOR_ATTACHMENT_BIT | Anvil::ImageUsageFlagBits::TRANSFER_SRC_BIT;
		auto imgNormals = prosper::util::create_image(dev,imgCreateInfo);

		prosper::util::TextureCreateInfo texCreateInfo {};
		texCreateInfo.flags = prosper::util::TextureCreateInfo::Flags::Resolvable;
		prosper::util::ImageViewCreateInfo imgViewCreateInfo {};
		prosper::util::SamplerCreateInfo samplerCreateInfo {};
		samplerCreateInfo.addressModeU = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeV = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
		samplerCreateInfo.addressModeW = Anvil::SamplerAddressMode::CLAMP_TO_EDGE;
		textureNormals = prosper::util::create_texture(dev,texCreateInfo,imgNormals,&imgViewCreateInfo,&samplerCreateInfo);

		auto &imgDepth = textureDepth->GetImage();
		renderTarget = prosper::util::create_render_target(dev,{textureNormals,textureDepth},shaderPrepass->GetRenderPass(umath::to_integral(pipelineType)));
		renderTarget->SetDebugName("prepass_depth_normal_rt");
		m_clearValues = {
			vk::ClearValue{vk::ClearColorValue{}}, // Unused, but required
			vk::ClearValue{vk::ClearDepthStencilValue{1.f,0}} // Clear depth
		};
	}
	else
	{
		textureNormals = nullptr;

		auto &dev = context.GetDevice();
		renderTarget = prosper::util::create_render_target(dev,{textureDepth},shaderPrepassDepth->GetRenderPass(umath::to_integral(pipelineType)));
		renderTarget->SetDebugName("prepass_depth_rt");
		m_clearValues = {
			vk::ClearValue{vk::ClearDepthStencilValue{1.f,0}} // Clear depth
		};
	}
}

void pragma::rendering::Prepass::BeginRenderPass(prosper::PrimaryCommandBuffer &cmdBuffer)
{
	// prosper TODO: Barriers for imgDepth and imgNormals
	prosper::util::record_begin_render_pass(static_cast<Anvil::PrimaryCommandBuffer&>(cmdBuffer.GetAnvilCommandBuffer()),*renderTarget,m_clearValues);
}
void pragma::rendering::Prepass::EndRenderPass(prosper::PrimaryCommandBuffer &cmdBuffer)
{
	prosper::util::record_end_render_pass(static_cast<Anvil::PrimaryCommandBuffer&>(cmdBuffer.GetAnvilCommandBuffer()));
}

void Console::commands::debug_prepass(NetworkState *state,pragma::BasePlayerComponent *pl,std::vector<std::string> &argv)
{
	auto &wgui = WGUI::GetInstance();
	auto *pRoot = wgui.GetBaseElement();
	if(c_game == nullptr || argv.empty() || pRoot == nullptr)
		return;
	const std::string name = "debug_ssao";
	auto *pEl = pRoot->FindDescendantByName(name);
	auto v = util::to_int(argv.front());
	if(v == 0)
	{
		if(pEl != nullptr)
			pEl->Remove();
		return;
	}
	if(pEl != nullptr)
		return;
	pEl = wgui.Create<WIBase>();
	if(pEl == nullptr)
		return;
	pEl->SetName(name);

	auto &scene = c_game->GetScene();
	auto &ssaoInfo = scene->GetSSAOInfo();
	auto &prepass = scene->GetPrepass();

	auto bExtended = prepass.IsExtended();
	auto xOffset = 0u;
	if(prepass.textureNormals != nullptr)
	{
		auto *pNormals = wgui.Create<WITexturedRect>(pEl);
		if(pNormals != nullptr)
		{
			pNormals->SetX(xOffset);
			pNormals->SetSize(256,256);
			pNormals->SetTexture(*prepass.textureNormals);
			pNormals->Update();
			xOffset += 256;
		}
	}
	auto *pPrepassDepth = wgui.Create<WIDebugDepthTexture>(pEl);
	if(pPrepassDepth != nullptr)
	{
		pPrepassDepth->SetX(xOffset);
		pPrepassDepth->SetSize(256,256);
		pPrepassDepth->SetTexture(*prepass.textureDepth);
		pPrepassDepth->Update();
		xOffset += 256;
	}

	pEl->SizeToContents();
}
