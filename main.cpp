#undef GLFW_DLL
#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <cmath>

#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "Libs/Mesh.h"
#include "Libs/stb_image.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

const GLint WIDTH = 800, HEIGHT = 600;

Window mainWindow;
std::vector<Mesh*> meshList;
std::vector<Shader> shaderList;

//Vertex Shader
static const char* vShader = "Shaders/shader.vert";

//Fragment Shader
static const char* fShader = "Shaders/shader.frag";

glm::vec3 lightColour = glm::vec3(1.0f, 1.0f, 1.0f);
glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0;


struct TexturedMesh
{
    Mesh *mesh;
    GLuint textureID;
};

std::vector<TexturedMesh> texturedMeshList;

GLuint loadTexture(const char* texturePath) {
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(texturePath, &width, &height, &nrChannels, 0);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, nrChannels == 4 ? GL_RGBA : GL_RGB, width, height, 0, nrChannels == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Fail to load image: " << texturePath << std::endl;
    }

    stbi_image_free(data);
    return textureID;
}

void CreateOBJ()
{
    Mesh *car = new Mesh();
    bool loadedCar = car->CreateMeshFromOBJ("Models/intialD.obj");
    Mesh *road = new Mesh();
    bool loadedRoad = road->CreateMeshFromOBJ("Models/road.obj");
    if (loadedCar && loadedRoad)
    {
        GLuint textureCar = loadTexture("Textures/initialD.png");
        GLuint textureRoad = loadTexture("Textures/roadtex.png");

        texturedMeshList.push_back({car, textureCar});
        texturedMeshList.push_back({road, textureRoad});
    }
    else
    {
        std::cout<<"Failed to load model"<<std::endl;
    }
}

void CreateTriangle()
{
    GLfloat vertices[] =
    {   //pos                   //aTexCoord
        -1.0f, -1.0f, 0.0f,     0.0f, 0.0f,
        0.0f, -1.0f, 1.0f,      0.5f, 0.0f,
        1.0f, -1.0f, 0.0f,      1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,       0.5f, 1.0f,
    };

    unsigned int indices[] = 
    {
        0, 3, 1,
        1, 3, 2,
        2, 3, 0,
        0, 1, 2
    };

    Mesh *obj1 = new Mesh();
    obj1->CreateMesh(vertices, indices, 20, 12);
    for (size_t i = 0; i < 10; i++)
    {
        meshList.push_back(obj1);
    }
    
}

void CreateShaders()
{
    Shader* shader1 = new Shader();
    shader1->CreateFromFiles(vShader, fShader);
    shaderList.push_back(*shader1);
} 

void checkMouse()
{
    double xpos, ypos;
    glfwGetCursorPos(mainWindow.getWindow(), &xpos, &ypos);

    // std::cout << xpos << " " << ypos << std::endl;

    // set to xpos since first and second time running, and else loops, remains static
    static float lastX = xpos;
    static float lastY = ypos;

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;

    lastX = xpos;
    lastY = ypos;

    float sensitivity = 0.2f;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    // std::cout << xoffset << yoffset << std::endl;
}

int main()
{
    mainWindow = Window(WIDTH, HEIGHT, 3, 3);
    mainWindow.initialise();

    // CreateTriangle();
    CreateOBJ();
    CreateShaders();

    GLuint uniformModel = 0; // int มีหน้าที่ชี้บอกว่าตัวที่ชื่อว่า model texture คือตัวไหน
    GLuint uniformProjection = 0;

    GLuint uniformView = 0;
    
    //for secret room 3 enter - https://forms.gle/U9VE4pkYAPNvUW1H9 
    glm::mat4 projection = glm::perspective(45.0f, (GLfloat) mainWindow.getBufferWidth() / (GLfloat) mainWindow.getBufferHeight(), 1.0f, 200.0f);
    
    glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
    // glm::mat4 projection = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.1f, 100.0f);
    
    // glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 cameraTarget = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);

    glm::vec3 cameraDirection = glm::normalize(cameraTarget - cameraPos);
    glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, up));
    glm::vec3 cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

    //texture
    unsigned int texture;
    glGenTextures(1, &texture);

    //start use texture on board
    glBindTexture(GL_TEXTURE_2D, texture);
    //stop use it
    // glBindTexture(GL_TEXTURE_2D, 0);
    float borderColor[] = {0, 0, 0, 1};
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    int width, height, nrChannels;
    // stbi_set_flip_vertically_on_load(true);
//     unsigned char* data = stbi_load("Textures/Dtex.png", &width, &height, &nrChannels, 0);
//     // unsigned char* data = stbi_load("Textures/lungtoo.jpg", &width, &height, &nrChannels, 0);
// 
//     if (data) {
//         // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
//     glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); //<< for png
//         glGenerateMipmap(GL_TEXTURE_2D);
//     }
//     else std::cout << "Failed to load texture" << std::endl;
//     stbi_image_free(data);

    float deltaTime, lastFrame;
    //Loop until window closed
    while (!mainWindow.getShouldClose())
    {

        
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        //Get + Handle user input events
        glfwPollEvents();
        
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_W) == GLFW_PRESS) {
            cameraPos.z += (-2.0f) * deltaTime;
        };
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_S) == GLFW_PRESS) {
            cameraPos.z += (2.0f) * deltaTime;
        };
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_A) == GLFW_PRESS) {
            cameraPos.x += (-2.0f) * deltaTime;
        };
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_D) == GLFW_PRESS) {
            cameraPos.x += (2.0f) * deltaTime;
        };
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_Z) == GLFW_PRESS) {
            cameraPos.y += (2.0f) * deltaTime;
        };
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_X) == GLFW_PRESS) {
            cameraPos.y += (-2.0f) * deltaTime;
        };

        //Clear window
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //draw here
        shaderList[0].UseShader();
        uniformView = shaderList[0].GetUniformLocation("view");
        uniformModel = shaderList[0].GetUniformLocation("model"); //get location
        uniformProjection = shaderList[0].GetUniformLocation("projection");

        
        glm::mat4 view (1.0f);
        
        checkMouse();

        glm::vec3 direction;
        direction.x = cos(glm::radians(pitch)) * cos(glm::radians(yaw));
        direction.y = sin(glm::radians(pitch));
        direction.z = cos(glm::radians(pitch)) * sin(glm::radians(yaw));

        cameraDirection = glm::normalize(direction);
        cameraRight = glm::normalize(glm::cross(cameraDirection, up));
        cameraUp = glm::normalize(glm::cross(cameraRight, cameraDirection));

        //         glm::mat4 cameraPosMat (1.0f);
        //         cameraPosMat[3][0] = -cameraPos.x;
        //         cameraPosMat[3][1] = -cameraPos.y;
        //         cameraPosMat[3][2] = -cameraPos.z;
        // 
        //         glm::mat4 cameraRotateMat(1.0f);
        //         cameraRotateMat[0] = glm::vec4(cameraRight.x, cameraUp.x, -cameraDirection.x, 0.0f);
        //         cameraRotateMat[1] = glm::vec4(cameraRight.y, cameraUp.y, -cameraDirection.y, 0.0f);
        //         cameraRotateMat[2] = glm::vec4(cameraRight.z, cameraUp.z, -cameraDirection.z, 0.0f);
        //         view = cameraRotateMat * cameraPosMat;

                //nah, about is just summarized in this one line lib:
        view = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);

        glm::vec3 pyramidPositions[] = 
        {
            glm::vec3(-10.0f, 0.38f, 4.0f),
            glm::vec3( 0.0f, 0.0f, 0.0f),
        }; // define position of each model here
        //Object

        for (size_t i = 0; i < texturedMeshList.size(); i++)
        {

            glm::mat4 model (1.0f);

            model = glm::translate(model, pyramidPositions[i]);
            // model = glm::translate(model, pyramidPositions[0]);
            if (i == 0) model = glm::rotate(model, glm::radians(2.0f * i), glm::vec3(1.0f, 0, 0));
            

            glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
            glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
            glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
            //light
            glUniform3fv(shaderList[0].GetUniformLocation("lightPos"), 1, (GLfloat *)&lightPos);
                    //texture level 0 v
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, texturedMeshList[i].textureID);
            texturedMeshList[i].mesh->RenderMesh();
            // meshList[0]->RenderMesh();
        };
        
        glUniform3fv(shaderList[0].GetUniformLocation("lightColour"), 1, (GLfloat  *)&lightColour);

        glUseProgram(0);
        //end draw

        //magic word - SAKURA

        mainWindow.swapBuffers();
    }

    return 0;
}
