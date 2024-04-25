#pragma once
#ifndef SCENE_H
#define SCENE_H
#include <vector>
#include "Model.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"

#include "../OpenGL/Renderer.h"

class Scene {
public:
	std::vector<Model> models;
	std::vector<Mesh> meshes;
	std::vector<Material> materials;
	Camera& camera;
    vr::EVRInitError err = vr::VRInitError_None;
    vr::IVRSystem* m_pHMD;

    Scene(std::vector<Model> models, std::vector<Mesh> meshes, std::vector<Material> materials, Camera& camera, Shader& shader) :
        models(models),
        meshes(meshes),
        materials(materials),
        camera(camera),
        m_shader(shader),
        vbGrid(NULL, 0),
        ibGrid(NULL, 0),
        vaGrid(),
        layoutGrid()
    {
        m_pHMD = vr::VR_Init(&err, vr::VRApplication_Scene);
        init();
	}
	~Scene() {};

	void draw() {
		// Create a Shader object and load shaders from files
		for (Model& model : models) {
			model.Draw(m_shader);
		}
	}

	void Render() {
        // Use the shader program
        m_shader.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.SetBackgroundClr(0.2f, 0.3f, 0.3f, 1.0f);
        // Camera render
        camera.OnRender();
        // View matrix: Position the camera
        m_shader.SetUniformMat4f("view", camera.ViewMatrix());
        // Projection matrix setup
        glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)1920 / 1080, 0.1f, 100.0f);
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
	}

    void RenderFrame() {
        //RenderControllerAxes();
        RenderStereoTargets();
        //RenderCompanionWindow();

        vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
        vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

        UpdateHMDMatrixPose();
    }

    void RenderStereoTargets() {
        //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        //glEnable(GL_MULTISAMPLE);

        // Left Eye
        glBindFramebuffer(GL_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
        glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
        Render(vr::Eye_Left);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDisable(GL_MULTISAMPLE);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, leftEyeDesc.m_nRenderFramebufferId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, leftEyeDesc.m_nResolveFramebufferId);

        glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight,
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

        glEnable(GL_MULTISAMPLE);

        // Right Eye
        glBindFramebuffer(GL_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
        glViewport(0, 0, m_nRenderWidth, m_nRenderHeight);
        //Render();
        Render(vr::Eye_Right);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        glDisable(GL_MULTISAMPLE);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, rightEyeDesc.m_nRenderFramebufferId);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, rightEyeDesc.m_nResolveFramebufferId);

        glBlitFramebuffer(0, 0, m_nRenderWidth, m_nRenderHeight, 0, 0, m_nRenderWidth, m_nRenderHeight,
            GL_COLOR_BUFFER_BIT,
            GL_LINEAR);

        glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    }

    void Render(vr::Hmd_Eye nEye) {
        //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //glEnable(GL_DEPTH_TEST);

        // Use the shader program
        m_shader.Bind();
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        renderer.SetBackgroundClr(0.2f, 0.3f, 0.3f, 1.0f);
        // Camera render
        //camera.OnRender();
        // View matrix: Position the camera
        //m_shader.SetUniformMat4f("view", camera.ViewMatrix());
        // View matrix: Position the camera
        m_shader.SetUniformMat4f("view", GetViewMatrix(nEye));

        // Get the view matrix from OpenVR

        // Projection matrix setup
        glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), float(m_nRenderWidth / m_nRenderWidth), m_fNearClip, m_fFarClip);
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

        bool bIsInputAvailable = m_pHMD->IsInputAvailable();

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

        //SetupStereoRenderTargets
        m_pHMD->GetRecommendedRenderTargetSize(&m_nRenderWidth, &m_nRenderHeight);
        //SetupCamera
        SetupCameras();
        CreateFrameBuffer(m_nRenderHeight, m_nRenderHeight, leftEyeDesc);
        CreateFrameBuffer(m_nRenderHeight, m_nRenderHeight, rightEyeDesc);
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


    struct FramebufferDesc
    {
        GLuint m_nDepthBufferId;
        GLuint m_nRenderTextureId;
        GLuint m_nRenderFramebufferId;
        GLuint m_nResolveTextureId;
        GLuint m_nResolveFramebufferId;
    };
    FramebufferDesc leftEyeDesc;
    FramebufferDesc rightEyeDesc;
    uint32_t m_nRenderWidth;
    uint32_t m_nRenderHeight;

    //void RenderScene(vr::Hmd_Eye nEye) {
    //
    //}
    //vr::IVRSystem* m_pHMD;
    glm::mat4 m_mat4ProjectionLeft;
    glm::mat4 m_mat4ProjectionRight;
    glm::mat4 m_mat4eyePosLeft;
    glm::mat4 m_mat4eyePosRight;

    vr::TrackedDevicePose_t m_rTrackedDevicePose[vr::k_unMaxTrackedDeviceCount];
    int m_iValidPoseCount;
    std::string m_strPoseClasses;
    glm::mat4 m_rmat4DevicePose[vr::k_unMaxTrackedDeviceCount];
    char m_rDevClassChar[vr::k_unMaxTrackedDeviceCount];
    glm::mat4 m_mat4HMDPose;

    void UpdateHMDMatrixPose()
    {
        if (!m_pHMD)
            return;

        vr::VRCompositor()->WaitGetPoses(m_rTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, NULL, 0);

        m_iValidPoseCount = 0;
        m_strPoseClasses = "";
        for (int nDevice = 0; nDevice < vr::k_unMaxTrackedDeviceCount; ++nDevice)
        {
            if (m_rTrackedDevicePose[nDevice].bPoseIsValid)
            {
                m_iValidPoseCount++;
                m_rmat4DevicePose[nDevice] = ConvertSteamVRMatrixToMatrix4(m_rTrackedDevicePose[nDevice].mDeviceToAbsoluteTracking);
                if (m_rDevClassChar[nDevice] == 0)
                {
                    switch (m_pHMD->GetTrackedDeviceClass(nDevice))
                    {
                    case vr::TrackedDeviceClass_Controller:        m_rDevClassChar[nDevice] = 'C'; break;
                    case vr::TrackedDeviceClass_HMD:               m_rDevClassChar[nDevice] = 'H'; break;
                    case vr::TrackedDeviceClass_Invalid:           m_rDevClassChar[nDevice] = 'I'; break;
                    case vr::TrackedDeviceClass_GenericTracker:    m_rDevClassChar[nDevice] = 'G'; break;
                    case vr::TrackedDeviceClass_TrackingReference: m_rDevClassChar[nDevice] = 'T'; break;
                    default:                                       m_rDevClassChar[nDevice] = '?'; break;
                    }
                }
                m_strPoseClasses += m_rDevClassChar[nDevice];
            }
        }

        if (m_rTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
        {
            m_mat4HMDPose = m_rmat4DevicePose[vr::k_unTrackedDeviceIndex_Hmd];
            m_mat4HMDPose = glm::inverse(m_mat4HMDPose);
        }
    }

    glm::mat4 ConvertSteamVRMatrixToMatrix4(const vr::HmdMatrix34_t& matPose)
    {
        glm::mat4 matrixObj(
            matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
            matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
            matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
            matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
        );
        return matrixObj;
    }

    bool CreateFrameBuffer(int nWidth, int nHeight, FramebufferDesc& framebufferDesc)
    {
        glGenFramebuffers(1, &framebufferDesc.m_nRenderFramebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nRenderFramebufferId);

        glGenRenderbuffers(1, &framebufferDesc.m_nDepthBufferId);
        glBindRenderbuffer(GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, 4, GL_DEPTH_COMPONENT, nWidth, nHeight);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, framebufferDesc.m_nDepthBufferId);

        glGenTextures(1, &framebufferDesc.m_nRenderTextureId);
        glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId);
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, 4, GL_RGBA8, nWidth, nHeight, true);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D_MULTISAMPLE, framebufferDesc.m_nRenderTextureId, 0);

        glGenFramebuffers(1, &framebufferDesc.m_nResolveFramebufferId);
        glBindFramebuffer(GL_FRAMEBUFFER, framebufferDesc.m_nResolveFramebufferId);

        glGenTextures(1, &framebufferDesc.m_nResolveTextureId);
        glBindTexture(GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, nWidth, nHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebufferDesc.m_nResolveTextureId, 0);

        // check FBO status
        GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE)
        {
            return false;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return true;
    }

    void SetupCameras()
    {
        m_mat4ProjectionLeft = GetHMDMatrixProjectionEye(vr::Eye_Left);
        m_mat4ProjectionRight = GetHMDMatrixProjectionEye(vr::Eye_Right);
        m_mat4eyePosLeft = GetHMDMatrixPoseEye(vr::Eye_Left);
        m_mat4eyePosRight = GetHMDMatrixPoseEye(vr::Eye_Right);
    }

    float m_fNearClip = 0.1f;
    float m_fFarClip = 30.0f;

    glm::mat4 GetHMDMatrixProjectionEye(vr::Hmd_Eye nEye)
    {

        vr::HmdMatrix44_t mat = m_pHMD->GetProjectionMatrix(nEye, m_fNearClip, m_fFarClip);

        return glm::mat4(
            mat.m[0][0], mat.m[1][0], mat.m[2][0], mat.m[3][0],
            mat.m[0][1], mat.m[1][1], mat.m[2][1], mat.m[3][1],
            mat.m[0][2], mat.m[1][2], mat.m[2][2], mat.m[3][2],
            mat.m[0][3], mat.m[1][3], mat.m[2][3], mat.m[3][3]
        );
    }

    glm::mat4 GetHMDMatrixPoseEye(vr::Hmd_Eye nEye)
    {

        vr::HmdMatrix34_t matEyeRight = m_pHMD->GetEyeToHeadTransform(nEye);
        glm::mat4 matrixObj(
            matEyeRight.m[0][0], matEyeRight.m[1][0], matEyeRight.m[2][0], 0.0,
            matEyeRight.m[0][1], matEyeRight.m[1][1], matEyeRight.m[2][1], 0.0,
            matEyeRight.m[0][2], matEyeRight.m[1][2], matEyeRight.m[2][2], 0.0,
            matEyeRight.m[0][3], matEyeRight.m[1][3], matEyeRight.m[2][3], 1.0f
        );

        return glm::inverse(matrixObj);
        //return matrixObj;
    }

    glm::mat4 GetCurrentViewProjectionMatrix(vr::Hmd_Eye nEye)
    {
        glm::mat4 matMVP;
        if (nEye == vr::Eye_Left)
        {
            matMVP = m_mat4ProjectionLeft * m_mat4eyePosLeft * m_mat4HMDPose;
        }
        else if (nEye == vr::Eye_Right)
        {
            matMVP = m_mat4ProjectionRight * m_mat4eyePosRight * m_mat4HMDPose;
        }

        return matMVP;
    }

    glm::mat4 GetViewMatrix(vr::Hmd_Eye nEye) {

        // Get the eye-to-head transform matrix
        vr::HmdMatrix34_t eyeToHeadTransform = m_pHMD->GetEyeToHeadTransform(nEye);

        // Get the pose of the HMD (head) 
        vr::TrackedDevicePose_t trackedDevicePose;
        m_pHMD->GetDeviceToAbsoluteTrackingPose(vr::TrackingUniverseStanding, 0, &trackedDevicePose, 1);

        // Check if the HMD is connected and tracked
        if (trackedDevicePose.bDeviceIsConnected && trackedDevicePose.bPoseIsValid) {
            // Extract the head-to-world transform matrix (HMD pose)
            vr::HmdMatrix34_t headToWorldTransform = trackedDevicePose.mDeviceToAbsoluteTracking;

            // Inverse the head-to-world transform matrix to get world-to-head matrix
            glm::mat4 worldToHead = glm::mat4(
                headToWorldTransform.m[0][0], headToWorldTransform.m[1][0], headToWorldTransform.m[2][0], 0.0f,
                headToWorldTransform.m[0][1], headToWorldTransform.m[1][1], headToWorldTransform.m[2][1], 0.0f,
                headToWorldTransform.m[0][2], headToWorldTransform.m[1][2], headToWorldTransform.m[2][2], 0.0f,
                headToWorldTransform.m[0][3], headToWorldTransform.m[1][3], headToWorldTransform.m[2][3], 1.0f
            );

            // Combine the eye-to-head and world-to-head matrices to get the view matrix
            glm::mat4 eyeToWorld = glm::mat4(
                eyeToHeadTransform.m[0][0], eyeToHeadTransform.m[1][0], eyeToHeadTransform.m[2][0], 0.0f,
                eyeToHeadTransform.m[0][1], eyeToHeadTransform.m[1][1], eyeToHeadTransform.m[2][1], 0.0f,
                eyeToHeadTransform.m[0][2], eyeToHeadTransform.m[1][2], eyeToHeadTransform.m[2][2], 0.0f,
                eyeToHeadTransform.m[0][3], eyeToHeadTransform.m[1][3], eyeToHeadTransform.m[2][3], 1.0f
            );
            glm::mat4 eyeToHead = glm::inverse(eyeToWorld);

            // Apply translation to the view matrix
            glm::mat4 viewMatrix = eyeToHead * worldToHead;
            viewMatrix = glm::translate(viewMatrix, camera.m_Position);

            return glm::inverse(viewMatrix);
        }

    }
};

#endif