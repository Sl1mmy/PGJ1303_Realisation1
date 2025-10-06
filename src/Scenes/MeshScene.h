#pragma once

#include "Scene.h"
#include "OpenGlDef.h"
#include "OpenGlResource.h"
#include "OpenGlProgram.h"
#include <glm/mat4x4.hpp>

class CMeshScene : public CScene
{
public:
	CMeshScene();

	void Update(double dt) override;
	void Draw() override;

private:
	enum LIGHT_TYPE
	{
		DIRECTIONAL = 0,
		POINT = 1,
		SPOT = 2,
	}; 
	
	struct Matrices
	{
		glm::mat4 worldViewProjMatrix;
	};

	struct Light
	{
		glm::vec4 ambientColor;
		glm::vec4 diffuseColor;
		glm::vec4 specularColor;
		glm::vec4 dir;
		glm::vec4 pos;
		float linAttenuation;
		float quadAttenuation;
		int type;
		int padding;
	};

	struct Lights
	{
		glm::vec4 viewDir;
		alignas(16) Light lights[2];
	};

	OpenGl::CBuffer m_vertexBuffer;
	OpenGl::CBuffer m_indexBuffer;
	OpenGl::CBuffer m_uniformBuffer;
	OpenGl::CBuffer m_lightsUniformBuffer;
	OpenGl::CVertexArray m_vertexArray;
	OpenGl::CProgram m_program;

	GLuint m_matricesUniformBinding = -1;

	Matrices m_matrices;
	Lights m_lights = {};
	GLuint m_lightsUniformBinding;
	uint32_t m_numIndices = 0;
};
