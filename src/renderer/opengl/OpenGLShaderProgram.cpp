#include <glad/glad.h>

#include "OpenGLShaderProgram.h"
#include "file/AssetManager.h" //for reading shader files
#include "OpenGLUniformBuffer.h"
#include "OpenGLTexture.h"
#include "OpenGLFrameBuffer.h"

namespace Ainan {
	namespace OpenGL {

		std::string LoadAndParseShader(std::string path)
		{
			std::string shader = AssetManager::ReadEntireTextFile(path);

			//parse include statements
			size_t includeLocation = 0;
			includeLocation = shader.find("#include ");
			while (includeLocation != std::string::npos)
			{
				std::filesystem::path shaderFolder = path;
				shaderFolder = shaderFolder.parent_path();

				std::string includeStatement = shader.substr(includeLocation, shader.find('\n', includeLocation) - includeLocation);
				size_t subPathBeginLocation = includeStatement.find('<');
				size_t subPathEndLocation = includeStatement.find('>');
				std::string includePath = includeStatement.substr(subPathBeginLocation + 1, subPathEndLocation - subPathBeginLocation - 1);

				std::filesystem::path fullPath = shaderFolder.string() + "/" + includePath;
				if (std::filesystem::exists(fullPath))
				{
					std::string includeFileContents = AssetManager::ReadEntireTextFile(fullPath.string());

					shader.erase(includeLocation, includeStatement.size());
					shader.insert(includeLocation, includeFileContents.c_str());
				}
				else
					assert(false);
				includeLocation = shader.find("#include ");
			}

			return shader;
		}

		OpenGLShaderProgram::OpenGLShaderProgram(const std::string& vertPath, const std::string& fragPath)
		{
			uint32_t vertex, fragment;

			vertex = glCreateShader(GL_VERTEX_SHADER);
			std::string vShaderCode = LoadAndParseShader(vertPath + ".vert");
			const char* c_vShaderCode = vShaderCode.c_str();
			glShaderSource(vertex, 1, &c_vShaderCode, NULL);
			glCompileShader(vertex);

			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			std::string fShaderCode = LoadAndParseShader(fragPath + ".frag");
			const char* c_fShaderCode = fShaderCode.c_str();
			glShaderSource(fragment, 1, &c_fShaderCode, NULL);
			glCompileShader(fragment);

			// shader Program
			m_RendererID = glCreateProgram();
			glAttachShader(m_RendererID, vertex);
			glAttachShader(m_RendererID, fragment);

			glLinkProgram(m_RendererID);
			// delete the shaders as they're linked into our program now and no longer necessery
			glDeleteShader(vertex);
			glDeleteShader(fragment);
		}

		std::shared_ptr<OpenGLShaderProgram> OpenGLShaderProgram::CreateRaw(const std::string& vertSrc, const std::string& fragSrc)
		{
			auto shader = std::make_shared<OpenGLShaderProgram>();

			uint32_t vertex, fragment;

			vertex = glCreateShader(GL_VERTEX_SHADER);
			const char* c_vShaderCode = vertSrc.c_str();
			glShaderSource(vertex, 1, &c_vShaderCode, NULL);
			glCompileShader(vertex);


			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			const char* c_fShaderCode = fragSrc.c_str();
			glShaderSource(fragment, 1, &c_fShaderCode, NULL);
			glCompileShader(fragment);


			// shader Program
			shader->m_RendererID = glCreateProgram();
			glAttachShader(shader->m_RendererID, vertex);
			glAttachShader(shader->m_RendererID, fragment);

			glLinkProgram(shader->m_RendererID);
			// delete the shaders as they're linked into our program now and no longer necessery
			glDeleteShader(vertex);
			glDeleteShader(fragment);

			return shader;
		}

		OpenGLShaderProgram::~OpenGLShaderProgram()
		{
			glDeleteProgram(m_RendererID);
		}

		void OpenGLShaderProgram::BindUniformBuffer(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage)
		{
			auto func = [this, slot, stage, &buffer]()
			{
				BindUniformBufferUnsafe(buffer, slot, stage);
			};
			Renderer::PushCommand(func);
		}

		void OpenGLShaderProgram::BindUniformBufferUnsafe(std::shared_ptr<UniformBuffer>& buffer, uint32_t slot, RenderingStage stage)
		{
			std::shared_ptr<OpenGLUniformBuffer> openglBuffer = std::static_pointer_cast<OpenGLUniformBuffer>(buffer);
			glBindBufferRange(GL_UNIFORM_BUFFER, slot, openglBuffer->m_RendererID, 0, buffer->GetAlignedSize());
		}

		void OpenGLShaderProgram::BindTexture(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage)
		{
			auto func = [this, slot, stage, &texture]()
			{
				BindTextureUnsafe(texture, slot, stage);
			};
			Renderer::PushCommand(func);
		}

		void OpenGLShaderProgram::BindTextureUnsafe(std::shared_ptr<Texture>& texture, uint32_t slot, RenderingStage stage)
		{
			std::shared_ptr<OpenGLTexture> openglTexture = std::static_pointer_cast<OpenGLTexture>(texture);
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, openglTexture->m_RendererID);
		}

		void OpenGLShaderProgram::BindTexture(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage)
		{
			auto func = [this, slot, stage, &framebuffer]()
			{
				BindTextureUnsafe(framebuffer, slot, stage);
			};
			Renderer::PushCommand(func);
		}

		void OpenGLShaderProgram::BindTextureUnsafe(std::shared_ptr<FrameBuffer>& framebuffer, uint32_t slot, RenderingStage stage)
		{
			std::shared_ptr<OpenGLFrameBuffer> openglTexture = std::static_pointer_cast<OpenGLFrameBuffer>(framebuffer);
			glActiveTexture(GL_TEXTURE0 + slot);
			glBindTexture(GL_TEXTURE_2D, openglTexture->m_TextureID);
		}
	}
}