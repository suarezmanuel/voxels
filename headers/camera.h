#pragma once
#include "helpers.h"
#define MOUSE_SENS 0.1
#define SCR_WIDTH 800
#define SCR_HEIGHT 600
#define RENDER_DISTANCE 10

glm::vec3 cameraPos   = {0.0f, 100.0f,  3.0f};
glm::vec3 cameraUp    = {0.0f, 1.0f,  0.0f};   // will be re-computed if you want roll
float     cameraYaw   = -90.0f;   // pointing toward –Z
float     cameraPitch =   0.0f;   // looking level
float     cameraRoll  =   0.0f;   // optional – see below
glm::vec3 front, up, right;

float lastX = SCR_WIDTH * 0.5;
float lastY = SCR_HEIGHT * 0.5;

float mouseDX = 0.0f;
float mouseDY = 0.0f;

bool first_move = true;

namespace camera {

void mouseMovementCallback (GLFWwindow* window, double xpos, double ypos) {
    if (first_move) {
        lastX = xpos;
        lastY = ypos;
        first_move = false;
    }

    mouseDX = (xpos - lastX) * MOUSE_SENS;
    mouseDY = (ypos - lastY) * MOUSE_SENS;

    cameraYaw += mouseDX;
    cameraPitch = fmin(89, fmax(-89, cameraPitch - mouseDY));

    lastX = xpos;
    lastY = ypos;
}

void updateCamera() {
    front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front.y = sin(glm::radians(cameraPitch));
    front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front   = glm::normalize(front);

    right = glm::normalize(glm::cross(front, {0.0f, 1.0f, 0.0f}));
    up    = glm::normalize(glm::cross(right, front));
}
}