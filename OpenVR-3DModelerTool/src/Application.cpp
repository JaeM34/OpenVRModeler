#include "Shader.h"
#include "GLLoader.h"
#include "VertexBuffer.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "Renderer.h"
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
glm::vec3 cameraPos = glm::vec3(0.0f, 1.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
float cameraSpeed = 0.05f;
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
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // GLFW configuration
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Create a GLFW window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Pyramid", NULL, NULL);
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
                                     
    verticese.push_back({0,  -0.5f,  0.0f, -0.5f, {1, 4}});
    verticese.push_back({1,   0.5f,  0.0f, -0.5f, {2, 4}});
    verticese.push_back({2,   0.5f,  0.0f,  0.5f, {3, 4}});
    verticese.push_back({3,  -0.5f,  0.0f,  0.5f, {0, 4}});
                                     
    verticese.push_back({4,   0.0f,  1.0f,  0.0f, {}});
                                     
    verticese.push_back({5,  -0.5f,  2.0f, -0.5f, {6, 8, 4} });
    verticese.push_back({6,   0.5f,  2.0f, -0.5f, {7, 4} });
    verticese.push_back({7,   0.5f,  2.0f,  0.5f, {8, 4} });
    verticese.push_back({8,  -0.5f,  2.0f,  0.5f, {4  } });

    verticese.push_back({9,   -0.5f, 0.5f, -1.0f, {10, 4} });
    verticese.push_back({10,   0.5f, 0.5f, -0.5f, {11, 4} });
    verticese.push_back({11,   0.5f, 1.5f,  0.5f, {12, 4} });
    verticese.push_back({12,  -0.5f, 1.5f,  1.0f, { 9, 4} });

    total_edges = 0;
    int i;
    for (i = 0; i < vertex_size; i++) {
        total_edges += verticese[i].edges.size();
    }

    for (Vertex v : verticese)
    {
        addVertex(v, vertices, indices);
    }

    Renderer renderer;
    // Use the shader program
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    // Set up callback functions
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    // Set up mouse button callback function
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    unsigned int gridSizeX = 450; // Number of squares along the X-axis
    unsigned int gridSizeZ = 250; // Number of squares along the Z-axis
    float gridSpacing = 1.0f;    // Spacing between grid squares

    std::vector<GLfloat> gridVertices;
    std::vector<GLuint> gridIndices;
    createGridData(gridVertices, gridIndices, gridSizeX, gridSizeZ, gridSpacing);
    VertexBuffer vbGrid(&gridVertices[0], sizeof(GLfloat)* gridVertices.size());
    IndexBuffer ibGrid(&gridIndices[0], gridIndices.size());
    VertexArray vaGrid;
    VertexBufferLayout layoutGrid;
    layoutGrid.Push<float>(3);
    vaGrid.AddBuffer(vbGrid, layoutGrid);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    glm::vec4 objectColor = glm::vec4(0.8f, 0.3f, 0.02f, 1.0f); 

    // render loop
    while (!glfwWindowShouldClose(window)) {
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        processInput(window);

        ImGui::Begin("Tool");
        ImGui::Text("Color");
        ImGui::ColorEdit4("Color", glm::value_ptr(objectColor)); 
        ImGui::End();

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

        // View matrix: Position the camera
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        shader.SetUniformMat4f("view", view);

        // Projection matrix setup
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        shader.SetUniformMat4f("proj", projection);

        glm::mat4 m = glm::mat4(1.0f);
        shader.SetUniformMat4f("model", m);
        
        shader.SetUniform4f("U_Color", objectColor.r, objectColor.g, objectColor.b, objectColor.a);
        renderer.Draw(va, ib, shader);
        vertex_size = vertices.size() / 3;
        total_edges += 2;

        //grid
        glm::mat4 grid = glm::mat4(1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        grid = glm::translate(grid, glm::vec3(-50.0f, -0.5f, -100.0f));
        shader.SetUniformMat4f("model", grid);
        shader.SetUniform4f("U_Color", 1.0f, 1.0f, 1.0f, 0.1f);
        glLineWidth(5.0f);

        renderer.Draw(vaGrid, ibGrid, shader);
        
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

    // Gets the position of the vertex at the mouse position
   if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        glfwGetCursorPos(window, &lastMouseX, &lastMouseY);


        addVertex({ (unsigned int)index_pos, (float)(lastMouseX / (1920/2)+1.0f),  (float)(lastMouseY/(1080/2)+1.0f), 0, {(unsigned int)(index_pos + 1)}}, vertices, indices);
        index_pos++;
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

        glm::vec3 front;
        front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        front.y = sin(glm::radians(pitch));
        front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        cameraFront = glm::normalize(front);
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    //camera interactions
    float cameraSpeed = 0.05f;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

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


/*void removeVertex(unsigned int index) {
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
