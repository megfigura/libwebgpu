#pragma once
#include <memory>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Controller;

class Camera
{
public:
    explicit Camera(std::shared_ptr<Controller> controller);
    ~Camera();

    void onMove(float x, float y, float z);


private:
    std::shared_ptr<Controller> m_controller;
    glm::vec3 m_position;
    glm::quat m_rotation;
};
