#include <GLFW/glfw3.h>

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    float a = -0.5f;
    float b = 0.5f;
    bool rev = false;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);

        if (!rev)
        {
            a += 0.0025f;
            b -= 0.0025f;
            if (a > 0)
            {
                rev = true;
            }
        }
        else
        {
            a -= 0.0025f;
            b += 0.0025f;
            if (a < -0.5f)
            {
                rev = false;
            }
        }
        glVertex2f(a, -0.5f);
        glVertex2f(0.0f, 0.5f);
        glVertex2f(b, -0.5f);
        glEnd();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}