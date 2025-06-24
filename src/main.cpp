#include "../headers/program.h"
#include "../headers/camera.h"
#include "../headers/generation.h"
#include <unordered_set>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


int main() {
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "💣NeriaCraft™💣", NULL, NULL);
    if (window == NULL) {
        std::cerr << "ERROR: Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    // crucial for macos
    glfwSetFramebufferSizeCallback(window, render_helper::framebuffer_size_callback);

    glfwSetCursorPosCallback(window, camera::mouseMovementCallback);
    if (glfwRawMouseMotionSupported()) {
        glfwSetInputMode(window, GLFW_RAW_MOUSE_MOTION, GLFW_CURSOR_DISABLED);
    } else {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    }
    // 3. Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "ERROR: Failed to initialize GLAD" << std::endl;
        return -1;
    }

    std::cout << "Successfully initialized OpenGL!" << std::endl;
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;

    // === IMGUI: 2. Initialize ImGui ===
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    const char* glsl_version = "#version 330"; // Match your OpenGL version
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
    // ===================================

    // Configure global OpenGL state
    glEnable(GL_DEPTH_TEST);

    Program* ShaderProgram = new Program("./shaders/vertex.glsl", "./shaders/fragment.glsl");
    Program* CrosshairProgram = new Program("./shaders/crosshair_vertex.glsl", "./shaders/crosshair_fragment.glsl");
    generator* gen = new generator(ShaderProgram);
    camera::updateCamera();

    float lastFrame = glfwGetTime();
    int frame_count = 0;
    float acc_time = 0;

    float crosshair_vertices [36] = {
        1,0,0, 1,0,0,
        0,0,0, 1,0,0,

        0,1,0, 0,1,0,
        0,0,0, 0,1,0,

        0,0,1, 0,0,1,
        0,0,0, 0,0,1
    };

    glUseProgram(CrosshairProgram->get_id());
    unsigned int CVAO, CVBO;
    glGenVertexArrays(1, &CVAO);
    glGenBuffers(1, &CVBO);
    glBindVertexArray(CVAO);
    glBindBuffer(GL_ARRAY_BUFFER, CVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(crosshair_vertices), crosshair_vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glBindVertexArray(0);

    // === IMGUI: State for UI mode and clear color ===
    bool ui_mode = false; // false = Game Mode, true = UI Mode
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // Start in game mode
    ImVec4 clear_color = ImVec4(0.2f, 0.3f, 0.3f, 1.0f);
    // ===============================================

    glfwSwapInterval(0);

    while (!glfwWindowShouldClose(window)) {

        glfwPollEvents();

        // === IMGUI: 3. Start the ImGui frame ===
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // ========================================

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentFrame = glfwGetTime();
        float deltaTime = currentFrame - lastFrame;
        acc_time += deltaTime;
        lastFrame = currentFrame;

        if (acc_time >= 1) {
            std::cout << "FPS: " << frame_count << std::endl;
            acc_time = 0;
            frame_count = 0;
        }

        static bool tab_key_was_pressed = false;
        if (glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS) {
            if (!tab_key_was_pressed) {
                ui_mode = !ui_mode;
                glfwSetInputMode(window, GLFW_CURSOR, ui_mode ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED);
                tab_key_was_pressed = true;
            }
        } else {
            tab_key_was_pressed = false;
        }

        // Only process game input and camera if NOT in UI mode
        if (!ui_mode) {
            render_helper::processInput(window, deltaTime, cameraPos, front, up);
            camera::updateCamera();
        }
        // ============================================

        // === IMGUI: 4. Build your UI ===
        // This is where you call ImGui functions to create windows, buttons, etc.
        ImGui::ShowDemoWindow(); // The big demo window, great for learning!

        // Example of a custom window
        {
            ImGui::Begin("NeriaCraft Control Panel");
            ImGui::Text("Press TAB to switch between game and UI mode.");
            ImGui::ColorEdit3("Background Color", (float*)&clear_color);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        render_helper::processInput(window, deltaTime, cameraPos, front, up);

        camera::updateCamera();

        glm::mat4 view = glm::mat4(1.0f);
        glm::mat4 projection = glm::mat4(1.0f);

        // model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        view = glm::lookAt(cameraPos, cameraPos + front, up);
        projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000000.0f);
    
        glm::vec3 light_position = cameraPos;

        glUseProgram(ShaderProgram->get_id());
        glUniform3fv(glGetUniformLocation(ShaderProgram->get_id(), "light_position"), 1, glm::value_ptr(light_position));

        std::unordered_set<glm::ivec3, IVec3Hash> current_required_chunks;
        generator_helper::calculate_required_chunks(current_required_chunks);

        gen->start_generation_tasks(current_required_chunks);
        gen->prune_unnecessary_chunks();
        gen->process_finished_mesh();
        gen->draw_all(ShaderProgram, view, projection);


        glUseProgram(CrosshairProgram->get_id());
        glm::mat4 gizmoModel = glm::mat4(1.0f);
        glm::vec3 gizmoPos = cameraPos + front * 3.0f; 
        gizmoModel = glm::translate(gizmoModel, gizmoPos);

        glUniformMatrix4fv(glGetUniformLocation(CrosshairProgram->get_id(), "model"), 1, GL_FALSE, glm::value_ptr(gizmoModel));
        glUniformMatrix4fv(glGetUniformLocation(CrosshairProgram->get_id(), "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(CrosshairProgram->get_id(), "projection"), 1, GL_FALSE, glm::value_ptr(projection));

        glLineWidth(3.0f);
        glBindVertexArray(CVAO);
        glDrawArrays(GL_LINES, 0, 6);

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();

        frame_count++;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glDeleteVertexArrays(1, &VAO);
    // glDeleteBuffers(1, &VBOPos);
    // glDeleteBuffers(1, &VBONormals);
    glDeleteProgram(ShaderProgram->get_id());
    delete ShaderProgram;

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}