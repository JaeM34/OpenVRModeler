#include "openvr.h"
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include "glad/glad.h"
#include <iostream>

class VRCamera {
public:
    float m_fNearClip = 0.1f;
    float m_fFarClip = 100.0f;
    glm::vec3 pos = glm::vec3(1.0f);
    float dist = 0.0f;
    float yaw = 0.0f;
    glm::vec3 position{0.0f};
    glm::vec4 orientation;
    glm::mat4 rotationMatrix;


    struct FramebufferDesc
    {
        GLuint m_nDepthBufferId;
        GLuint m_nRenderTextureId;
        GLuint m_nRenderFramebufferId;
        GLuint m_nResolveTextureId;
        GLuint m_nResolveFramebufferId;
    };

    VRCamera() {
        m_pHMD = vr::VR_Init(&err, vr::VRApplication_Scene);
        init_camera();
    }

    void RenderFrame() {

        vr::Texture_t leftEyeTexture = { (void*)(uintptr_t)leftEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Left, &leftEyeTexture);
        vr::Texture_t rightEyeTexture = { (void*)(uintptr_t)rightEyeDesc.m_nResolveTextureId, vr::TextureType_OpenGL, vr::ColorSpace_Gamma };
        vr::VRCompositor()->Submit(vr::Eye_Right, &rightEyeTexture);

        UpdateHMDMatrixPose();

        // Assuming m_pHMD is a pointer to your OpenVR system
        vr::TrackedDeviceIndex_t leftControllerIndex = vr::k_unTrackedDeviceIndexInvalid;
        vr::TrackedDeviceIndex_t rightControllerIndex = vr::k_unTrackedDeviceIndexInvalid;

        for (vr::TrackedDeviceIndex_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; deviceIndex++) {
            if (m_pHMD->IsTrackedDeviceConnected(deviceIndex)) {
                vr::VRControllerState_t controllerState;
                vr::TrackedDevicePose_t controllerPose;

                uint32_t packetNum = m_pHMD->GetControllerState(deviceIndex, &controllerState, sizeof(controllerState));
                // Get the role of the device
                vr::ETrackedControllerRole controllerRole = m_pHMD->GetControllerRoleForTrackedDeviceIndex(deviceIndex);
                // Left controller
                if (packetNum != 0) {

                    if (controllerRole == vr::TrackedControllerRole_LeftHand) {
                        //vr::VRSystem()->GetControllerStateWithPose(vr::TrackingUniverseStanding, deviceIndex, &controllerState, sizeof(controllerState), &controllerPose);
                        //vr::HmdMatrix34_t matPose = controllerPose.mDeviceToAbsoluteTracking;
                        //glm::mat4 poseMatrix = ConvertSteamVRMatrixToMatrix4(controllerPose.mDeviceToAbsoluteTracking);
                        //
                        //
                        //
                        ////poseMatrix *= rotationMatrix;
                        //glm::mat4 matMVPL = GetCurrentViewProjectionMatrix(vr::Eye_Left);
                        //glm::mat4 matMVPR= GetCurrentViewProjectionMatrix(vr::Eye_Right);
                        //
                        //position = {
                        //    poseMatrix[3][0],
                        //    poseMatrix[3][1],
                        //    poseMatrix[3][2]
                        //};
                        //
                        //glm::vec4 direction = glm::inverse(matMVPL) * glm::vec4(poseMatrix[3][0], poseMatrix[3][1], poseMatrix[3][2], 0.0f);
                        //position.x += direction.x;
                        //position.y += direction.y;
                        //position.z += direction.z;
                        //
                        //orientation = {
                        //    matPose.m[0][0],
                        //    matPose.m[1][0],
                        //    matPose.m[2][0],
                        //    sqrt(1 + matPose.m[0][0] + matPose.m[1][1] + matPose.m[2][2]) / 2
                        //};
                        ////position *= glm::vec3(orientation[0], orientation[1], orientation[2]);
                        //
                        //// Output the position and orientation of the controller
                        //std::cout << "Controller Position: (" << poseMatrix[3][0] << ", " << poseMatrix[3][1] << ", " << poseMatrix[3][2] << ")" << std::endl;
                        //std::cout << "Controller Position: (" << matPose.m[0][3] << ", " << matPose.m[1][3] << ", " << matPose.m[2][3] << ")" << std::endl;
                        //std::cout << "Controller Orientation: (" << orientation.x << ", " << orientation.y << ", " << orientation.z << ", " << orientation.w << ")" << std::endl;
                        pos.y -= deadzoned(controllerState.rAxis[0].y, 0.5f) / 50;
                    }
                    // Right controller
                    else if (controllerRole == vr::TrackedControllerRole_RightHand) {
                        //pos.x += deadzoned(controllerState.rAxis[0].y) / 10;
                        yaw += glm::radians(deadzoned(controllerState.rAxis[0].x, 0.95f));

                        rotationMatrix = glm::rotate(glm::mat4(1.0f), yaw, glm::vec3(0.0f, 1.0f, 0.0f));

                        m_mat4HMDPose *= rotationMatrix;

                        MoveForward(deadzoned(controllerState.rAxis[0].y, 0.5f) / 10);
                        
                        //m_mat4HMDPose = glm::translate(m_mat4HMDPose, rota);
                    }
                }
            }
        }
    }

    glm::quat GetOrientation() {
        glm::mat3 rotationMat(
            m_mat4HMDPose[0][0], m_mat4HMDPose[0][1], m_mat4HMDPose[0][2],
            m_mat4HMDPose[1][0], m_mat4HMDPose[1][1], m_mat4HMDPose[2][2],
            m_mat4HMDPose[2][0], m_mat4HMDPose[2][1], m_mat4HMDPose[2][2]
        );
        return glm::toQuat(rotationMat);
    }

    void MoveForward(float distance) {
        glm::vec4 direction = glm::inverse(m_mat4HMDPose) * glm::vec4(0.0f, 0.0f, distance, 0.0f);
        pos.x += direction.x;
        pos.y += direction.y;
        pos.z += direction.z;
    }

    float deadzoned(float num, float zone) {
        if (num > 0)
            return (num > zone) ? num : 0.0f;
        else
            return (num < -zone) ? num : 0.0f;
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
        return glm::translate(matMVP, pos);
    }

    uint32_t GetRenderWidth() const{
        return m_nRenderWidth;
    }

    uint32_t GetRenderHeight() const{
        return m_nRenderHeight;
    }

    FramebufferDesc GetFrameBufferDesc(vr::Hmd_Eye nEye) {
        if (nEye == vr::Eye_Left)
        {
            return leftEyeDesc;
        }
        else if (nEye == vr::Eye_Right)
        {
            return rightEyeDesc;
        }
    }

private:
    FramebufferDesc leftEyeDesc;
    FramebufferDesc rightEyeDesc;
    uint32_t m_nRenderWidth;
    uint32_t m_nRenderHeight;

    vr::EVRInitError err = vr::VRInitError_None;
    vr::IVRSystem* m_pHMD;
    glm::vec3 worldPos;

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

    void init_camera() {
        //SetupStereoRenderTargets
        m_pHMD->GetRecommendedRenderTargetSize(&m_nRenderWidth, &m_nRenderHeight);
        //SetupCamera
        SetupCameras();
        CreateFrameBuffer(m_nRenderHeight, m_nRenderHeight, leftEyeDesc);
        CreateFrameBuffer(m_nRenderHeight, m_nRenderHeight, rightEyeDesc);
    }

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
        return glm::mat4(
            matPose.m[0][0], matPose.m[1][0], matPose.m[2][0], 0.0,
            matPose.m[0][1], matPose.m[1][1], matPose.m[2][1], 0.0,
            matPose.m[0][2], matPose.m[1][2], matPose.m[2][2], 0.0,
            matPose.m[0][3], matPose.m[1][3], matPose.m[2][3], 1.0f
        );
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

};