#include <OpenGLPrj.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <vector>
#include <cmath>
#include <cstdlib> //for rand()
#include <algorithm>
#include <functional>
#include <random>


const std::string program_name = ("GLSL shaders & uniforms");

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;


static const char *vertexShaderSource ="#version 330 core\n"
                                       "layout (location = 0) in vec3 aPos;\n"
                                       "uniform mat4 model;\n"
                                       "uniform mat4 view;\n"
                                       "uniform mat4 projection;"
                                       "void main()\n"
                                       "{\n"
                                       "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
                                       "}\0";

static const char *fragmentShaderSource = "#version 330 core\n"
                                          "out vec4 FragColor;\n"
                                          "uniform vec4 ourColor;\n"
                                          "void main()\n"
                                          "{\n"
                                          "   FragColor = ourColor;\n"
                                          "}\n\0";

glm::mat4 view = glm::mat4(1.0f);
glm::vec3 cameraPos   = glm::vec3(10.0f, 0.0f,  7.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

std::vector<std::vector<int>> maze;

void generateMaze(std::vector<std::vector<int>>& maze, int rows, int cols) {
    // Initialize the maze with walls
    maze.resize(rows, std::vector<int>(cols, 1));

    // Start recursive backtracking from a random cell
    std::function<void(int, int)> carve = [&](int x, int y) {
        maze[x][y] = 0; // Mark the current cell as a path

        // Directions for moving (right, down, left, up)
        std::vector<std::pair<int, int>> directions = {{0, 2}, {2, 0}, {0, -2}, {-2, 0}};
        std::shuffle(directions.begin(), directions.end(), std::default_random_engine(std::random_device{}()));

        for (auto& [dx, dy] : directions) {
            int nx = x + dx, ny = y + dy;
            if (nx > 0 && nx < rows - 1 && ny > 0 && ny < cols - 1 && maze[nx][ny] == 1) {
                // Break the wall between cells
                maze[x + dx / 2][y + dy / 2] = 0;
                carve(nx, ny);
            }
        }
    };

    carve(1, 1); // Start carving from (1, 1)
}

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, program_name.c_str(), nullptr, nullptr);
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    // vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);

    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // link shaders
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------

    float cubeVertices[] = {
            // Define the vertices of the cube (each cube is 1x1)
            -0.5f, -0.9f, -0.5f,
            0.5f, -0.9f, -0.5f,
            0.5f, 5.0f, -0.5f,
            -0.5f, 5.0f, -0.5f,
            -0.5f, -0.9f, 0.5f,
            0.5f, -0.9f, 0.5f,
            0.5f, 5.0f, 0.5f,
            -0.5f, 5.0f, 0.5f
    };

    unsigned int cubeIndices[] = {
            0, 1, 2, 2, 3, 0,  // back face
            4, 5, 6, 6, 7, 5,  // front face
            0, 4, 1, 4, 1, 5,  // left face
            3, 7, 2, 2, 7, 6,  // right face
            0, 4, 3, 4, 3, 7,  // bottom face
            1, 5, 2, 5, 2, 6   // top face
    };

    generateMaze(maze, 19, 19);


    unsigned int cubeVAO, cubeVBO, cubeEBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);
    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *) 0);
    glEnableVertexAttribArray(0);

    // Set up some OpenGL state
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Wireframe mode
    glEnable(GL_DEPTH_TEST);

    // hide the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    void mouse_callback(GLFWwindow *window, double xpos, double ypos);
    // render loop

    // -----------
    while (!glfwWindowShouldClose(window)) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // Handle input and update frame timing
        glfwSetCursorPosCallback(window, mouse_callback);
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        processInput(window);

        // Clear screen and set up matrices
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shaderProgram);

        glm::mat4 model = glm::mat4(1.0f);
        view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 800.0f, 0.1f, 100.0f);

        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        glUniform4f(vertexColorLocation, 0.0f, 0.75f, 1.0f, 1.0f);

        // Render maze
        for (int i = 0; i < maze.size(); ++i) {
            for (int j = 0; j < maze[i].size(); ++j) {
                if (maze[i][j] == 1) { // Wall
                    glm::mat4 cubeModel = glm::mat4(1.0f);
                    cubeModel = glm::translate(cubeModel,
                                               glm::vec3(j - maze[0].size() / 2, -0.5f, i - maze.size() / 2));
                    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(cubeModel));
                    glBindVertexArray(cubeVAO);
                    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
                }
            }
        }

        // Swap buffers and poll events (only once per frame)
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

        // Optional: de-allocate all resources once they've outlived their purpose
        glDeleteVertexArrays(1, &cubeVAO);
        glDeleteBuffers(1, &cubeVBO);
        glDeleteBuffers(1, &cubeEBO);
        glDeleteProgram(shaderProgram);

        // glfw: terminate, clearing all previously allocated GLFW resources.
        glfwTerminate();
        return 0;
    }


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processMovement(glm::vec3 direction, float speed, glm::vec3& position, const std::vector<glm::vec3>& walls, float wallSize = 1.0f)
{
    glm::vec3 newPos = position + direction * speed;

    for (const auto& wall : walls)
    {
        float distanceX = std::abs(newPos.x - wall.x);
        float distanceZ = std::abs(newPos.z - wall.z);

        // Only check collisions on the X-Z plane
        if (distanceX < wallSize / 2.0f && distanceZ < wallSize / 2.0f)
        {
            return; // Collision detected, do not update position
        }
    }

    // Update position if no collision is detected
    position.x = newPos.x;
    position.z = newPos.z;
}
void processInput(GLFWwindow *window)
{
    std::vector<glm::vec3> wallCoordinates;


    for (int i = 0; i < maze.size(); ++i) {
        for (int j = 0; j < maze[i].size(); ++j) {
            if (maze[i][j] == 1) { // If it's a wall
                // Calculate the world coordinates
                glm::vec3 wallPosition(
                        j - maze[0].size() / 2, // X coordinate (adjusted for centering)
                        0.0f,                  // Y coordinate (fixed at ground level)
                        i - maze.size() / 2    // Z coordinate (adjusted for centering)
                );

                // Store the wall position
                wallCoordinates.push_back(wallPosition);
            }
        }
    }





    //ogranichuvanje za dvizhenje
    float minX = -10.0f;
    float maxX = 10.0f;
    float minZ = -10.0f;
    float maxZ = 10.0f;

    float cameraSpeed = 3.0f * deltaTime;
    glm::vec3 camera = glm::vec3(1.0f, 0.0f,  1.0f);
    static bool isJumping = false;
    static float jumpHeight = 0.5f;
    static float jumpTime = 0.0f;
    static float jumpDuration = 0.5f;
    static glm::vec3 originalCameraPos;
    static bool isCrouching = false;
    if (!isJumping && !isCrouching && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    {
        // Start crouching
        isCrouching = true;

        cameraPos.y -= 0.5f;
    }
    else if (isCrouching && glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) != GLFW_PRESS)
    {

        isCrouching = false;
        cameraPos.y+=0.5f;
    }
    if (!isJumping)
    {
        originalCameraPos = camera;
    }
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        processMovement(cameraFront, cameraSpeed, cameraPos, wallCoordinates);
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        processMovement(-cameraFront, cameraSpeed, cameraPos, wallCoordinates);
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        processMovement(-glm::normalize(glm::cross(cameraFront, cameraUp)), cameraSpeed, cameraPos, wallCoordinates);
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        processMovement(glm::normalize(glm::cross(cameraFront, cameraUp)), cameraSpeed, cameraPos, wallCoordinates);
    }
    if (!isJumping && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    {
        // Start the jump
        isJumping = true;
        jumpTime = 0.0f; // Reset jump time
    }

    // Handle the jump logic
    if (isJumping)
    {
        // Update jump time
        jumpTime += deltaTime;

        // If still within jump duration
        if (jumpTime <= jumpDuration)
        {
            // Jump up
            cameraPos.y = originalCameraPos.y + jumpHeight * (1.0f - 4.0f * pow((jumpTime / jumpDuration - 0.5f), 2.0f));
        }
        else
        {
            // End the jump
            isJumping = false;
            jumpTime = 0.0f; // Reset jump time

        }
    }



    // Adjust as needed

    //cameraPos -= glm::normalize(glm::cross(cam, cameraUp)) * cameraSpeed;
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
bool firstMouse = true;
float lastX = 400, lastY = 400;
float yaw = -90.0f;
float pitch = 0.0f;

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw   += xoffset;
    pitch += yoffset;

    if(pitch > 89.0f)
        pitch = 89.0f;
    if(pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 direction;
    direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    direction.y = sin(glm::radians(pitch));
    direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(direction);
}