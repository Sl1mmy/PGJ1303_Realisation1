#include "MultipleObjectScene.h"
#include "OpenGlShader.h"
#include <glm/vec3.hpp>
#include <glm/ext/matrix_clip_space.hpp>
#include <glm/ext/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <iostream>

enum UNIFORM_BINDINGS
{
	MATRICES,
	LIGHTS,
};

CMultipleObjectScene::CMultipleObjectScene()
{
	m_meshes.push_back(LoadMeshFromAssimp("./resources/models/teapot.dae"));
	m_meshes.push_back(CreateCube());


	{
		m_uniformBuffer = OpenGl::CBuffer::Create();
		glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(m_matrices), &m_matrices, GL_DYNAMIC_DRAW);
	}

	{
		m_lightsUniformBuffer = OpenGl::CBuffer::Create();
		glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUniformBuffer);
		glBufferData(GL_UNIFORM_BUFFER, sizeof(m_lights), &m_lights, GL_DYNAMIC_DRAW);
	}

	/* {
		auto vertShader = OpenGl::CShader::CreateFromFile(GL_VERTEX_SHADER, "./shaders/proj_v.glsl");
		auto fragShader = OpenGl::CShader::CreateFromFile(GL_FRAGMENT_SHADER, "./shaders/proj_f.glsl");

		vertShader.Compile();
		fragShader.Compile();

		m_program = OpenGl::CProgram::Create();
		m_program.AttachShader(vertShader);
		m_program.AttachShader(fragShader);
		m_program.Link();

		m_matricesUniformBinding = glGetUniformBlockIndex(m_program, "Matrices");
		assert(m_matricesUniformBinding != GL_INVALID_INDEX);
		glUniformBlockBinding(m_program, m_matricesUniformBinding, 0);
	}*/

	{
		auto vertShader = OpenGl::CShader::CreateFromFile(GL_VERTEX_SHADER, "./shaders/light_v.glsl");
		auto fragShader = OpenGl::CShader::CreateFromFile(GL_FRAGMENT_SHADER, "./shaders/light_f.glsl");

		vertShader.Compile();
		fragShader.Compile();

		m_program = OpenGl::CProgram::Create();
		m_program.AttachShader(vertShader);
		m_program.AttachShader(fragShader);
		m_program.Link();

		glBindAttribLocation(m_program, static_cast<GLuint>(VERTEX_ATTRIBUTES::POSITION), "a_position");
		glBindAttribLocation(m_program, static_cast<GLuint>(VERTEX_ATTRIBUTES::NORMAL), "a_normal");

		{
			m_matricesUniformBinding = glGetUniformBlockIndex(m_program, "Matrices");
			assert(m_matricesUniformBinding != GL_INVALID_INDEX);
			glUniformBlockBinding(m_program, m_matricesUniformBinding, 0);
		}

		{
			m_lightsUniformBinding = glGetUniformBlockIndex(m_program, "Lights");
			assert(m_lightsUniformBinding != GL_INVALID_INDEX);
			glUniformBlockBinding(m_program, m_lightsUniformBinding, 1);
		}
	}
}

void CMultipleObjectScene::Update(double dt)
{
	CScene::Update(dt);

	float aspectRatio = static_cast<float>(m_windowWidth) / static_cast<float>(m_windowHeight);

	// Projection + view
	m_matrices.proj = glm::perspective(glm::pi<float>() * 0.25f, aspectRatio, 0.1f, 1000.f);
	m_matrices.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -200.0f));

	// Update Model Matrix Per Object:

	// Teapot
	m_meshes[0].position = glm::vec3(100.0f, -50.0f, 0.0f);

	// Cube
	m_meshes[1].position = glm::vec3(-50.0f, -25.0f, 0.0f);
	m_meshes[1].rotation.y = static_cast<float>(m_currentTime) * 2;
	m_meshes[1].scale = glm::vec3(25.0f);

	glBindBuffer(GL_UNIFORM_BUFFER, m_uniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_matrices), &m_matrices, GL_DYNAMIC_DRAW);

	m_lights.viewDir = m_matrices.view[2];

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


	glBindBuffer(GL_UNIFORM_BUFFER, m_lightsUniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, sizeof(m_lights), &m_lights, GL_DYNAMIC_DRAW);
}

void CMultipleObjectScene::Draw()
{
	glViewport(0, 0, m_windowWidth, m_windowHeight);

	glClearDepthf(1.0f);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//glEnable(GL_CULL_FACE);
	glDisable(GL_CULL_FACE);

	glUseProgram(m_program);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, m_uniformBuffer);
	glBindBufferBase(GL_UNIFORM_BUFFER, 1, m_lightsUniformBuffer); 

	GLint modelLoc = glGetUniformLocation(m_program, "model");

	// Draw each mesh with its own model matrix
	for (const auto& mesh : m_meshes)
	{
		glm::mat4 modelMat(1.0f);
		modelMat = glm::translate(modelMat, mesh.position);
		
		modelMat = glm::rotate(modelMat, mesh.rotation.y, glm::vec3(0, 1, 0));
		modelMat = glm::rotate(modelMat, mesh.rotation.x, glm::vec3(1, 0, 0));
		modelMat = glm::rotate(modelMat, mesh.rotation.z, glm::vec3(0, 0, 1));
		
		modelMat = glm::scale(modelMat, mesh.scale);

		glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &modelMat[0][0]);

		glBindVertexArray(mesh.vaoResource);
		glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
	}
}


MeshData CMultipleObjectScene::CreateMeshData(const std::vector<VERTEX>& vertices, const std::vector<uint32_t>& indices)
{
	MeshData mesh{};
	mesh.indexCount = static_cast<GLsizei>(indices.size());

	// VBO
	mesh.vboResource = OpenGl::CBuffer::Create();

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vboResource);
	glBufferData(GL_ARRAY_BUFFER, sizeof(VERTEX) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	// IBO
	mesh.iboResource = OpenGl::CBuffer::Create();
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.iboResource);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * indices.size(), indices.data(), GL_STATIC_DRAW);

	// VAO
	mesh.vaoResource = OpenGl::CVertexArray::Create();
	glBindVertexArray(mesh.vaoResource);

	glBindBuffer(GL_ARRAY_BUFFER, mesh.vboResource);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.iboResource);

	glEnableVertexAttribArray(POSITION);
	glVertexAttribPointer(POSITION, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), reinterpret_cast<GLvoid*>(offsetof(VERTEX, position)));

	glEnableVertexAttribArray(NORMAL);
	glVertexAttribPointer(NORMAL, 3, GL_FLOAT, GL_FALSE, sizeof(VERTEX), reinterpret_cast<GLvoid*>(offsetof(VERTEX, normal)));

	glEnableVertexAttribArray(COLOR);
	glVertexAttribPointer(COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(VERTEX), reinterpret_cast<GLvoid*>(offsetof(VERTEX, color)));

	glBindVertexArray(0);

	return mesh;
}

MeshData CMultipleObjectScene::LoadMeshFromAssimp(const std::string& path)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(path,
	                                         aiProcess_Triangulate |
	                                             aiProcess_GenSmoothNormals |
	                                             aiProcess_FlipUVs |
	                                             aiProcess_CalcTangentSpace);

	assert(scene && scene->HasMeshes());
	auto mesh = scene->mMeshes[0];
	assert(mesh->mPrimitiveTypes == aiPrimitiveType_TRIANGLE);

	std::vector<VERTEX> vertices;
	vertices.reserve(mesh->mNumVertices);
	for(unsigned i = 0; i < mesh->mNumVertices; i++)
	{
		auto pos = mesh->mVertices[i];
		auto norm = mesh->mNormals[i];
		vertices.push_back({{pos.x, pos.y, pos.z}, {norm.x, norm.y, norm.z}, {0.6f, 0.3f, 0.1f, 1.0f}}); // default white color
	}

	std::vector<uint32_t> indices;
	indices.reserve(mesh->mNumFaces * 3);
	for(unsigned i = 0; i < mesh->mNumFaces; i++)
	{
		auto face = mesh->mFaces[i];
		assert(face.mNumIndices == 3);
		for(unsigned j = 0; j < face.mNumIndices; j++)
			indices.push_back(face.mIndices[j]);
	}

	return CreateMeshData(vertices, indices);
}

MeshData CMultipleObjectScene::CreateCube()
{
	std::vector<VERTEX> CubeVertices =
	{
	    {{-1.0f, 1.0f, -1.0f}, {0, 0, 1}, {1.0f, 0.0f, 0.0f, 1.0f}}, // red
	    {{1.0f, 1.0f, -1.0f}, {0, 0, 1}, {0.0f, 1.0f, 0.0f, 1.0f}},  // green
	    {{1.0f, 1.0f, 1.0f}, {0, 0, 1}, {0.0f, 0.0f, 1.0f, 1.0f}},   // blue
	    {{-1.0f, 1.0f, 1.0f}, {0, 0, 1}, {1.0f, 1.0f, 1.0f, 1.0f}},  // white

	    {{-1.0f, -1.0f, 1.0f}, {0, 0, 1}, {1.0f, 1.0f, 0.0f, 1.0f}},  // yellow
	    {{1.0f, -1.0f, 1.0f}, {0, 0, 1}, {1.0f, 0.0f, 1.0f, 1.0f}},   // magenta
	    {{1.0f, -1.0f, -1.0f}, {0, 0, 1}, {0.0f, 1.0f, 1.0f, 1.0f}},  // cyan
	    {{-1.0f, -1.0f, -1.0f}, {0, 0, 1}, {0.5f, 0.5f, 0.5f, 1.0f}}, // gray
	};

	std::vector<uint32_t> cubeIndices =
	{
	    0, 1, 2,
	    0, 2, 3,

	    4, 5, 6,
	    4, 6, 7,

	    3, 2, 5,
	    3, 5, 4,

	    2, 1, 6,
	    2, 6, 5,

	    1, 7, 6,
	    1, 0, 7,

	    0, 3, 4,
	    0, 4, 7
	};

	return CreateMeshData(CubeVertices, cubeIndices);
}