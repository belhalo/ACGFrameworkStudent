#include "material.h"

#include "application.h"

#include <istream>
#include <fstream>
#include <algorithm>

/*
make a flat.js inside of shaders -> check in application.cpp
*/

// [.vs] - vertexShader:
// Executed for every vertex. It is used mostly to project from local coordinates to clip coordinates (use mvp)

// [.fs] - fragmentShader:
// After the rasterization of the triangles; executed for every pixel


FlatMaterial::FlatMaterial(glm::vec4 color)
{
	this->color = color;
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/flat.fs");
}

FlatMaterial::~FlatMaterial() { }

void FlatMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	// Upload node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	this->shader->setUniform("u_camera_position", camera->eye);
	this->shader->setUniform("u_model", model);

	this->shader->setUniform("u_color", this->color);
}

void FlatMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (mesh && this->shader) {
		// Enable shader
		this->shader->enable();

		// Upload uniforms
		setUniforms(camera, model);

		// Do the draw call
		mesh->render(GL_TRIANGLES);

		this->shader->disable();
	}
}

void FlatMaterial::renderInMenu()
{
	ImGui::Text("Material Type: %s", std::string("Flat").c_str());

	ImGui::ColorEdit3("Color", (float*)&this->color);
}

WireframeMaterial::WireframeMaterial()
{
	this->color = glm::vec4(1.f);
	this->shader = Shader::Get("res/shaders/basic.vs", "res/shaders/flat.fs");
}

WireframeMaterial::~WireframeMaterial() { }

void WireframeMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (this->shader && mesh)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glDisable(GL_CULL_FACE);

		// Enable shader
		this->shader->enable();

		// Upload material specific uniforms
		setUniforms(camera, model);

		// Do the draw call
		mesh->render(GL_TRIANGLES);

		glEnable(GL_CULL_FACE);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
}

StandardMaterial::StandardMaterial(glm::vec4 color)
{
	this->color = color;
	this->base_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/basic.fs");
	this->normal_shader = Shader::Get("res/shaders/basic.vs", "res/shaders/normal.fs");
	this->shader = this->base_shader;
}

StandardMaterial::~StandardMaterial() { }

void StandardMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	// Upload node uniforms
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);
	this->shader->setUniform("u_camera_position", camera->eye);
	this->shader->setUniform("u_model", model);

	this->shader->setUniform("u_color", this->color);

	if (this->texture) {
		this->shader->setUniform("u_texture", this->texture, 0);
	}
}

void StandardMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	bool first_pass = true;
	if (mesh && this->shader)
	{
		// Enable shader
		this->shader->enable();

		// Multi pass render
		int num_lights = (int)Application::instance->light_list.size();
		for (int nlight = -1; nlight < num_lights; nlight++)
		{
			if (nlight == -1) { nlight++; } // hotfix

			// Upload uniforms
			setUniforms(camera, model);

			// Upload light uniforms
			if (!first_pass) {
				glBlendFunc(GL_SRC_ALPHA, GL_ONE);
				glDepthFunc(GL_LEQUAL);
			}
			this->shader->setUniform("u_ambient_light", Application::instance->ambient_light * (float)first_pass);

			if (num_lights > 0) {
				Light* light = Application::instance->light_list[nlight];
				light->setUniforms(this->shader, model);
			}
			else {
				// Set some uniforms in case there is no light
				this->shader->setUniform("u_light_intensity", 1.f);
				this->shader->setUniform("u_light_shininess", 1.f);
				this->shader->setUniform("u_light_color", glm::vec4(0.f));
			}

			// Do the draw call
			mesh->render(GL_TRIANGLES);
            
			first_pass = false;
		}

		// Disable shader
		this->shader->disable();
	}
}

void StandardMaterial::renderInMenu()
{
	ImGui::Text("Material Type: %s", std::string("Standard").c_str());

	if (ImGui::Checkbox("Show Normals", &this->show_normals)) {
		if (this->show_normals) {
			this->shader = this->normal_shader;
		}
		else {
			this->shader = this->base_shader;
		}
	}

	if (!this->show_normals) ImGui::ColorEdit3("Color", (float*)&this->color);
}

// Task1
// here we choose the files .vs and .fs to compute the color material!!
VolumeMaterial::VolumeMaterial(glm::vec4 color)
{
	this->color = color;
	// CHANGE ONCE WE HAVE our .vs and .fs edited!
	this->shader = Shader::Get("res/shaders/volume.vs", "res/shaders/volume.fs");

	// init default values 
	this->absorptionCoefficient = 0.03f;
}

VolumeMaterial::~VolumeMaterial() {}
void VolumeMaterial::setUniforms(Camera* camera, glm::mat4 model)
{
	glm::mat4 inverseModel = glm::inverse(model);
	glm::vec4 temp = glm::vec4(camera->eye, 1.0);
	temp = inverseModel * temp;
	glm::vec3 local_camera_pos = glm::vec3(temp.x / temp.w, temp.y / temp.w, temp.z / temp.w);

	// Upload node uniforms
	this->shader->setUniform("u_background_color", this->backgroundColor);
	this->shader->setUniform("u_absorption", this->absorptionCoefficient);

	this->shader->setUniform("u_local_camera_position", local_camera_pos);
	this->shader->setUniform("u_inverse_model", inverseModel);

	this->shader->setUniform("u_step_len", this->stepLength);
	this->shader->setUniform("u_noise", this->noise);
	this->shader->setUniform("u_volume_type", this->volumeType);

	this->shader->setUniform("u_model", model);
	this->shader->setUniform("u_viewprojection", camera->viewprojection_matrix);

	this->shader->setUniform("u_color", this->color);
}

void VolumeMaterial::render(Mesh* mesh, glm::mat4 model, Camera* camera)
{
	if (mesh && this->shader) {
		// enable shader
		this->shader->enable();

		// upload uniforms
		setUniforms(camera, model);

		// do the draw call
		mesh->render(GL_TRIANGLES);

		this->shader->disable();
	}
}

void VolumeMaterial::renderInMenu()
{
	//mGui::Text("Material Type: %s", std::string("Flat").c_str());
	// ImGui::ColorEdit3("Color", (float*)&this->color);

	ImGui::Text("Material Type: Volume");

    // to modify the background color is in the scene renderInMenu() (should be there but..)
	// modify the bgColor too!
	// ImGui::ColorEdit3("Background Color", &backgroundColor.x);

	// slider to modify in real time the absorption coeff
	ImGui::SliderFloat(
		"Absorption",
		&absorptionCoefficient,
		0.0f,
		5.0f
	);

	// modify step len
	ImGui::SliderFloat(
		"Step Length",
		&this->stepLength,
		0.001f,
		0.050f
	);

	// select the volume type
	const char* volumeTypes[] = {
		"Homogeneous",
		"Heterogeneous"
	};

	ImGui::Combo(
		"Volume Type",
		&this->volumeType,
		volumeTypes,
		IM_ARRAYSIZE(volumeTypes)
	);

	// modify noise value
	ImGui::SliderFloat(
		"Noise Scale",
		&this->noise,
		1.0f,
		5.0f
	);
}