#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Shader.h"
#include "GLLoader.h"
#include "VertexBuffer.h"
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "Renderer.h"
#include <iostream>

#include <vec3.hpp> // glm::vec3
#include <vec4.hpp> // glm::vec4
#include <mat4x4.hpp> // glm::mat4
#include <ext/matrix_transform.hpp> // glm::translate, glm::rotate, glm::scale
#include <ext/matrix_clip_space.hpp> // glm::perspective
#include <ext/scalar_constants.hpp> // glm::pi

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void wireFrame(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenVR_3DModeler", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    {

        // set up vertex data (and buffer(s)) and configure vertex attributes
        // ------------------------------------------------------------------
        float vertices[] = {
            0.5f, 0.5f, 0.0f,   // top right
            0.5f, 0.0f, 0.0f,   // bottom right
            0.0f, 0.0f, 0.0f,   // bottom left
            0.0f, 0.5f, 0.0f,   // top left

            0.0f, 0.0f, 0.0f,   // top right
            0.0f, -0.5f, 0.0f,  // bottom right
            -0.5f, -0.5f, 0.0f, // bottom left
            -0.5f, 0.0f, 0.0f,  // top left

            0.5f, 0.0f, 0.0f,   // top right
            0.5f, -0.5f, 0.0f,  // bottom right
            0.0f, -0.5f, 0.0f,  // bottom left
            0.0f, 0.0f, 0.0f,   // top left

            0.0f, 0.5f, 0.0f,   // top right
            0.0f, 0.0f, 0.0f,   // bottom right
            -0.5f, 0.0f, 0.0f,  // bottom left
            -0.5f, 0.5f, 0.0f  // top left

        };
        unsigned int indices[] = {  // note that we start from 0!
              
            0, 1, 3,    // first triangle
            1, 2, 3,    // second triangle

            4, 5, 7,    // first triangle
            5, 6, 7,    // second triangle

            8, 9, 11,   // first triangle
            9, 10, 11,   // second triangle

            12, 13, 15,  // first triangle
            13, 14, 15   // second triangle

        };

        // Generating buffers for Arrays and Indix buffers
        VertexArray va;
        VertexBuffer vb(vertices, sizeof(vertices));
        VertexBufferLayout layout;
        layout.Push<float>(3);
        va.AddBuffer(vb, layout);
        IndexBuffer ib(indices, sizeof(indices));

        Shader shader("shaders/RainbowShader.shader");
        shader.Bind();

        Renderer renderer;


        float r = 0, g = 0, b = 0;
        float a = 0.001f, bb = 0.002f, c = 0.003f;
        shader.SetUniform4f("U_Color", r, g, b, 1.0f);
        renderer.SetBackgroundClr(0.2f, 0.3f, 0.3f, 1.0f);


        // render loop
        // -----------
        while (!glfwWindowShouldClose(window))
        {
            renderer.Clear();
            // input
            // -----
            processInput(window);
            wireFrame(window);

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

            // render
            renderer.Draw(va, ib, shader);

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        // optional: de-allocate all resources once they've outlived their purpose:
        // ------------------------------------------------------------------------


    }
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}



void wireFrame(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
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

