#pragma once
#include "Camera.h"
#include <iostream>

Camera::Camera(float x, float y, float z) :
	m_Position(x, y, z),
	m_View(0.0f, 0.0f, -1.0f),
	m_Right(1.0f, 0.0f, 0.0f),
	m_Up(0.0f, 1.0f, 0.0f),
	m_Rotation(0.0f, 0.0f, 0.0f),
	yaw(-90.0f),
	pitch(0.0f),
	m_fov(60.0f)
{
}

Camera::Camera(const glm::vec3& position, const glm::vec3& up, const glm::vec3& right, const glm::vec3& view) :
	m_Position(position),
	m_View(view),
	m_Right(right),
	m_Up(up),
	m_Rotation(0.0f, 0.0f, 0.0f),
	yaw(-90.0f),
	pitch(0.0f),
	m_fov(60.0f)
{
}

void Camera::OnRender() {
	m_ViewMatrix = glm::lookAt(m_Position, m_Position + m_View, m_Up);
}

void Camera::Rotate(float yaw, float pitch)
{
	glm::vec3 front;
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	m_View = glm::normalize(front);
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
	m_Position = m_Position + glm::normalize(glm::cross(m_View, m_Up)) * distance;
}

