#include "application.h"

bool render_wireframe = false;
Camera* Application::camera = nullptr;

// Application::Init
// Application::update
// Application::render

void Application::init(GLFWwindow* window)
{
    this->instance = this;
    // init the framebuffer sizes (w, h)
    glfwGetFramebufferSize(window, &this->window_width, &this->window_height);

    // OpenGL flags -> enable the use of the gpu!!
    glEnable(GL_CULL_FACE); // render both sides of every triangle
    glEnable(GL_DEPTH_TEST); // check the occlusions using the Z buffer

    // create camera and set up its values
    this->camera = new Camera();
    this->camera->lookAt(glm::vec3(1.f, 1.5f, 4.f), glm::vec3(0.f, 0.0f, 0.f), glm::vec3(0.f, 1.f, 0.f));
    this->camera->setPerspective(60.f, this->window_width / (float)this->window_height, 0.1f, 500.f); // set the projection to perspective

    this->flag_grid = true;
    this->flag_wireframe = false;

    // put ambient light
    this->ambient_light = glm::vec4(0.75f, 0.75f, 0.75f, 1.f);

    // put bgColor
    this->background_color = glm::vec3(0.1f, 0.1f, 0.1f);

    /* ADD NODES TO THE SCENE */
    /*SceneNode* example = new SceneNode("Example Node");
    example->mesh = Mesh::Get("res/meshes/sphere.obj");
    example->material = new StandardMaterial();
    this->node_list.push_back(example);*/

    // Task1
    VolumeNode* v1 = new VolumeNode();
    v1->mesh = Mesh::Get("res/meshes/cube.obj");
    v1->material = new VolumeMaterial();
    v1->material->backgroundColor = this->background_color; //added the backgroundCOlor!!!
    this->node_list.push_back(v1);
}

void Application::update(float dt)
{
    // Mouse update
    glm::vec2 delta = this->lastMousePosition - this->mousePosition;
    if (this->dragging) {
        this->camera->orbit(-delta.x * dt, delta.y * dt);
    }
    this->lastMousePosition = this->mousePosition;
}

void Application::render()
{
    // Set the clear color (the background color)
        //glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

    // apply the modification of the bgColor in real-time
    glClearColor(
        this->background_color.r,
        this->background_color.g,
        this->background_color.b,
        1.0f
    );

    // Clear the window and the depth buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Set flags
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    for (unsigned int i = 0; i < this->node_list.size(); i++)
    {
        // Task1 update the bgColor 
        this->node_list[i]->material->backgroundColor = this->background_color; 
        this->node_list[i]->render(this->camera);
        if (this->flag_wireframe) this->node_list[i]->renderWireframe(this->camera);
    }

    // Draw the floor grid
    if (this->flag_grid) drawGrid();
}

void Application::renderGUI()
{
    if (ImGui::TreeNodeEx("Scene", ImGuiTreeNodeFlags_DefaultOpen))
    {
        ImGui::ColorEdit3("Ambient light", (float*)&this->ambient_light);

        // modify the bgColor too!
        ImGui::ColorEdit3("Background Color", (float*)&this->background_color);

        if (ImGui::TreeNode("Camera")) {
            this->camera->renderInMenu();
            ImGui::TreePop();
        }

        unsigned int count = 0;
        std::stringstream ss;
        for (auto& node : this->node_list) {
            ss << count;
            if (ImGui::TreeNode(node->name.c_str())) {
                node->renderInMenu();
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

void Application::shutdown() { }

// keycodes: https://www.glfw.org/docs/3.3/group__keys.html
void Application::onKeyDown(int key, int scancode)
{
    switch (key) {
    case GLFW_KEY_ESCAPE: // quit
        close = true;
        break;
    case GLFW_KEY_R:
        Shader::ReloadAll();
        break;
    }
}

// keycodes: https://www.glfw.org/docs/3.3/group__keys.html
void Application::onKeyUp(int key, int scancode)
{
    switch (key) {
    case GLFW_KEY_T:
        std::cout << "T released" << std::endl;
        break;
    }
}

void Application::onRightMouseDown()
{
    this->dragging = true;
    this->lastMousePosition = this->mousePosition;
}

void Application::onRightMouseUp()
{
    this->dragging = false;
    this->lastMousePosition = this->mousePosition;
}

void Application::onLeftMouseDown()
{
    this->dragging = true;
    this->lastMousePosition = this->mousePosition;
}

void Application::onLeftMouseUp()
{
    this->dragging = false;
    this->lastMousePosition = this->mousePosition;
}

void Application::onMiddleMouseDown() { }

void Application::onMiddleMouseUp() { }

void Application::onMousePosition(double xpos, double ypos) { }

void Application::onScroll(double xOffset, double yOffset)
{
    int min = (int)this->camera->min_fov;
    int max = (int)this->camera->max_fov;

    if (yOffset < 0) {
        this->camera->fov += 4.f;
        if (this->camera->fov > max) {
            this->camera->fov = (float)max;
        }
    }
    else {
        this->camera->fov -= 4.f;
        if (this->camera->fov < min) {
            this->camera->fov = (float)min;
        }
    }
    this->camera->updateProjectionMatrix();
}

void Application::onWindowSize(int width, int height)
{
    glViewport(0, 0, width, height);
    this->camera->setAspectRatio(width / (float)height);
    this->camera->updateProjectionMatrix();
}