#pragma once

#include "renderer/ShaderProgram.h"
#include "Window.h"
#include "Particle.h"
#include "../gui/ParticleCustomizer.h"

class ParticleSystem 
{
public:
	ParticleSystem();
	~ParticleSystem();

	void Update(const float& deltaTime);
	void Draw();
	void SpawnAllParticlesOnQue(const float& deltaTime);
	void SpawnParticle(const Particle& particle);
	void ClearParticles();
	void DisplayGUI();

	ParticleSystem(const ParticleSystem& Psystem);
	ParticleSystem operator=(const ParticleSystem& Psystem);

public:
	void* m_ParticleInfoBuffer;
	float m_TimeTillNextParticleSpawn = 0.0f;
	ParticleCustomizer m_Customizer;
	std::string m_Name;
	bool m_EditorOpen;
	bool m_RenameTextOpen;
	int m_ID;
	unsigned int m_ActiveParticleCount = 0;

	//only for spawning on mouse press
	bool m_ShouldSpawnParticles;
private:
	ShaderProgram m_Shader;
	std::vector<Particle> m_Particles;
	unsigned int m_ParticleCount;

};