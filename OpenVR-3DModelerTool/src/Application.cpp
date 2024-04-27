#include "OpenGL/Shader.h"
#include "OpenGL/GLLoader.h"
#include "OpenGL/VertexBuffer.h"
#include "OpenGL/VertexArray.h"
#include "OpenGL/IndexBuffer.h"
#include "OpenGL/VertexBufferLayout.h"
#include "OpenGL/Renderer.h"
#include "Renderer/Camera.h"
#include <iostream>
#include <glm/vec3.hpp> // glm::vec3
#include <glm/vec4.hpp> // glm::vec4
#include <glm/mat4x4.hpp> // glm::mat4
#include <glm/ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <glm/ext/matrix_clip_space.hpp> // glm::perspective
#include <glm/ext/scalar_constants.hpp> // glm::pi
#include <glm/gtc/type_ptr.hpp>

#include"imgui.h"
#include"imgui_impl_glfw.h"
#include"imgui_impl_opengl3.h"
#include"imgui_internal.h"

#include "Renderer/Model.h"

#include "openvr.h"
#include <stdlib.h>
#include <filesystem>
#include "Renderer/Scene.h"

#include <dirent.h>
#include <limits.h>

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
//camera settings
Camera camera(0.0f, 0.0f, 0.0f);
//cursor settings
float yaw = -90.0f;
float pitch = 0.0f;
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
//vector
int vertex_size = 0;
int total_edges;
//std::vector<Vertex> verticese;


float fov = 45.0f;

bool isMousePressed = false;
double lastMouseX, lastMouseY;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
//void addVertex(Vertex v, std::vector<GLfloat>& vertices, std::vector<GLuint>& indices);

std::vector<GLfloat> vertices;
std::vector<GLuint> indices;

int main() {    
    //OpenVRApplication openVR;

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // GLFW configuration
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    //OpenVRApplication vrApp;

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "VR Tool", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Create a Shader object and load shaders from files
    Shader shader("shaders/Base.shader");
    glm::vec3 lightPos(2.0f, 1.0f, 2.0f);
    shader.Bind();
    //shader.SetVec3("objectColor", 1.0f, 0.5f, 0.31f);
    //shader.SetVec3("lightColor", 1.0f, 1.0f, 1.0f);
    //shader.SetVec3("lightPos", lightPos);
                                     
    Renderer renderer;
    // Use the shader program
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set up callback functions
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    // Set up mouse button callback function
    glfwSetMouseButtonCallback(window, mouse_button_callback);


    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    std::vector<string> supportModels = {
        ".3mf", ".dae", ".xml", ".blend", ".bvh", ".3ds", ".ase", ".glTF", ".glTF", ".fbx",
        ".ply", ".dxf", ".ifc", ".iqm", ".nff", ".nff", ".smd", ".vta", ".mdl", ".md2",
        ".md3", ".pk3", ".mdc", ".md5mesh", ".md5anim", ".md5camera", ".x", ".q3o", ".q3s",
        ".raw", ".ac", ".ac3d", ".stl", ".dxf", ".irrmesh", ".xml", ".irr", ".xml", ".off",
        ".obj", ".ter", ".mdl", ".hmp", ".mesh.xml", ".skeleton.xml", ".material", ".ogex", ".ms3d",
        ".lwo", ".lws", ".lxo", ".csm", ".ply", ".cob", ".scn", ".xgl",
    };

    std::vector<Model> models;

    DIR* directory = opendir("models");
    if (directory != nullptr) {
        struct dirent* entry;
        while ((entry = readdir(directory)) != nullptr) {
            if (entry->d_type == DT_REG) {
                for (string filetype : supportModels) {
                    if (std::strstr(entry->d_name, filetype.c_str())) {
                        char truePath[PATH_MAX];
                        snprintf(truePath, sizeof(truePath), "./models/%s", entry->d_name);
                        Model m(truePath, false);
                        models.push_back(m);
                        std::cout << "File: " << entry->d_name << std::endl;
                    }

                }
            }
        }

        closedir(directory);
    }
    else {
        std::cerr << "Failed to open directory." << std::endl;
    }

    std::vector<Mesh> meshes;
    std::vector<Material> materials;

    Scene scene(models, meshes, materials, camera, shader);
    int index = 1;
    int i = 0;

    while (!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        processInput(window);
        
        ImGui::Begin("Tool");
        ImGui::Text("Settings");
        ImGui::Text("Draw Objects");

        if (index > scene.models.size()) {
            index = scene.models.size();
        }

        if (ImGui::Button("<")) {
            if (index > 1) {
                index--;
            }
        }
        ImGui::SameLine();
        ImGui::Text("< %d >", index);
        ImGui::SameLine();
        if (ImGui::Button(">")) {
            if (index < scene.models.size()) {
                index++;
            }
        }
        i = index - 1;

        if (scene.models.size() > 0) {
            bool move = false;
            ImGui::Columns(3, "Coordinates");

            //--- Movement Along X-Axis
            ImGui::Text("Move X-Axis");
            ImGui::Button("+X");
            if (ImGui::IsItemActive()) {
                scene.models[i].Forward(1.0f);
            }
            ImGui::SameLine();
            ImGui::Button("-X");
            if (ImGui::IsItemActive()) {
                scene.models[i].Forward(-1.0f);
            }
            //--- Movement Along Y-Axis
            ImGui::Text("Move Y-Axis");
            ImGui::Button("+Y");
            if (ImGui::IsItemActive()) {
                scene.models[i].Upwards(1.0f);
            }
            ImGui::SameLine();
            ImGui::Button("-Y");
            if (ImGui::IsItemActive()) {
                scene.models[i].Upwards(-1.0f);
            }
            //--- Movement Along Z-Axis
            ImGui::Text("Move Z-Axis");
            ImGui::Button("+Z");
            if (ImGui::IsItemActive()) {
                scene.models[i].Strafe(1.0f);
            }
            ImGui::SameLine();
            ImGui::Button("-Z");
            if (ImGui::IsItemActive()) {
                scene.models[i].Strafe(-1.0f);
            }
            ImGui::NextColumn();


            //--- Rotate Along X-Axis
            ImGui::Text("Rotate X-Axis");
            ImGui::Button("+Pitch");
            if (ImGui::IsItemActive()) {
                scene.models[i].RotateX(1.0f);
            }
            ImGui::SameLine();
            ImGui::Button("-Pitch");
            if (ImGui::IsItemActive()) {
                scene.models[i].RotateX(-1.0f);
            }

            //--- Rotate Along Y-Axis
            ImGui::Text("Rotate Y-Axis");
            ImGui::Button("+Yaw");
            if (ImGui::IsItemActive()) {
                scene.models[i].RotateY(1.0f);
            }
            ImGui::SameLine();
            ImGui::Button("-Yaw");
            if (ImGui::IsItemActive()) {
                scene.models[i].RotateY(-1.0f);
            }

            //--- Rotate Along Z-Axis
            ImGui::Text("Rotate Z-Axis");
            ImGui::Button("+Roll");
            if (ImGui::IsItemActive()) {
                scene.models[i].RotateZ(1.0f);
            }
            ImGui::SameLine();
            ImGui::Button("-Roll");
            if (ImGui::IsItemActive()) {
                scene.models[i].RotateZ(-1.0f);
            }

            ImGui::NextColumn();
            ImGui::Text("Scale");
            ImGui::Button("+");
            if (ImGui::IsItemActive()) {
                scene.models[i].Scale(1.005f);
            }
            ImGui::Button("-");
            if (ImGui::IsItemActive()) {
                scene.models[i].Scale(0.995f);
            }

            ImGui::Text("Duplicate");
            if (ImGui::Button("[]")) {
                Model m = *new Model(scene.models[i]);
                scene.models.push_back(m);
            }

            ImGui::Text("Delete");
            if (ImGui::Button("][")) {
                scene.models.erase(scene.models.begin() + i);
            }
            if (ImGui::IsItemActive()) {
                scene.models[i].Scale(1.005f);
            }
            ImGui::EndColumns();
        }
        scene.Render();
        scene.RenderStereoTargets();

        if (ImGui::Button("Close Application")) {
            //Action to close the application
            return 0;
        }
        ImGui::End();
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;
}

//calculateDistance float for removeVertex functionality
float calculateDistance(float x1, float y1, float x2, float y2) {
    return sqrt(pow(x2 - x1, 2) + pow(y2 - y1, 2));
}


// mouse interactions
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_MIDDLE) {
        if (action == GLFW_PRESS) {
            isMousePressed = true;
            glfwGetCursorPos(window, &lastMouseX, &lastMouseY);
        }
        else if (action == GLFW_RELEASE) {
            isMousePressed = false;
        }
    }
}
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (isMousePressed) {
        double xOffset = xpos - lastMouseX;
        double yOffset = lastMouseY - ypos;
        lastMouseX = xpos;
        lastMouseY = ypos;

        const float sensitivity = 0.1f;
        xOffset *= sensitivity;
        yOffset *= sensitivity;

        yaw += xOffset;
        pitch += yOffset;

        if (pitch > 89.0f) pitch = 89.0f;
        if (pitch < -89.0f) pitch = -89.0f;

        camera.Rotate(yaw, pitch);
    }
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.MoveForward(0.05f);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.MoveForward(-0.05f);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.MoveSideways(-0.05f);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.MoveSideways(0.05f);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        glLineWidth(5.0f);
        GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
    }
    else if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
    {
        glPointSize(10.0f);
        GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_POINT));
    }
    else
    {
        GLCall(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}