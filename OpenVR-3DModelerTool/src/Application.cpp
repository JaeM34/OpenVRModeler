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
int vertex_size;
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
    
    float r = 0, g = 0, b = 0;
    float a = 0.001f, bb = 0.002f, c = 0.003f;



    // render loop
    while (!glfwWindowShouldClose(window)) {
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
        processInput(window);


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.SetBackgroundClr(0.2f, 0.3f, 0.3f, 1.0f);

        // View matrix: Position the camera
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        shader.SetUniformMat4f("view", view);

        // Projection matrix setup
        glm::mat4 projection = glm::perspective(glm::radians(fov), (float)SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
        shader.SetUniformMat4f("proj", projection);

        // Model matrix: Rotate the pyramid
        glm::mat4 model = glm::mat4(1.0f);
        //model = glm::rotate(model, (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f)); // Pyramid spin
        shader.SetUniformMat4f("model", model);

        // RAINBOW SHADER LOGIC!!!
        r += a;
        g += bb;
        b += c;
        if (r > 1.0f || r < 0.0f)
        {
            a *= -1.0f;
        }
        if (g > 1.0f || g < 0.0f)
        {
            bb *= -1.0f;
        }
        if (b > 1.0f || b < 0.0f)
        {
            c *= -1.0f;
        }
        shader.SetUniform4f("U_Color", r, g, b, 1.0f);
        // Draw the pyramid
        renderer.Draw(va, ib, shader);
        vertex_size = vertices.size() / 3;
        total_edges += 2;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Clean up and terminate GLFW
    glfwTerminate();

    return 0;
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
    if (button == GLFW_MOUSE_BUTTON_LEFT)
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

void removeVertex(unsigned int index)
{
    int i;
    int remove_index;
    for (i = 0; i < vertex_size; i++)
    {
        if (verticese[i].index == index)
        {
            verticese.erase(verticese.begin()+i);
            return;
        }
    }
}