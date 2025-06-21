#include "helpers.h"
#include "shader.h"
#include "program.h"

#define MOUSE_SENS 0.1

// Settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

float lastX = SCR_WIDTH * 0.5;
float lastY = SCR_HEIGHT * 0.5;

glm::vec3 cameraPos   = {0.0f, 0.0f,  3.0f};
glm::vec3 cameraUp    = {0.0f, 1.0f,  0.0f};   // will be re-computed if you want roll
float     cameraYaw   = -90.0f;   // pointing toward –Z
float     cameraPitch =   0.0f;   // looking level
float     cameraRoll  =   0.0f;   // optional – see below
float     cameraSpeed =   2.5f;   // units per second
glm::vec3 front, up, right;

float mouseDX = 0.0f;
float mouseDY = 0.0f;

bool first_move = true;

void func (GLFWwindow* window, double xpos, double ypos) {
    if (first_move) {
        lastX = xpos;
        lastY = ypos;
        first_move = false;
    }

    mouseDX = (xpos - lastX) * MOUSE_SENS;
    mouseDY = (ypos - lastY) * MOUSE_SENS;

    cameraYaw += mouseDX;
    cameraPitch = fmin(89, fmax(-89, cameraPitch - mouseDY));


    // std::cout << cameraYaw << "   " << cameraPitch << std::endl;

    lastX = xpos;
    lastY = ypos;
}

int main()
{
    if (!glfwInit()) {
        std::cerr << "ERROR: Failed to initialize GLFW" << std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 2. Create a Window
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Cube", NULL, NULL);
    if (window == NULL) {
        std::cerr << "ERROR: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // glfwSwapInterval(1);
    // crucial for macos
    glfwSetFramebufferSizeCallback(window, render_helper::framebuffer_size_callback);

    // 3. Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "ERROR: Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::cout << "Successfully initialized OpenGL!" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);

    Program* ShaderProgram = new Program("./shaders/vertex.glsl", "./shaders/fragment.glsl");

    // 6. Set up vertex data and buffers (EBO version)
    float vertices[] = {
        // positions
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,

         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,

        -0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,

        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,

         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,

        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
    };

    unsigned int indices[] = {
        0, 1, 2,  2, 3, 0, // Back
        4, 5, 6,  6, 7, 4, // Front
        7, 3, 2,  2, 6, 7, // Top
        4, 0, 1,  1, 5, 4, // Bottom
        4, 7, 3,  3, 0, 4, // Left
        1, 5, 6,  6, 2, 1  // Right
    };

    int normals[] = {
        0,  0, -1,    0,  0, -1, 
        0,  0,  1,    0,  0,  1,
        0, -1,  0,    0, -1,  0,
        0,  1,  0,    0,  1,  0,
       -1,  0,  0,   -1,  0,  0,
        1,  0,  0,    1,  0,  0
    };

    // vertex buffer obeject, vertex array object, element buffer object
    unsigned int VBOPos, VBONormals, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBOPos);
    glBindBuffer(GL_ARRAY_BUFFER, VBOPos);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glGenBuffers(1, &VBONormals);
    glBindBuffer(GL_ARRAY_BUFFER, VBONormals);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normals), normals, GL_STATIC_DRAW);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front.y = sin(glm::radians(cameraPitch));
    front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front   = glm::normalize(front);

    right = glm::normalize(glm::cross(front, {0.0f, 1.0f, 0.0f}));
    up    = glm::normalize(glm::cross(right, front)); // roll = 0°

    float lastFrame = glfwGetTime();
    glfwSetCursorPosCallback(window, func);
     if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_CURSOR_DISABLED);
        std::cout << "yey" << std::endl;
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    int frame_count = 0;
    float acc_time = 0;

    while (!glfwWindowShouldClose(window)) {

        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        acc_time += deltaTime;
        lastFrame = currentFrame;

        if (acc_time >= 1) {
            std::cout << "FPS: " << frame_count << std::endl;
            acc_time = 0;
            frame_count = 0;
        }

        render_helper::processInput(window, deltaTime, cameraPos, front, up, cameraSpeed);

        front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
        front.y = sin(glm::radians(cameraPitch));
        front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
        front   = glm::normalize(front);

        right = glm::normalize(glm::cross(front, {0.0f, 1.0f, 0.0f}));
        up    = glm::normalize(glm::cross(right, front)); // roll = 0°

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(ShaderProgram->get_id());

        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        view = glm::lookAt(cameraPos, cameraPos + front, up);
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        
        glm::vec3 color = glm::vec3(0, 1, 0);
        glm::vec3 light_position = glm::vec3(-1.0f, 1.0f, 1.0f);

        glUniform3fv(glGetUniformLocation(ShaderProgram->get_id(), "color"), 1, glm::value_ptr(color));

        glUniform3fv(glGetUniformLocation(ShaderProgram->get_id(), "light_position"), 1, glm::value_ptr(light_position));

        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram->get_id(), "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram->get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram->get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_count++;
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBOPos);
    glDeleteBuffers(1, &VBONormals);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(ShaderProgram->get_id());
    delete ShaderProgram;

    glfwTerminate();
    return 0;
}