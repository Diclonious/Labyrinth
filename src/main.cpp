#include <OpenGLPrj.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include <vector>
#include <cmath>
#include <cstdlib> //for rand()
#include <algorithm>


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
glm::vec3 cameraPos   = glm::vec3(0.0f, 0.0f,  3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp    = glm::vec3(0.0f, 1.0f,  0.0f);

float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame



int main()
{
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
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, program_name.c_str(), nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress)))
    {
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
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
    // fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
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

    float floorVertices[] = {
            // positions
            -10.0f, -0.9f,  10.0f,
            10.0f, -0.9f,  10.0f,
            -10.0f, -0.9f, -10.0f,
            10.0f, -0.9f, -10.0f,
            //ceiling
            // positions
            -10.0f, 9.0f,  10.0f,  // Bottom-left corner
            10.0f,  9.0f,  10.0f,  // Bottom-right corner
            -10.0f, 9.0f, -10.0f,  // Top-left corner
            10.0f,  9.0f, -10.0f   // Top-right corner
    };

    float cubeVertices[] = {
            // positions
            -10.0f, -0.9f, -10.0f,//0
            -10.0f, 9.0f, -10.f,//1
            -6.67f,  9.0f, -10.0f,//2 //1 triangle
            -6.67f,  -0.9f, -10.0f,//3
            -10.0f, -0.9f,  -6.67f,//4
            -6.67f, -0.9f,  -6.67f,//5
            -10.0f,  9.0f,  -6.67,//6
          -6.67f,  9.0f,  -6.67//7
    };

    unsigned int cubeIndices[] = {
            0, 1, 2, 2, 3, 0, // back face
            4, 5, 6, 6, 7, 5, // front face
            0,4,1,4,1,6,//left face
            3,5,2,2,5,7//right face

    };

    unsigned int cubeVAO, cubeVBO, cubeEBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glGenBuffers(1, &cubeEBO);
    glBindVertexArray(cubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), cubeVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cubeIndices), cubeIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
//floor and ceiling
    unsigned int floorVBO, floorVAO;
    glGenVertexArrays(1, &floorVAO);

    glGenBuffers(1, &floorVBO);


    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);



    // Unbind VAO (it's always a good thing to unbind any buffer/array to prevent strange bugs

    // Set up some OpenGL state
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // Wireframe mode
    glEnable(GL_DEPTH_TEST);
    //hide the cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    void mouse_callback(GLFWwindow* window, double xpos, double ypos);


    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        glfwSetCursorPosCallback(window, mouse_callback);

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Matrices
        glm::mat4 model = glm::mat4(1.0f);
        view = glm::lookAt(cameraPos, cameraFront + cameraPos, cameraUp);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 800.0f, 0.1f, 100.0f);

        unsigned int modelLoc = glGetUniformLocation(shaderProgram, "model");
        unsigned int viewLoc = glGetUniformLocation(shaderProgram, "view");
        unsigned int projLoc = glGetUniformLocation(shaderProgram, "projection");

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, &projection[0][0]);

        // Input
        processInput(window);

        // Render
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);

        int vertexColorLocation = glGetUniformLocation(shaderProgram, "ourColor");
        glUniform4f(vertexColorLocation, 0.0f,  0.75f, 1.0f, 1.0f);

        // Enable depth test
        glEnable(GL_DEPTH_TEST);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        // First pass: Draw the filled geometry (walls, floor, ceiling)
        glBindVertexArray(floorVAO);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);  // Floor
        glDrawArrays(GL_TRIANGLE_STRIP, 4, 4);  // Ceiling
        glBindVertexArray(0);
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glBindVertexArray(cubeVAO);
        glDrawElements(GL_TRIANGLES, 100, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);




        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // Switch back to filled mode

        // Swap buffers and poll events
        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &floorVAO);

    glDeleteBuffers(1, &floorVBO);

    glDeleteProgram(shaderProgram);
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
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
        // Move forward
        glm::vec3 newPos = cameraPos + cameraSpeed * camera * cameraFront * 1.5f;
        if (newPos.x > minX && newPos.x < maxX && newPos.z > minZ && newPos.z < maxZ)
            cameraPos = newPos;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        // Move backward
        glm::vec3 newPos = cameraPos - cameraSpeed * camera * cameraFront * 1.5f;
        if (newPos.x > minX && newPos.x < maxX && newPos.z > minZ && newPos.z < maxZ)
            cameraPos = newPos;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    {
        // Move left
        glm::vec3 newPos = cameraPos - glm::normalize(glm::cross(cameraFront * camera, cameraUp)) * cameraSpeed;
        if (newPos.x > minX && newPos.x < maxX && newPos.z > minZ && newPos.z < maxZ)
            cameraPos = newPos;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    {
        // Move right
        glm::vec3 newPos = cameraPos + glm::normalize(glm::cross(cameraFront * camera, cameraUp)) * cameraSpeed;
        if (newPos.x > minX && newPos.x < maxX && newPos.z > minZ && newPos.z < maxZ)
            cameraPos = newPos;
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