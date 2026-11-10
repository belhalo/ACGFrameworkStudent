#pragma once

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/matrix.hpp>

#include "../framework/camera.h"
#include "mesh.h"
#include "texture.h"
#include "shader.h"

class Material {
public:

	Shader* shader = NULL;
	Texture* texture = NULL;
	glm::vec4 color;

	// Task1
	glm::vec3 backgroundColor = glm::vec3(0.1f);

	virtual void setUniforms(Camera* camera, glm::mat4 model) = 0;
	virtual void render(Mesh* mesh, glm::mat4 model, Camera* camera) = 0;
	virtual void renderInMenu() = 0;
};

class FlatMaterial : public Material {
public:

	FlatMaterial(glm::vec4 color = glm::vec4(1.f));
	~FlatMaterial();

	void setUniforms(Camera* camera, glm::mat4 model);
	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
	void renderInMenu();
};

class WireframeMaterial : public FlatMaterial {
public:

	WireframeMaterial();
	~WireframeMaterial();

	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
};

class StandardMaterial : public Material {
public:

	bool first_pass = false;

	bool show_normals = false;
	Shader* base_shader = NULL;
	Shader* normal_shader = NULL;

	StandardMaterial(glm::vec4 color = glm::vec4(1.f));
	~StandardMaterial();

	void setUniforms(Camera* camera, glm::mat4 model);
	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
	void renderInMenu();
};

// Task1
// VolumeMaterial = propietats del material -> comportament d'aquest i com resulta en un color RGB, computant la llum
class VolumeMaterial : public Material {
public:
	// L(t) = BT, where T (transmittance) = exp(-(tb−ta)µa) <- Beer-Lamber Law
	float absorptionCoefficient; // (µa) cte absorption coefficient -> volum homogeni
	float stepLength = 0.01f;
	// [true] -> heterogeneous
	// [false] -> homogeneous
	int volumeType = 1; //// 0 = Homogeneous, 1 = Heterogeneous
	float noise = 1.0f; // noise value 

	// constructors 
	VolumeMaterial(glm::vec4 color = glm::vec4(1.f));
	~VolumeMaterial();

	// will use the fs and sh files!
	void setUniforms(Camera* camera, glm::mat4 model);
	void render(Mesh* mesh, glm::mat4 model, Camera* camera);
	void renderInMenu();
};
