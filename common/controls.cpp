// Include standard headers
#include <stdio.h>

// Include GLFW
#include <glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "controls.hpp"

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

glm::mat4 getViewMatrix(){
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix(){
	return ProjectionMatrix;
}

// Initial distance between eye and center :
static float radius = 5;

// Initial positions and directions of view :
glm::vec4 center = glm::vec4( 0, 0, 0, 1 );
glm::vec4 direction = glm::vec4( 0, 0, -1, 0);
glm::vec4 right = glm::vec4( 1, 0, 0, 0 );
glm::vec4 up = glm::vec4( 0, 1, 0, 0 );

// Mouse info
double xpos, ypos;
float dx, dy;

enum MouseMode { Rotating, Translating, Zooming, None };
MouseMode mode = None;

float translationSpeed = 0.005f;
float rotationSpeed = 0.005f;
float zoomSpeed = 0.01f;
float zoomSpeedScrollWheel = 0.05f;

void updateView() {
    ViewMatrix = glm::lookAt(
                             glm::vec3(center-radius*direction), // camera is located here
                             glm::vec3(center), // camera looks at center point
                             glm::vec3(up)  // head is up
                             );
}

void MouseDraggedCallback(GLFWwindow*, double x, double y)
{
    dx = x - xpos; // x cursor increases from left to right
    dy = ypos - y; // y cursor increases from top to bottom
    
    switch (mode)
    {
    case Rotating:
        {
        //printf ("rotate: dx = %f, dy = %f\n", dx, dy);
        
        glm::mat4 Rtilt = glm::rotate(glm::mat4(1.0f), rotationSpeed * dy, glm::vec3(right));
        direction = Rtilt * direction;
        up = Rtilt * up;
        
        glm::mat4 Rpan = glm::rotate(glm::mat4(1.0f), rotationSpeed * -dx, glm::vec3(up));
        direction = Rpan * direction;
        right = Rpan * right;
        }
        break;
            
    
    case Translating:
        
        //printf ("translate: dx = %f, dy = %f\n", dx, dy);
        center -= dx * translationSpeed * right;
        center -= dy * translationSpeed * up;

        break;
        
    case Zooming:
        
        //printf ("zoom: dy = %f, radius = %f\n", dy, radius);
        radius += zoomSpeed * radius * dy;
        break;
            
    case None:
        break;
            
    }
    
    xpos = x;
    ypos = y;
    updateView();
}


void MouseScrollCallback(GLFWwindow*, double dx, double dy)
{
    radius += zoomSpeedScrollWheel * radius * dy;
    updateView();
}

void MousePressCallback(GLFWwindow*, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT  && action == GLFW_PRESS)
    {
        if(glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS) {
            mode = Translating;
        }
        else if(glfwGetKey( window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS) {
            mode = Zooming;
        }
        else {
            mode = Rotating;
        }
    }
    
    if (action == GLFW_RELEASE)
    {
        mode = None;
    }
}

void initializeMouseCallbacks() {
    
    // Setup callback functions
    glfwSetScrollCallback(window, MouseScrollCallback);
    glfwSetMouseButtonCallback(window, MousePressCallback);
    glfwSetCursorPosCallback(window, MouseDraggedCallback);
    
    // Initialize View and Project matrices
    ProjectionMatrix = glm::perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
    updateView();
    
}
