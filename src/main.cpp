#include "shader.h"
#include "camera.h"
#include "model.h"
#include "mesh.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/string_cast.hpp>


#include <iostream>
#include <fstream>
#include <cmath>
#include <memory>
#include <unistd.h>

const int screenWidth = 800;
const int screenHeight = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 4.0f));
float lastX = screenWidth / 2.0f;
float lastY = screenHeight / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

// Window functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
GLFWwindow* InitializeAndCreateWindow(int width, int height);

int main() {
    GLFWwindow *window = InitializeAndCreateWindow(screenWidth, screenHeight);
    if (window == NULL) {
        glfwTerminate();
        return -1;
    }
    // Set resize callback
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glEnable(GL_DEPTH_TEST);

    std::unique_ptr<Mesh> cube(createCube(0.5f));

    // Light
    glm::vec3 lightPos(1.2f, 1.0f, 1.0f);

    // Shaders
    ShaderProgram shaderProgram("resources/shaders/skeleton_shader.vert",
                                "resources/shaders/diffuse_texture_shader.frag");
    shaderProgram.use();
    shaderProgram.setFloatVector("lightColor", {1.0f, 1.0f, 1.0f});
    shaderProgram.setVec3("lightPos", lightPos);

    ShaderProgram lampShader("resources/shaders/lamp.vert", "resources/shaders/lamp.frag");

    // Choose a model to load
    // AnimatedModel ourModel("resources/models/stickTut15.dae");
	// std::unique_ptr<AnimatedModel> ourModel(new AnimatedModel("resources/models/stickTut15.dae"));
    MotionCaptureData motion_capture_data("resources/models/17_03.bvh");
    std::unique_ptr<AnimatedModel> ourModel(new AnimatedModel("resources/models/eng_attempt2.6.dae", &motion_capture_data));

    // AnimatedModel ourModel("resources/models/BlackDragon/Dragon 2.5_dae.dae");
    ourModel->debugPrintout();

    glm::mat4 model;

    float startTime = glfwGetTime();
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime() - startTime;
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float) screenWidth / (float) screenHeight,
                                                0.1f, 100.0f);
        // camera/view transformation
        glm::mat4 view = camera.GetViewMatrix();

        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.2f));
        lampShader.use();
        lampShader.setMat4("view", view);
        lampShader.setMat4("projection", projection);
        lampShader.setMat4("model", model);
        cube->draw(lampShader);

        shaderProgram.use();
        shaderProgram.setMat4("projection", projection);
        shaderProgram.setMat4("view", view);
        shaderProgram.setVec3("viewPos", camera.Position);

        model = glm::scale(glm::mat4(1.0f), glm::vec3(0.1f));
        shaderProgram.setMat4("model", model);
        shaderProgram.setMat3("normalModel", glm::mat3(model));
        ourModel->draw(shaderProgram, currentFrame);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    cube.reset();
    ourModel.reset();

    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow *window)
{
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if(firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates range from bottom to top
    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(yoffset);
}

GLFWwindow* InitializeAndCreateWindow(int width, int height) {

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); for macOS

    GLFWwindow* window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        return NULL;
    }
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return NULL;
    }

    glViewport(0, 0, width, height);
    return window;
}