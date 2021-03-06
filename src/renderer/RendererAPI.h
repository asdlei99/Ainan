#pragma once

#include "RendererContext.h"

namespace Ainan {

	class ShaderProgram;
	class IndexBuffer;
	class Texture;
	struct Rectangle;

	enum class Primitive
	{
		Triangles,
		TriangleFan,
		Lines
	};

	enum class RenderingBlendMode
	{
		Additive,
		Screen,
		Overlay,
		NotSpecified //this will use the current mode it is set on
	};

	enum class RenderingStage
	{
		VertexShader,
		FragmentShader
	};

	//pure virtual class (interface) for each renderer api to inherit from
	class RendererAPI
	{
	public:
		virtual ~RendererAPI() {};
		virtual void Draw(ShaderProgram& shader, Primitive mode, uint32_t vertexCount) = 0;
		virtual void Draw(ShaderProgram& shader, Primitive mode, const IndexBuffer& indexBuffer) = 0;
		virtual void Draw(ShaderProgram& shader, Primitive mode, const IndexBuffer& indexBuffer, uint32_t vertexCount) = 0;

		virtual void InitImGui() = 0;
		virtual void ImGuiNewFrame() = 0;
		virtual void ImGuiEndFrame() = 0;
		virtual void DrawImGui(ImDrawData* drawData) = 0;

		virtual void ClearScreen() = 0;

		virtual void SetRenderTargetApplicationWindow() = 0;

		virtual void Present() = 0;

		virtual void RecreateSwapchain(const glm::vec2& newSwapchainSize) = 0;

		virtual void SetBlendMode(RenderingBlendMode blendMode) = 0;

		virtual void SetViewport(const Rectangle& viewport) = 0;

		virtual RendererContext* GetContext() = 0;
	};
}