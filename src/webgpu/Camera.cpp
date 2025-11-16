#include "Camera.h"

#include "input/Controller.h"

Camera::Camera(std::shared_ptr<Controller> controller) : m_controller{controller}, m_position{}, m_rotation{}
{

}

void Camera::onMove(float x, float y, float z)
{
}


