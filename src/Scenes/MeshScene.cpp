#include "MeshScene.h"
#include "OpenGlShader.h"
#include <glm/vec3.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

namespace
{
	struct VERTEX
	{
		glm::vec3 position;
		glm::vec3 normal;
		glm::vec2 texcoord; // UV
		                    //glm::vec3 tangent; // for normal map
		                    //glm::vec3 bitangent; // for normal map
	};

	enum VERTEX_ATTRIBUTES
	{
		POSITION,
		NORMAL,
		//TEXCOORD,
		//TANGENT,
		//BITANGENT,
	};
}

CMeshScene::CMeshScene()
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile("./resources/models/teapot.dae",
	                                         aiProcess_Triangulate |
	                                             aiProcess_GenSmoothNormals |
	                                             aiProcess_FlipUVs |
	                                             aiProcess_CalcTangentSpace);

	assert(scene->HasMeshes());
	auto mesh = scene->mMeshes[0];
	assert(mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE);

	std::vector<VERTEX> vertices;
	vertices.reserve(mesh->mNumVertices);
	for(int i = 0; i < mesh->mNumVertices; i++)
	{
		auto position = mesh->mVertices[i];
		auto normal = mesh->mNormals[i];
		auto texcoord = mesh->mTextureCoords[0] ? mesh->mTextureCoords[0][i] : aiVector3D(0, 0, 0);
		auto tangent = mesh->mTangents ? mesh->mTangents[i] : aiVector3D(0, 0, 0);
		auto bitangent = mesh->mBitangents ? mesh->mBitangents[i] : aiVector3D(0, 0, 0);

		vertices.push_back({
		    {position.x, position.y, position.z},
		    {normal.x, normal.y, normal.z},
		    //{texcoord.x, texcoord.y},
		    //{tangent.x, tangent.y, tangent.z},
		    //{bitangent.x, bitangent.y, bitangent.z}

		});
	}

	std::vector<uint32_t> indices;
	indices.reserve(mesh->mNumFaces * 3);
	for(int i = 0; i < mesh->mNumFaces; i++)
	{
		auto face = mesh->mFaces[i];
		assert(face.mNumIndices == 3);
		for(int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	m_numIndices = indices.size();

	{
		m_vertexBuffer = OpenGl::CBuffer::Create();
		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX) * vertices.size(), vertices.data(), GL_STATIC_DRAW);
	}

	{
		m_indexBuffer = OpenGl::CBuffer::Create();
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);
	}

	{
		m_uniformBuffer = OpenGl::CBuffer::Create();
		glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(m_matrices), &m_matrices, GL_DYNAMIC_DRAW);
	}

	m_lightsUniformBuffer = OpenGl::CBuffer::Create();

	{
		auto vertShader = OpenGl::CShader::CreateFromFile(GL_VERTEX_SHADER, "./shaders/proj_v.glsl");
		auto fragShader = OpenGl::CShader::CreateFromFile(GL_FRAGMENT_SHADER, "./shaders/proj_f.glsl");

		vertShader.Compile();
		fragShader.Compile();

		m_program = OpenGl::CProgram::Create();
		m_program.AttachShader(vertShader);
		m_program.AttachShader(fragShader);
		m_program.Link();

		GLint numBlocks = 0;
		glGetProgramiv(m_program, GL_ACTIVE_UNIFORM_BLOCKS, &numBlocks);
		printf("Active uniform blocks: %d\n", numBlocks);
		for(GLuint i = 0; i < (GLuint)numBlocks; ++i)
		{
			char name[256];
			GLsizei length = 0;
			glGetActiveUniformBlockName(m_program, i, sizeof(name), &length, name);
			printf("Uniform block %u : %s\n", i, name);
		}


		glBindAttribLocation(m_program, static_cast<GLuint>(VERTEX_ATTRIBUTES::POSITION), "a_position");
		glBindAttribLocation(m_program, static_cast<GLuint>(VERTEX_ATTRIBUTES::NORMAL), "a_normal");
		// BIND TEXCOORD
		// BIND TANGENT
		// BIND BITANGENT

		m_matricesUniformBinding = glGetUniformBlockIndex(m_program, "Matrices");
		assert(m_matricesUniformBinding != GL_INVALID_INDEX);
		glUniformBlockBinding(m_program, m_matricesUniformBinding, 0);
	}

	m_lightsUniformBinding = glGetUniformBlockIndex(m_program, "Lights");
	assert(m_lightsUniformBinding != GL_INVALID_INDEX);
	glUniformBlockBinding(m_program, m_lightsUniformBinding, 1);

	m_vertexArray = OpenGl::CVertexArray::Create();

	{
		glBindVertexArray(m_vertexArray);

		glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

		glEnableVertexAttribArray(VERTEX_ATTRIBUTES::POSITION);
		glVertexAttribPointer(VERTEX_ATTRIBUTES::POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), reinterpret_cast<GLvoid*>(offsetof(VERTEX, position)));

		glEnableVertexAttribArray(VERTEX_ATTRIBUTES::NORMAL);
		glVertexAttribPointer(VERTEX_ATTRIBUTES::NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), reinterpret_cast<GLvoid*>(offsetof(VERTEX, normal)));

		//glEnableVertexAttribArray(VERTEX_ATTRIBUTES::TEXCOORD);
		//glVertexAttribPointer(VERTEX_ATTRIBUTES::TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(VERTEX), reinterpret_cast<GLvoid*>(offsetof(VERTEX, texcoord)));

		//glEnableVertexAttribArray(VERTEX_ATTRIBUTES::TANGENT);
		//glVertexAttribPointer(VERTEX_ATTRIBUTES::TANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), reinterpret_cast<GLvoid*>(offsetof(VERTEX, tangent)));

		//glEnableVertexAttribArray(VERTEX_ATTRIBUTES::BITANGENT);
		//glVertexAttribPointer(VERTEX_ATTRIBUTES::BITANGENT, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), reinterpret_cast<GLvoid*>(offsetof(VERTEX, bitangent)));

		glBindVertexArray(0);
	}
}

void CMeshScene::Update(double dt)
{
	CScene::Update(dt);

	float aspectRatio = static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight);

	glm::mat4 projMat = glm::perspective(glm::pi<float>() * 0.25f, aspectRatio, 0.1f, 1000.f);
	glm::mat4 viewMat = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -200.0f));

	// Model matrix: translate down a bit and rotate over time
	glm::mat4 worldMat = glm::translate(glm::mat4(1.0f), -glm::vec3(0.0f, 50.0f, 0.0f));
	worldMat = glm::rotate(worldMat, static_cast<float>(m_currentTime), glm::vec3(0, 1, 0));
	worldMat = glm::scale(worldMat, glm::vec3(1.0f)); // adjust if needed

	m_matrices.worldViewProjMatrix = projMat * viewMat * worldMat;

	m_lights.lights[0].ambientColor = glm::vec4(0.1, 0.1, 0.1, 0);
	m_lights.lights[0].diffuseColor = glm::vec4(1.0, 0.0, 0.0, 0);
	m_lights.lights[0].specularColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
	m_lights.lights[0].dir = glm::vec4(sin(m_currentTime), 0, cos(m_currentTime), 0);

	m_lights.lights[1].diffuseColor = glm::vec4(1, 1, 1, 0);
	m_lights.lights[1].specularColor = glm::vec4(1, 1, 1, 0);
	m_lights.lights[1].pos = glm::vec4(0.0f, 0.5 * cos(m_currentTime * 5), 0.75f,
	                                   0.0f);
	m_lights.lights[1].type = LIGHT_TYPE::POINT;
	m_lights.lights[1].linAttenuation = 2;
	m_lights.lights[1].quadAttenuation = 10;

	m_lights.lights[0].type = LIGHT_TYPE::DIRECTIONAL;
	m_lights.lights[1].type = LIGHT_TYPE::POINT;

	m_lights.viewDir = glm::vec4(glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)), 0.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_matrices), &m_matrices, GL_DYNAMIC_DRAW);

	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_lightsUniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_lights), &m_lights, GL_DYNAMIC_DRAW);
}

void CMeshScene::Draw()
{
	glViewport(0, 0, m_windowWidth, m_windowHeight);

	glClearDepthf(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE);

	glUseProgram(m_program);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniformBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_lightsUniformBuffer);
	glBindVertexArray(m_vertexArray);
	glDrawElements(GL_TRIANGLES, m_numIndices, GL_UNSIGNED_INT, nullptr);
}
