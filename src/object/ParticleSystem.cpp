#include <pch.h>
#include "ParticleSystem.h"

namespace ALZ {
	static bool InitilizedCircleVertices = false;
	static VertexArray* VAO = nullptr;
	static VertexBuffer* VBO = nullptr;
	static ShaderProgram* CircleInstancedShader = nullptr;

	static Texture* DefaultTexture;

	ParticleSystem::ParticleSystem()
	{
		Type = InspectorObjectType::ParticleSystemType;

		m_Name = "Particle System";

		m_Noise.Init();

		//TODO make this customizable or something, for now it's a default value of 1000
		m_ParticleCount = 1000;

		m_ParticleDrawTransformationBuffer.resize(m_ParticleCount);
		m_ParticleDrawColorBuffer.resize(m_ParticleCount);
		m_Particles.resize(m_ParticleCount);


		//initilize the vertices only the first time a particle system is created
		if (!InitilizedCircleVertices) {

			VAO = Renderer::CreateVertexArray().release();
			VAO->Bind();

			//						 Position				  Texture Coordinates
			glm::vec2 vertices[] = { glm::vec2(-1.0f, -1.0f), glm::vec2(0.0, 0.0),
									 glm::vec2(-1.0f,  1.0f), glm::vec2(0.0, 1.0),
									 glm::vec2( 1.0f,  1.0f), glm::vec2(1.0, 1.0),

									 glm::vec2( 1.0f,  1.0f), glm::vec2(1.0, 1.0),
									 glm::vec2( 1.0f, -1.0f), glm::vec2(1.0, 0.0),
									 glm::vec2(-1.0f, -1.0f), glm::vec2(0.0, 0.0) };


			VBO = Renderer::CreateVertexBuffer(vertices, sizeof(vertices)).release();

			//				 Position					Texture Coordinates
			VBO->SetLayout({ ShaderVariableType::Vec2, ShaderVariableType::Vec2 });

			VBO->Unbind();
			VAO->Unbind();

			DefaultTexture = Renderer::CreateTexture().release();
			DefaultTexture->SetImage(Image::LoadFromFile("res/Circle.png"));
			DefaultTexture->Bind();

			CircleInstancedShader = Renderer::CreateShaderProgram("shaders/CircleInstanced.vert", "shaders/CircleInstanced.frag").release();

			InitilizedCircleVertices = true;
		}
	}

	void ParticleSystem::Update(const float& deltaTime)
	{
		SpawnAllParticlesOnQue(deltaTime);

		ActiveParticleCount = 0;
		for (Particle& particle : m_Particles) {

			//add noise if it is enabled
			if (Customizer.m_NoiseCustomizer.m_NoiseEnabled && particle.isActive) {
				particle.m_Velocity.x += m_Noise.Noise(particle.m_Position.x, particle.m_Position.y) * Customizer.m_NoiseCustomizer.m_NoiseStrength;
				particle.m_Velocity.y += m_Noise.Noise(particle.m_Position.x + 30, particle.m_Position.y - 30) * Customizer.m_NoiseCustomizer.m_NoiseStrength;
			}

			if (particle.isActive) {

				//add forces to the particle
				for (auto& force : Customizer.m_ForceCustomizer.m_Forces)
				{
					particle.m_Acceleration += force.second * deltaTime;
				}

				//update particle speed, lifetime etc
				particle.Update(deltaTime);

				//limit particle velocity
				if (Customizer.m_VelocityCustomizer.CurrentVelocityLimitType != VelocityCustomizer::NoLimit)
				{
					//to make the code look cleaner
					VelocityCustomizer& velocityCustomizer = Customizer.m_VelocityCustomizer;

					//use normal velocity limit
					//by calculating the velocity in both x and y and limiting the length of the vector
					if (velocityCustomizer.CurrentVelocityLimitType == VelocityCustomizer::NormalLimit)
					{
						float length = glm::length(particle.m_Velocity);
						if (length > velocityCustomizer.m_MaxNormalVelocityLimit ||
							length < velocityCustomizer.m_MinNormalVelocityLimit)
						{
							glm::vec2 direction = glm::normalize(particle.m_Velocity);
							length = std::clamp(length, velocityCustomizer.m_MinNormalVelocityLimit, velocityCustomizer.m_MaxNormalVelocityLimit);
							particle.m_Velocity = length * direction;
						}
					}
					//limit velocity in each axis
					else if (Customizer.m_VelocityCustomizer.CurrentVelocityLimitType == VelocityCustomizer::PerAxisLimit)
					{
						particle.m_Velocity.x = std::clamp(particle.m_Velocity.x, velocityCustomizer.m_MinPerAxisVelocityLimit.x, velocityCustomizer.m_MaxPerAxisVelocityLimit.x);
						particle.m_Velocity.y = std::clamp(particle.m_Velocity.y, velocityCustomizer.m_MinPerAxisVelocityLimit.y, velocityCustomizer.m_MaxPerAxisVelocityLimit.y);
					}
				}

				//update active particle count
				ActiveParticleCount++;
			}
		}
	}

	void ParticleSystem::Draw()
	{
		//bind vertex array and shader
		VAO->Bind();
		CircleInstancedShader->Bind();

		//set texture uniform (Sampler2D) to 0
		CircleInstancedShader->SetUniform1i("particleTexture", 0);

		//if we are using the default texture
		if (Customizer.m_TextureCustomizer.UseDefaultTexture) {
			//bind the default texture to slot 0
			DefaultTexture->Bind(0);
			DefaultTexture->SetDefaultTextureSettings();
		}
		else
			//if we are using a custom texture, bind it to slot 0
			if (Customizer.m_TextureCustomizer.ParticleTexture) {
				Customizer.m_TextureCustomizer.ParticleTexture->Bind(0);
				Customizer.m_TextureCustomizer.ParticleTexture->SetDefaultTextureSettings();
			}

		//reset the amount of particles to be drawn every frame
		m_ParticleDrawCount = 0;

		//go through all the particles
		for (unsigned int i = 0; i < m_ParticleCount; i++)
		{
			if (m_Particles[i].isActive) {

				//get a value from 0 to 1, showing how much the particle lived.
				//1 meaning it's lifetime is over and it is going to die (get deactivated and not rendered).
				//0 meaning it's just been spawned (activated).
				float t = (m_Particles[i].m_LifeTime - m_Particles[i].m_RemainingLifeTime) / m_Particles[i].m_LifeTime;

				//create a new model matrix for each particle
				glm::mat4 model = glm::mat4(1.0f);

				//move particle to it's position
				model = glm::translate(model, glm::vec3(m_Particles[i].m_Position.x, m_Particles[i].m_Position.y, 0.0f));
				//use the t value to get the scale of the particle using it's not using a Custom Curve
				float scale;
				if (m_Particles[i].m_ScaleInterpolator.Type == Custom)
					scale = m_Particles[i].CustomScaleCurve.Interpolate(m_Particles[i].m_ScaleInterpolator.startPoint, m_Particles[i].m_ScaleInterpolator.endPoint, t);
				else
					scale = m_Particles[i].m_ScaleInterpolator.Interpolate(t);

				//scale the particle by that value
				model = glm::scale(model, glm::vec3(scale, scale, scale));

				//put the drawing properties of the particles in the draw buffers that would be drawn this frame
				m_ParticleDrawTransformationBuffer[m_ParticleDrawCount] = model;
				m_ParticleDrawColorBuffer[m_ParticleDrawCount] = m_Particles[i].m_ColorInterpolator.Interpolate(t);

				//up the amount of particles to be drawn this frame
				m_ParticleDrawCount++;
			}
		}

		//how many times we need to drae (currently we draw 40 particles at a time)
		int drawCount = (int)m_ParticleDrawCount / 40;

		//draw 40 particles
		for (int i = 0; i < drawCount; i++)
		{
			CircleInstancedShader->SetUniformMat4s("model", &m_ParticleDrawTransformationBuffer[i * 40], 40);
			CircleInstancedShader->SetUniformVec4s("colorArr", &m_ParticleDrawColorBuffer[i * 40], 40);
			Renderer::DrawInstanced(*VAO, *CircleInstancedShader, Primitive::TriangleFan, 26, 40);
		}

		//get the remaining particles (because we draw 40 at a time and not every number is divisble by 40)
		int remaining = m_ParticleDrawCount % 40;

		//return early if non are remaining
		if (remaining == 0)
			return;

		//draw them
		CircleInstancedShader->SetUniformMat4s("model", &m_ParticleDrawTransformationBuffer[drawCount * 40], remaining);
		CircleInstancedShader->SetUniformVec4s("colorArr", &m_ParticleDrawColorBuffer[drawCount * 40], remaining);
		Renderer::DrawInstanced(*VAO, *CircleInstancedShader, Primitive::TriangleFan, 26, remaining);
	}

	void ParticleSystem::SpawnParticle(const Particle& particle)
	{
		//go through all the particles
		for (unsigned int i = 0; i < m_ParticleCount; i++)
		{
			//find a particle that is not active
			if (!m_Particles[i].isActive)
			{
				//assign particle variables from the passed particle
				m_Particles[i].m_Position = particle.m_Position;
				m_Particles[i].m_ColorInterpolator = particle.m_ColorInterpolator;
				m_Particles[i].m_Velocity = particle.m_Velocity;
				m_Particles[i].isActive = true;
				m_Particles[i].m_ScaleInterpolator = particle.m_ScaleInterpolator;
				m_Particles[i].CustomScaleCurve = particle.CustomScaleCurve;
				m_Particles[i].SetLifeTime(particle.m_LifeTime);
				m_Particles[i].m_Acceleration = glm::vec2(0.0f);

				//break out of the for loop because we are spawning one particle only
				break;
			}
		}

		//if no inactive particle is found, don't do anything (do not spawn a new particle)
	}

	void ParticleSystem::ClearParticles()
	{
		//deactivate all particles which will make them stop rendering
		for (Particle& m_particle : m_Particles)
			m_particle.isActive = false;
	}

	glm::vec2& ParticleSystem::GetPositionRef()
	{
		switch (Customizer.Mode)
		{
		case SpawnMode::SpawnOnPoint:
			return Customizer.m_SpawnPosition;

		case SpawnMode::SpawnOnLine:
			return Customizer.m_LinePosition;

		case SpawnMode::SpawnOnCircle:
		case SpawnMode::SpawnInsideCircle:
			return Customizer.m_CircleOutline.Position;

		}

		assert(false);
		return glm::vec2(0.0f);
	}

	ParticleSystem::ParticleSystem(const ParticleSystem& Psystem) :
		Customizer(Psystem.Customizer)
	{

		//these are calculated every frame, so there is no need to copy them. a resize should be enough
		m_ParticleDrawTransformationBuffer.resize(Psystem.m_ParticleDrawTransformationBuffer.size());
		m_ParticleDrawColorBuffer.resize(Psystem.m_ParticleDrawColorBuffer.size());

		//copy other variables
		m_Particles = Psystem.m_Particles;
		m_ParticleCount = Psystem.m_ParticleCount;
		m_Name = Psystem.m_Name;
		EditorOpen = Psystem.EditorOpen;
		RenameTextOpen = Psystem.RenameTextOpen;

		//initilize the noise class
		m_Noise.Init();
	}

	ParticleSystem ParticleSystem::operator=(const ParticleSystem & Psystem)
	{
		//forward the call to the copy constructor
		return ParticleSystem(Psystem);
	}

	void ParticleSystem::DisplayGUI()
	{
		ImGui::PushID(this);
		if (EditorOpen)
			Customizer.DisplayGUI(m_Name, EditorOpen);
		ImGui::PopID();

		//update editor line
		if (Customizer.Mode == SpawnMode::SpawnOnLine)
		{
			Customizer.m_Line.SetPoints(Customizer.m_LinePosition, Customizer.m_LineLength,Customizer.m_LineAngle);
		}
	}

	void ParticleSystem::SpawnAllParticlesOnQue(const float& deltaTime)
	{
		TimeTillNextParticleSpawn -= deltaTime;
		if (TimeTillNextParticleSpawn < 0.0f) {
			TimeTillNextParticleSpawn = abs(TimeTillNextParticleSpawn);

			while (TimeTillNextParticleSpawn > 0.0f) {
				Particle p = Customizer.GetParticle();
				SpawnParticle(p);
				TimeTillNextParticleSpawn -= Customizer.GetTimeBetweenParticles();
			}

			TimeTillNextParticleSpawn = Customizer.GetTimeBetweenParticles();
		}
	}
}