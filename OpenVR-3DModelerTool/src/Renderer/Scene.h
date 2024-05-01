#pragma once
#ifndef SCENE_H
#define SCENE_H
#include <vector>
#include "Model.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"
#include "../OpenVR/VRCamera.h"
#include "../OpenGL/Renderer.h"
//#include <bullet/btBulletDynamicsCommon.h>

class Scene {
public:
	std::vector<Model> models;
	Camera& camera;
    VRCamera vrCam;

    Scene(std::vector<Model> models, Camera& camera, Shader& shader) :
        models(models),
        camera(camera),
        m_shader(shader),
        vbGrid(NULL, 0),
        ibGrid(NULL, 0),
        vaGrid(),
        layoutGrid()
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);
        init();
	}
	~Scene() {};

	void draw() {
		// Create a Shader object and load shaders from files
		for (Model& model : models) {
            //model.Transform((vrCam.position));
            model.shader.Bind();
			model.Draw();
		}
	}

	void Render() {
        glEnable(GL_DEPTH_TEST);

        m_shader.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.SetBackgroundClr(0.2f, 0.3f, 0.3f, 1.0f);
        // Camera render
        camera.OnRender();
        // View matrix: Position the camera
        m_shader.SetUniformMat4f("view", camera.ViewMatrix());
        // Projection matrix setup
        glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)(camera.scrWidth / camera.scrHeight), 0.1f, 100.0f);
        m_shader.SetUniformMat4f("proj", projection);

        // Grid
        glm::mat4 grid = glm::mat4(1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        grid = glm::translate(grid, glm::vec3(-50.0f, -0.5f, -100.0f));
        m_shader.SetUniformMat4f("model", grid);
        m_shader.SetUniform4f("U_Color", 1.0f, 1.0f, 1.0f, 0.1f);
        glLineWidth(5.0f);
        renderer.Draw(vaGrid, ibGrid, m_shader);

        for (Model& model : models) {
            model.shader.Bind();
            model.shader.SetMat4("view", camera.ViewMatrix());
            model.shader.SetMat4("projection", projection);
            model.shader.SetVec3("viewPos", camera.m_Position);
            model.Draw();
        }
	}

    void RenderStereoTargets() {
        RenderTarget(vr::Eye_Left, vrCam.GetFrameBufferDesc(vr::Eye_Left));
        RenderTarget(vr::Eye_Right, vrCam.GetFrameBufferDesc(vr::Eye_Right));
        vrCam.RenderFrame();
    }

    void RenderTarget(vr::Hmd_Eye nEye, VRCamera::FramebufferDesc eyeDesc) {
        glBindFramebuffer(GL_FRAMEBUFFER, eyeDesc.m_nRenderFramebufferId);
        glViewport(0, 0, vrCam.GetRenderWidth(), vrCam.GetRenderHeight());
        Render(nEye);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDisable(GL_MULTISAMPLE);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, eyeDesc.m_nRenderFramebufferId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, eyeDesc.m_nResolveFramebufferId);

        glBlitFramebuffer(0, 0, vrCam.GetRenderWidth(), vrCam.GetRenderHeight(), 0, 0, vrCam.GetRenderWidth(), vrCam.GetRenderHeight(),
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glEnable(GL_MULTISAMPLE);
    }

    void Render(vr::Hmd_Eye nEye) {
        glEnable(GL_DEPTH_TEST);
        // Use the shader program
        m_shader.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.SetBackgroundClr(0.2f, 0.3f, 0.3f, 1.0f);
        // Camera render
        m_shader.SetUniformMat4f("view", glm::mat4(1.0f));

        // Projection matrix setup
        glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), float(vrCam.GetRenderWidth() / vrCam.GetRenderHeight()), vrCam.m_fNearClip, vrCam.m_fFarClip);
        m_shader.SetUniformMat4f("proj", vrCam.GetCurrentViewProjectionMatrix(nEye));

        // Grid
        glm::mat4 grid = glm::mat4(1.0f);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        grid = glm::translate(grid, glm::vec3(-50.0f, -0.5f, -100.0f));
        m_shader.SetUniformMat4f("model", grid);
        m_shader.SetUniform4f("U_Color", 1.0f, 1.0f, 1.0f, 0.1f);
        glLineWidth(5.0f);
        renderer.Draw(vaGrid, ibGrid, m_shader);
        for (Model& model : models) {
            model.shader.Bind();
            model.shader.SetMat4("view", glm::mat4(1.0f));
            model.shader.SetMat4("projection", vrCam.GetCurrentViewProjectionMatrix(nEye));
            model.shader.SetVec3("viewPos", vrCam.pos);
            model.Draw();
        }
    }
		
private:
	Shader& m_shader;
    Renderer renderer;  

    unsigned int gridSizeX = 450; // Number of squares along the X-axis
    unsigned int gridSizeZ = 250; // Number of squares along the Z-axis
    float gridSpacing = 1.0f;    // Spacing between grid squares

    std::vector<GLfloat> gridVertices;
    std::vector<GLuint> gridIndices;
    VertexBuffer vbGrid;
    IndexBuffer ibGrid;
    VertexArray vaGrid;
    VertexBufferLayout layoutGrid;


    void init() {
        createGridData(gridVertices, gridIndices, gridSizeX, gridSizeZ, gridSpacing);
        vbGrid = *new VertexBuffer(&gridVertices[0], sizeof(GLfloat) * gridVertices.size());
        ibGrid = *new IndexBuffer(&gridIndices[0], gridIndices.size());
        vaGrid = *new VertexArray();
        layoutGrid = *new VertexBufferLayout();
        layoutGrid.Push<float>(3);
        vaGrid.AddBuffer(vbGrid, layoutGrid);
    };


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
};

#endif