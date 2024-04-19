#ifndef CAMERA_H
#define CAMERA_H
#include "glm/glm.hpp"
#include "glm/gtc/quaternion.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "math.h"

class Camera {
public:
    Camera(float x, float y, float z);

    Camera(const glm::vec3& position,
        const glm::vec3& up,
        const glm::vec3& right,
        const glm::vec3& view);

    void OnRender();

    void Rotate(float yaw, float pitch);
    //void Rotate(glm::quat &q);
    //void RotateX(float radians);
    //void RotateY(float radians);
    //void RotateZ(float radians);
    void Transform(float x, float y, float z);

    void MoveForward(float distance);
    void MoveUpwards(float distance);
    void MoveSideways(float distance);


    const glm::mat4 &ViewMatrix() const {
        return m_ViewMatrix;
    }

    const glm::mat4 &ProjectionMatrix() const {
        return m_ProjectionMatrix;
    }

    void SetUpVector(float x, float y, float z) {
        m_Up.x = x;
        m_Up.y = y;
        m_Up.z = z;
    }

    void SetRightVector(float x, float y, float z) {
        m_Right.x = x;
        m_Right.y = y;
        m_Right.z = z;
    }

    void SetViewVector(float x, float y, float z) {
        m_View.x = x;
        m_View.y = y;
        m_View.z = z;
    }

    void SetFov(float fov) {
        m_fov = fov;
    }

    float GetFov() {
        return m_fov;
    }

private:
    // Position
    glm::vec3 m_Position;
    glm::mat4 m_ViewMatrix;
    glm::mat4 m_ProjectionMatrix;

    // Orientation
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_View;

    // Rotation
    float yaw;
    float pitch;
    glm::vec3 m_Rotation;

    // Settings
    float m_fov;
};

#endif