#pragma once
#ifdef PLATFORM_WINDOWS

#include "renderer/RendererAPI.h"
#include "renderer/VertexBuffer.h"
#include <d3d11.h>

#include "D3D11RendererContext.h"

namespace Ainan {
	namespace D3D11 {

		class D3D11VertexBuffer : public VertexBuffer
		{
		public:
			D3D11VertexBuffer(void* data, uint32_t size,
				const VertexLayout& layout, const std::shared_ptr<ShaderProgram>& shaderProgram,
				bool dynamic, RendererContext* context);
			virtual ~D3D11VertexBuffer();

			virtual void UpdateData(int32_t offset, int32_t size, void* data) override;
			virtual void UpdateDataUnsafe(int32_t offset, int32_t size, void* data) override;
			virtual uint32_t GetUsedMemory() const override { return Memory; };
			virtual void Bind() const override;
			virtual void Unbind() const override;

		public:
			D3D11RendererContext* Context;
			ID3D11Buffer* Buffer;
			ID3D11InputLayout* Layout;
			uint32_t Stride;
			uint32_t Memory;
		};
	}
}

#endif