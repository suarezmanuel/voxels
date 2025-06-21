#include "helpers.h"
#include "shader.h"
#include "program.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

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
        // Back Face
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
         0.5f,  0.5f, -0.5f,
        -0.5f,  0.5f, -0.5f,
        -0.5f, -0.5f, -0.5f,

        // Front Face
        -0.5f, -0.5f,  0.5f,
         0.5f, -0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
         0.5f,  0.5f,  0.5f,
        -0.5f,  0.5f,  0.5f,
        -0.5f, -0.5f,  0.5f,

        // Left Face
        -0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, 
        -0.5f, -0.5f, -0.5f, 
        -0.5f, -0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, 

        // Right Face
         0.5f,  0.5f,  0.5f, 
         0.5f,  0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f,  0.5f, 
         0.5f,  0.5f,  0.5f, 

        // Bottom Face
        -0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f, -0.5f, 
         0.5f, -0.5f,  0.5f, 
         0.5f, -0.5f,  0.5f, 
        -0.5f, -0.5f,  0.5f, 
        -0.5f, -0.5f, -0.5f, 

        // Top Face
        -0.5f,  0.5f, -0.5f, 
         0.5f,  0.5f, -0.5f, 
         0.5f,  0.5f,  0.5f, 
         0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f,  0.5f, 
        -0.5f,  0.5f, -0.5f, 
    };

    float normals[] = {
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,
        0.0f,  0.0f, -1.0f,

        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,
        0.0f,  0.0f,  1.0f,

       -1.0f,  0.0f,  0.0f,
       -1.0f,  0.0f,  0.0f,
       -1.0f,  0.0f,  0.0f,
       -1.0f,  0.0f,  0.0f,
       -1.0f,  0.0f,  0.0f,
       -1.0f,  0.0f,  0.0f,

        1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,
        1.0f,  0.0f,  0.0f,

        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,
        0.0f, -1.0f,  0.0f,

        0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f,
        0.0f,  1.0f,  0.0f
    };

    int row = 25;
    int col = 20;
    float len = 1.0/16.0; // textures are 16x16
    float off = 0.5;
    float textures[] = {

        col*len, row*len,
        (col+off/2)*len, (row)*len,
        (col+off/2)*len, (row+off)*len,

        (col+off/2)*len, (row+off)*len,
        (col+0.0f)*len, (row+off)*len,
        (col)*len, (row)*len,

        col*len, row*len,
        (col+off/2)*len, (row)*len,
        (col+off/2)*len, (row+off)*len,
        (col+off/2)*len, (row+off)*len,
        (col+0.0f)*len, (row+off)*len,
        (col)*len, (row)*len,

        col*len, row*len,
        (col+off/2)*len, (row)*len,
        (col+off/2)*len, (row+off)*len,
        (col+off/2)*len, (row+off)*len,
        (col+0.0f)*len, (row+off)*len,
        (col)*len, (row)*len,

        col*len, row*len,
        (col+off/2)*len, (row)*len,
        (col+off/2)*len, (row+off)*len,
        (col+off/2)*len, (row+off)*len,
        (col+0.0f)*len, (row+off)*len,
        (col)*len, (row)*len,

        col*len, row*len,
        (col+off/2)*len, (row)*len,
        (col+off/2)*len, (row+off)*len,
        (col+off/2)*len, (row+off)*len,
        (col+0.0f)*len, (row+off)*len,
        (col)*len, (row)*len,

        col*len, row*len,
        (col+off/2)*len, (row)*len,
        (col+off/2)*len, (row+off)*len,
        (col+off/2)*len, (row+off)*len,
        (col+0.0f)*len, (row+off)*len,
        (col)*len, (row)*len,
    };

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load("./textures/atlas.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        // Note: If your atlas has transparency (alpha channel), use GL_RGBA
        GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data); // Free the image memory


    // vertex buffer obeject, vertex array object, element buffer object
    unsigned int VBOPos, VBONormals, VBOTexture, VAO;
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

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);

    glGenBuffers(1, &VBOTexture);
    glBindBuffer(GL_ARRAY_BUFFER, VBOTexture);
    glBufferData(GL_ARRAY_BUFFER, sizeof(textures), textures, GL_STATIC_DRAW);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(2);

    front.x = cos(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front.y = sin(glm::radians(cameraPitch));
    front.z = sin(glm::radians(cameraYaw)) * cos(glm::radians(cameraPitch));
    front   = glm::normalize(front);

    right = glm::normalize(glm::cross(front, {0.0f, 1.0f, 0.0f}));
    up    = glm::normalize(glm::cross(right, front)); // roll = 0°

    float lastFrame = glfwGetTime();
    glfwSetCursorPosCallback(window, func);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
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

        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        view = glm::lookAt(cameraPos, cameraPos + front, up);
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        
        glm::vec3 color = glm::vec3(0, 1, 0);
        glm::vec3 light_position = glm::vec3(-1.0f, 1.0f, 1.0f);

        glUniform1i(glGetUniformLocation(ShaderProgram->get_id(), "textureSampler"), 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        // glUniform3fv(glGetUniformLocation(ShaderProgram->get_id(), "color"), 1, glm::value_ptr(color));

        glUniform3fv(glGetUniformLocation(ShaderProgram->get_id(), "light_position"), 1, glm::value_ptr(light_position));

        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram->get_id(), "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram->get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(ShaderProgram->get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, sizeof(vertices) / (3 * sizeof(vertices[0])));

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_count++;
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBOPos);
    glDeleteBuffers(1, &VBONormals);
    glDeleteProgram(ShaderProgram->get_id());
    delete ShaderProgram;

    glfwTerminate();
    return 0;
}