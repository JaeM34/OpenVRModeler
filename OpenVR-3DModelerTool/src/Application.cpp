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

#include "Renderer/Model.h"

#include "openvr.h"
#include <stdlib.h>
#include <filesystem>
#include "Renderer/Scene.h"

struct Vertex {
    unsigned int index;
    float x;
    float y;
    float z;

    std::vector<unsigned int> edges;
};
// window settings
const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;
//camera settings
//glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 3.0f);
//glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
//glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
//float cameraSpeed = 0.05f;
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
std::vector<Vertex> verticese;


float fov = 45.0f;

bool isMousePressed = false;
double lastMouseX, lastMouseY;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void addVertex(Vertex v, std::vector<GLfloat>& vertices, std::vector<GLuint>& indices);
void removeVertex(unsigned int index);
void createData(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices);

std::vector<GLfloat> vertices;
std::vector<GLuint> indices;

unsigned int index_pos;

void createGridData(std::vector<GLfloat>& gridVertices, std::vector<GLuint>& gridIndices, int gridSizeX, int gridSizeZ, float gridSpacing);

int main() {    
    //OpenVRApplication *openVR = new OpenVRApplication();

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
    shader.Bind();
                                     
    Renderer renderer;
    // Use the shader program
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set up callback functions
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    // Set up mouse button callback function
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    GLfloat verticespyramid[] = {
    -0.5f, 0.0f, -0.5f,
    0.5f, 0.0f, -0.5f,
    0.5f, 0.0f, 0.5f,
    -0.5f, 0.0f, 0.5f,
    0.0f, 1.0f, 0.0f
    };

    GLuint indicespyramid[] = {
        0, 1, 4,
        1, 2, 4,
        2, 3, 4,
        3, 0, 4
    };
    // Vertices for a unit cube centered at the origin
    const float verticesCube[] = {
        // Front face
        -0.5f, -0.5f,  0.5f, // 0
         0.5f, -0.5f,  0.5f, // 1
         0.5f,  0.5f,  0.5f, // 2
        -0.5f,  0.5f,  0.5f, // 3

        // Back face
        -0.5f, -0.5f, -0.5f, // 4
         0.5f, -0.5f, -0.5f, // 5
         0.5f,  0.5f, -0.5f, // 6
        -0.5f,  0.5f, -0.5f, // 7
    };

    // Indices for a unit cube (using triangles)
    const unsigned int indicesCube[] = {
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Right face
        1, 5, 6,
        6, 2, 1,

        // Back face
        5, 4, 7,
        7, 6, 5,

        // Left face
        4, 0, 3,
        3, 7, 4,

        // Bottom face
        0, 1, 5,
        5, 4, 0,

        // Top face
        3, 2, 6,
        6, 7, 3
    };
    // Vertices for a unit 3D rectangular prism centered at the origin
    const float verticesrec[] = {
        // Front face
        -1.5f, -0.5f,  0.5f, // 0: Bottom-left
         1.5f, -0.5f,  0.5f, // 1: Bottom-right
         1.5f,  0.5f,  0.5f, // 2: Top-right
        -1.5f,  0.5f,  0.5f, // 3: Top-left

        // Back face
        -1.5f, -0.5f, -0.5f, // 4: Bottom-left
         1.5f, -0.5f, -0.5f, // 5: Bottom-right
         1.5f,  0.5f, -0.5f, // 6: Top-right
        -1.5f,  0.5f, -0.5f  // 7: Top-left
    };

    // Indices for the rectangular prism (12 triangles, 6 faces)
    const unsigned int indicesrec[] = {
        // Front face
        0, 1, 2,
        2, 3, 0,

        // Right face
        1, 5, 6,
        6, 2, 1,

        // Back face
        5, 4, 7,
        7, 6, 5,

        // Left face
        4, 0, 3,
        3, 7, 4,

        // Bottom face
        0, 1, 5,
        5, 4, 0,

        // Top face
        3, 2, 6,
        6, 7, 3
    };

    // CREATION OF FRAME BUFFER
    //unsigned int fbo;
    //glGenFramebuffers(1, &fbo);
    //glBindFramebuffer(GL_FRAMEBUFFER, fbo);


    // Generating buffers for Arrays and Index buffers
    VertexArray vapyramid;
    VertexBuffer vbpyramid(verticespyramid, sizeof(verticespyramid));
    VertexBufferLayout layout;
    layout.Push<float>(3);
    vapyramid.AddBuffer(vbpyramid, layout);
    IndexBuffer ibpyramid(indicespyramid, sizeof(indicespyramid));

    VertexArray vaCube;
    VertexBuffer vbCube(verticesCube, sizeof(verticesCube));
    vaCube.AddBuffer(vbCube, layout);
    IndexBuffer ibCube(indicesCube, sizeof(indicesCube));

    VertexArray varec;
    VertexBuffer vbrec(verticesrec, sizeof(verticesrec));
    varec.AddBuffer(vbrec, layout);
    IndexBuffer ibrec(indicesrec, sizeof(indicesrec));

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark;
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glm::vec4 objectColor = glm::vec4(0.8f, 0.3f, 0.02f, 1.0f); 
    glm::vec4 objectColorC = glm::vec4(0.8f, 0.3f, 0.02f, 1.0f);
    glm::vec4 objectColorR = glm::vec4(0.8f, 0.3f, 0.02f, 1.0f);

    glm::mat4 m = glm::mat4(1.0f);
    glm::mat4 m1 = glm::mat4(1.0f);
    glm::mat4 m2 = glm::mat4(1.0f);
    // render loop
    bool drawTriangle = false;
    bool drawCube = false;
    bool drawrec = false;
    float rotateAngle = m[3][0]; // Initial rotation angle
    float rotateAngleC = m1[3][0]; // Initial rotation angle
    float rotateAngleR = m2[3][0]; // Initial rotation angle

    float scaleValue = 1.0f; // Initial scale value
    float scaleValueC = 1.0f; // Initial scale value
    float scaleValueR = 1.0f; // Initial scale value
    //std::string str("C:\\Users\\deft\\Documents\\Classes\\Comp490\\OpenVR-3DModelerTool\\FinalBaseMesh.obj");
    //std::string str("C:\\Users\\deft\\Documents\\Classes\\Comp490\\OpenVR-3DModelerTool\\sphere.fbx");
    //
    //Model model(string("C:\\Users\\deft\\Documents\\Classes\\Comp490\\OpenVR-3DModelerTool\\FinalBaseMesh.obj"), false);
    //Model model(string("C:\\Users\\deft\\Documents\\Classes\\Comp490\\OpenVR-3DModelerTool\\sphere.fbx"), false);

    std::vector<Model> models;
    std::vector<Mesh> meshes;
    std::vector<Material> materials;
    models.push_back(Model("C:\\Users\\deft\\Documents\\Classes\\Comp490\\OpenVR-3DModelerTool\\FinalBaseMesh.obj", false));
    models.push_back(Model("C:\\Users\\deft\\Documents\\Classes\\Comp490\\OpenVR-3DModelerTool\\sphere.fbx", false));
    

    Scene scene(models, meshes, materials, camera, shader);
    scene.draw();

    //unsigned int fbo;
    //glBindFramebuffer(GL_FRAMEBUFFER, 0);

    while (!glfwWindowShouldClose(window)) {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        processInput(window);
        
        ImGui::Begin("Tool");
        ImGui::Text("Settings");
        ImGui::Text("Draw Objects");

        //index_pos = vertex_size - 1;
        scene.Render();
        scene.draw();

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
    /*
    while (!glfwWindowShouldClose(window)) {
        


        processInput(window);


        index_pos = vertex_size - 1;
        // Use the shader program
        shader.Bind();
        // Generating buffers for Arrays and Index buffers
        VertexArray va;
        VertexBufferLayout layout;
        layout.Push<float>(3);
        VertexBuffer vb(&vertices[0], sizeof(GLfloat) * 3 * vertex_size);
        va.AddBuffer(vb, layout);
        IndexBuffer ib(&indices[0], sizeof(GLuint) * 2 * total_edges);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.SetBackgroundClr(0.2f, 0.3f, 0.3f, 1.0f);

        camera.OnRender();

        // View matrix: Position the camera
        //glm::mat4 view = glm::lookAt(camera, cameraPos + cameraFront, cameraUp);
        shader.SetUniformMat4f("view", camera.ViewMatrix());
        // Projection matrix setup
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        shader.SetUniformMat4f("proj", projection);
        ImGui::Text("Draw Objects");

        ImGui::Checkbox("Draw Pyramid", &drawTriangle);
        if (drawTriangle)
        {
            ImGui::ColorEdit4("Pyramid: Color", glm::value_ptr(objectColor));
            m = glm::translate(glm::mat4(1.0f), glm::vec3(m[3][0], m[3][1], m[3][2]));  // Update the translation matrix
            m = glm::rotate(m, glm::radians(rotateAngle), glm::vec3(0.0f, 1.0f, 0.0f));  // Update the rotation matrix
            m = glm::scale(m, glm::vec3(scaleValue));  // Update the scale matrix

            shader.SetUniformMat4f("model", m);
            shader.SetUniform4f("U_Color", objectColor.r, objectColor.g, objectColor.b, objectColor.a);
            ImGui::SliderFloat("Pyramid: Scale", &scaleValue, 0.1f, 10.0f);

            ImGui::SliderFloat("Pyramid: Rotate", &rotateAngle, -180.0f, 180.0f);

            // ImGui slider for translation in X-axis
            ImGui::SliderFloat("Pyramid: Translate X", &m[3][0], -10.0f, 20.0f);

            // ImGui slider for translation in Y-axis
            ImGui::SliderFloat("Pyramid: Translate Y", &m[3][1], -0.50f, 20.0f);

            // ImGui slider for translation in Z-axis
            ImGui::SliderFloat("Pyramid: Translate Z", &m[3][2], -10.0f, 10.0f);


            renderer.Draw1(vapyramid, ibpyramid, shader);
        }
        ImGui::Checkbox("Draw Cube", &drawCube);
        if (drawCube)
        {
            ImGui::ColorEdit4("Cube: Color", glm::value_ptr(objectColorC));
            m1 = glm::translate(glm::mat4(1.0f), glm::vec3(m1[3][0], m1[3][1], m1[3][2]));  // Update the translation matrix
            m1 = glm::rotate(m1, glm::radians(rotateAngleC), glm::vec3(0.0f, 1.0f, 0.0f));  // Update the rotation matrix
            m1 = glm::scale(m1, glm::vec3(scaleValueC));  // Update the scale matrix

            shader.SetUniformMat4f("model", m1);
            shader.SetUniform4f("U_Color", objectColorC.r, objectColorC.g, objectColorC.b, objectColorC.a);
            ImGui::SliderFloat("Cube: Scale", &scaleValueC, 0.1f, 10.0f);

            ImGui::SliderFloat("Cube: Rotate", &rotateAngleC, -180.0f, 180.0f);
            // ImGui slider for translation in X-axis
            ImGui::SliderFloat("Cube: Translate X", &m1[3][0], -10.0f, 20.0f);

            // ImGui slider for translation in Y-axis
            ImGui::SliderFloat("Cube: Translate Y", &m1[3][1], -0.50f, 20.0f);

            // ImGui slider for translation in Z-axis
            ImGui::SliderFloat("Cube: Translate Z", &m1[3][2], -10.0f, 10.0f);
            renderer.Draw1(vaCube, ibCube, shader);
        }
        ImGui::Checkbox("Draw rec", &drawrec);
        if (drawrec)
        {
            ImGui::ColorEdit4("Rectangular: Color", glm::value_ptr(objectColorR));
            m2 = glm::translate(glm::mat4(1.0f), glm::vec3(m2[3][0], m2[3][1], m2[3][2]));  // Update the translation matrix
            m2 = glm::rotate(m2, glm::radians(rotateAngleR), glm::vec3(0.0f, 1.0f, 0.0f));  // Update the rotation matrix
            m2 = glm::scale(m2, glm::vec3(scaleValueR));  // Update the scale matrix

            shader.SetUniformMat4f("model", m2);
            shader.SetUniform4f("U_Color", objectColorR.r, objectColorR.g, objectColorR.b, objectColorR.a);
            // ImGui slider for translation in X-axis
            ImGui::SliderFloat("Rectangular: Scale", &scaleValueR, 0.1f, 12.0f);

            ImGui::SliderFloat("Rectangular: Rotate", &rotateAngleR, -180.0f, 180.0f);

            ImGui::SliderFloat("Rectangular: Translate X", &m2[3][0], -10.0f, 20.0f);

            // ImGui slider for translation in Y-axis
            ImGui::SliderFloat("Rectangular: Translate Y", &m2[3][1], -0.50f, 20.0f);

            // ImGui slider for translation in Z-axis
            ImGui::SliderFloat("Rectangular: Translate Z", &m2[3][2], -10.0f, 10.0f);
            renderer.Draw1(varec, ibrec, shader);
        }


        //model.Draw(shader);
        //scene.draw();
     
        
        if (ImGui::Button("Close Application")) {
            //Action to close the application
            return 0;
        }


        ImGui::End();

       // vertex_size = vertices.size() / 3;
      //  total_edges += 2;

        //grid
        glm::mat4 grid = glm::mat4(1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        grid = glm::translate(grid, glm::vec3(-50.0f, -0.5f, -100.0f));
        shader.SetUniformMat4f("model", grid);
        shader.SetUniform4f("U_Color", 1.0f, 1.0f, 1.0f, 0.1f);
        glLineWidth(5.0f);
        //renderer.Draw(vaGrid, ibGrid, shader);
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        glfwPollEvents();
    }
    */

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

    // Gets the position of the vertex at the mouse position
  /* if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        glfwGetCursorPos(window, &lastMouseX, &lastMouseY);


        addVertex({ (unsigned int)index_pos, (float)(lastMouseX / (1920/2)+1.0f),  (float)(lastMouseY/(1080/2)+1.0f), 0, {(unsigned int)(index_pos + 1)}}, vertices, indices);
        index_pos++;
    }*/
   /*if (button == GLFW_MOUSE_BUTTON_RIGHT)
   {
       double lastMouseX, lastMouseY;
       glfwGetCursorPos(window, &lastMouseX, &lastMouseY);

       float closestDistance = std::numeric_limits<float>::max();
       unsigned int closestVertexIndex = 0;

       for (const Vertex& vertex : verticese) {
           float distance = calculateDistance(
               lastMouseX, lastMouseY,

               vertex.x, vertex.y

           );

           if (distance < closestDistance) {
               closestDistance = distance;
               closestVertexIndex = vertex.index;
           }
       }

       removeVertex(closestVertexIndex);
   }
   */
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
    //camera interactions
    //float cameraSpeed = 0.05f;
    //if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    //    cameraPos += cameraSpeed * cameraFront;
    //if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    //    cameraPos -= cameraSpeed * cameraFront;
    //if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    //    cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    //if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    //    cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
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

void addVertex(Vertex v, std::vector<GLfloat>& vertices, std::vector<GLuint>& indices)
{
    std::cout << "index: " << v.index;
    std::cout << " x: " << v.x;
    std::cout << ", y: " << v.y;
    std::cout << ", z: " << v.z;
    vertices.push_back(v.x);
    vertices.push_back(v.y);
    vertices.push_back(v.z);

    int i;
    int edge_size = v.edges.size();
    std::cout << " edges: ";
    for (i = 0; i < edge_size; i++) {
        indices.push_back(v.index);
        indices.push_back(v.edges[i]);
        std::cout << v.index << ", ";
        std::cout << v.edges[i] << ", ";
    }
    std::cout << std::endl;
}

/*
void removeVertex(unsigned int index) {
    std::cout << "Trying to remove vertex with index: " << index << std::endl; 

    auto it = std::find_if(verticese.begin(), verticese.end(), [index](const Vertex& v) {
        return v.index == index;

        });

    if (it != verticese.end()) {
        std::cout << "Removing Vertex: " << it->index << std::endl;

        verticese.erase(it);

        for (auto& vertex : verticese) {
            auto& edges = vertex.edges;
            edges.erase(std::remove(edges.begin(), edges.end(), index), edges.end());
        }

        vertices.clear();
        indices.clear();

        for (Vertex v : verticese) {
            addVertex(v, vertices, indices);
        }
    }

    else {
        std::cerr << "Vertex with index " << index << " not found! " << std::endl;

    }

}
void createData(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices)
{
}*/
// function to create grid
void createGridData(std::vector<GLfloat>& gridVertices, std::vector<GLuint>& gridIndices, int gridSizeX, int gridSizeZ, float gridSpacing) {
    for (unsigned int i = 0; i < gridSizeX; ++i) {
        for (unsigned int j = 0; j < gridSizeZ; ++j) {
            float x = i * gridSpacing;
            float y = 0.0f;
            float z = j * gridSpacing;

            // Add vertices for each corner of the square
            gridVertices.push_back(x);
            gridVertices.push_back(y);
            gridVertices.push_back(z);

            gridVertices.push_back(x + gridSpacing);
            gridVertices.push_back(y);
            gridVertices.push_back(z);

            gridVertices.push_back(x + gridSpacing);
            gridVertices.push_back(y);
            gridVertices.push_back(z + gridSpacing);

            gridVertices.push_back(x);
            gridVertices.push_back(y);
            gridVertices.push_back(z + gridSpacing);

            // Add indices for the square
            GLuint baseIndex = static_cast<GLuint>(gridVertices.size() / 3) - 4;
            gridIndices.push_back(baseIndex);
            gridIndices.push_back(baseIndex + 1);
            gridIndices.push_back(baseIndex + 2);
            gridIndices.push_back(baseIndex + 2);
            gridIndices.push_back(baseIndex + 3);
            gridIndices.push_back(baseIndex);
        }
    }
}

//function for creating cylinder - not rendering properly though. works on previous versions
void createCylinder(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, float radius, float height, int sides) {
    float angleIncrement = glm::two_pi<float>() / static_cast<float>(sides);

    // Top and bottom vertices
    vertices.push_back(0.0f);  // Center of the bottom circle
    vertices.push_back(-height / 2.0f);
    vertices.push_back(0.0f);

    vertices.push_back(0.0f);  // Center of the top circle
    vertices.push_back(height / 2.0f);
    vertices.push_back(0.0f);

    // Vertices for the sides
    for (int i = 0; i < sides; ++i) {
        float x = radius * cos(angleIncrement * i);
        float z = radius * sin(angleIncrement * i);

        // Bottom circle vertices
        vertices.push_back(x);
        vertices.push_back(-height / 2.0f);
        vertices.push_back(z);

        // Top circle vertices
        vertices.push_back(x);
        vertices.push_back(height / 2.0f);
        vertices.push_back(z);
    }

    // Indices for the sides
    for (int i = 0; i < sides; ++i) {
        // Side triangles
        indices.push_back(0);                   // Center of bottom circle
        indices.push_back(2 * i + 2);           // Next point on bottom circle
        indices.push_back(2 * ((i + 1) % sides) + 2); // Next point on bottom circle in next segment

        indices.push_back(1);                   // Center of top circle
        indices.push_back(2 * i + 3);           // Next point on top circle in current segment
        indices.push_back(2 * ((i + 1) % sides) + 3); // Next point on top circle in next segment
    }
}

//function for Sphere. also doesnt render on current version.
void createSphere(std::vector<GLfloat>& vertices, std::vector<GLuint>& indices, float radius, int stacks, int slices) {
    // Iterate through the stacks (vertical divisions)
    for (int i = 0; i <= stacks; ++i) {
        // Calculate the vertical angle (phi) for the current stack
        float phi = glm::pi<float>() * static_cast<float>(i) / static_cast<float>(stacks);

        // Iterate through the slices (horizontal divisions)
        for (int j = 0; j <= slices; ++j) {
            // Calculate the horizontal angle (theta) for the current slice
            float theta = 2.0f * glm::pi<float>() * static_cast<float>(j) / static_cast<float>(slices);

            // Convert spherical coordinates to Cartesian coordinates
            float x = cos(theta) * sin(phi);
            float y = cos(phi);
            float z = sin(theta) * sin(phi);

            // Add the Cartesian coordinates of the current vertex to the vertices vector
            vertices.push_back(radius * x); // x coordinate
            vertices.push_back(radius * y); // y coordinate
            vertices.push_back(radius * z); // z coordinate

            // Create triangles by connecting vertices in a winding order
            if (i != stacks) {
                indices.push_back((i * (slices + 1)) + j);
                indices.push_back(((i + 1) * (slices + 1)) + j);
                indices.push_back((i * (slices + 1)) + j + 1);

                indices.push_back(((i + 1) * (slices + 1)) + j);
                indices.push_back(((i + 1) * (slices + 1)) + j + 1);
                indices.push_back((i * (slices + 1)) + j + 1);
            }
        }
    }
}