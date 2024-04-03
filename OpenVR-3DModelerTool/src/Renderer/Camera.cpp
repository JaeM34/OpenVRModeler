#include "Camera.h"
#include <iostream>

Camera::Camera(float x, float y, float z) :
	m_Position(x, y, z),
	m_View(0.0f, 0.0f, -1.0f),
	m_Right(1.0f, 0.0f, 0.0f),
	m_Up(0.0f, 1.0f, 0.0f),
	m_Rotation(0.0f, 0.0f, 0.0f)
{
}

Camera::Camera(const glm::vec3& position, const glm::vec3& up, const glm::vec3& right, const glm::vec3& view) :
	m_Position(position),
	m_View(view),
	m_Right(right),
	m_Up(up),
	m_Rotation(0.0f, 0.0f, 0.0f)
{
}

void Camera::OnRender() {
	m_ViewMatrix = glm::lookAt(m_Position, m_View + m_Rotation, m_Up);
}

void Camera::Rotate(float angle, float x, float y, float z)
{
	m_Rotation = glm::rotate(m_View, angle, glm::vec3(x, y, z));
}

void Camera::RotateX(float radians)
{
	m_Rotation.x += radians;

	Rotate(radians, m_Right.x, m_Right.y, m_Right.z);
	m_Up = glm::cross(m_Right, m_View);
}

void Camera::RotateY(float radians)
{
	m_Rotation.y += radians;

	Rotate(radians, m_Right.x, m_Right.y, m_Right.z);
	m_Up = glm::cross(m_Right, m_View);
}

void Camera::RotateZ(float radians)
{
	m_Rotation.z += radians;

	Rotate(radians, m_Right.x, m_Right.y, m_Right.z);
	m_Up = glm::cross(m_Right, m_View);
}



void Camera::Transform(float x, float y, float z)
{

}

void Camera::MoveForward(float distance)
{
	m_Position = m_Position + (m_View * distance);
}

void Camera::MoveUpwards(float distance)
{
	m_Position = m_Position + (m_Up * distance);
}

void Camera::MoveSideways(float distance)
{
	m_Position = m_Position + (m_Right * distance);
}

