// Include standard headers
#include <ctype.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>


GLuint createTexture(int width, int height, GLubyte * data)
{
    GLuint renderedTexture;
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &renderedTexture);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0,GL_RGBA, GL_UNSIGNED_BYTE, data);
    return renderedTexture;
}

void drawQuadVertexBuffer(GLuint vertexBuffer)
{
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer);
    glVertexAttribPointer(
      0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
      3,                  // size
      GL_FLOAT,           // type
      GL_FALSE,           // normalized?
      0,                  // stride
      (void*)0            // array buffer offset
    );
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 2*3 indices starting at 0 -> 2 triangles
    glDisableVertexAttribArray(0);
}

int main( int argc, char* argv[] )
{
    int c;

    glm::mat3 aliveCondition(0);
    glm::mat3 deathCondition(0);
    int scale = 1;
    while ((c = getopt (argc, argv, "a:d:s:")) != -1)
    {
        switch (c)
        {
        case 'a':
            for(int i = 0 ; i < strlen(optarg); i++)
            {
                int number = optarg[i]-48;
                aliveCondition[number/3][number%3] = 1;
            }
            break;
        case 'd':
            for(int i = 0 ; i < strlen(optarg); i++)
            {
                int number = optarg[i]-48;
                deathCondition[number/3][number%3] = 1;
            }
            break;
        case 's':
            scale = atoi(optarg);
            break;
        }
    }

    for(int i = 0 ; i < 9; i++)
    {
        printf("%d : %lf\n", i, aliveCondition[i/3][i%3]);
    }
    for(int i = 0 ; i < 9; i++)
    {
        printf("%d : %lf\n", i, deathCondition[i/3][i%3]);
    }
    printf("scale : %d\n",scale);

    srand( time( NULL ) );
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "Game of life", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // We would expect width and height to be 1024 and 768
    int windowWidth = 1024;
    int windowHeight = 768;

    glfwGetFramebufferSize(window, &windowWidth, &windowHeight);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);
    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Set the mouse at the center of the screen
    glfwPollEvents();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    GLuint programID = LoadShaders( "Passthrough.vertexshader", "Passthrough.fragmentshader" );
    GLuint quad_programID = LoadShaders( "GameOfLife.vertexshader", "GameOfLife.fragmentshader" );

    GLubyte* data = (GLubyte*)malloc(windowWidth*windowHeight*4*sizeof(GLubyte));
    int val;
    for( int i = 0; i < windowWidth*windowHeight; i++)
    {
        if ((rand() % 100)> 50 ) val = 255; else val = 0;
        data[i*4] = data[i*4+1] = data[i*4+2] = val;
        data[i*4+3]= 255;
    }

    GLuint textures[2];
    textures[0] = createTexture(windowWidth, windowHeight, 0);
    textures[1] = createTexture(windowWidth, windowHeight, data);

    GLuint FBOs[2];
    glGenFramebuffers(1, &FBOs[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, FBOs[0]);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[0], 0);

    glGenFramebuffers(1, &FBOs[1]);
    glBindFramebuffer(GL_FRAMEBUFFER, FBOs[1]);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, textures[1], 0);

    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;
    // The fullscreen quad's FBO
    static const GLfloat g_quad_vertex_buffer_data[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
    };

    GLuint quad_vertexbuffer;
    glGenBuffers(1, &quad_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

    GLuint textureID = glGetUniformLocation(quad_programID, "front");
    GLuint aliveConditionID = glGetUniformLocation(quad_programID, "aliveCondition");
    GLuint deathConditionID = glGetUniformLocation(quad_programID, "deathCondition");
    GLuint timeID = glGetUniformLocation(quad_programID, "time");
    GLuint scaleID = glGetUniformLocation(programID, "scale");

    int currentTexture = 0;
    bool isActive = true;
    int number = 0;
    do{
        if((++number % 10) == 0 && isActive)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, FBOs[currentTexture]);
            glViewport(0,0,windowWidth,windowHeight);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glUseProgram(quad_programID);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, textures[!currentTexture]);

            // Set our "front" sampler to user Texture Unit 0
            glUniformMatrix3fv(aliveConditionID, 1, GL_FALSE, &aliveCondition[0][0]);
            glUniformMatrix3fv(deathConditionID, 1, GL_FALSE, &deathCondition[0][0]);
            glUniform1i(textureID, 0);

            number %= 100;
            glUniform1f(timeID, 0.5);
            drawQuadVertexBuffer(quad_vertexbuffer);
            glUseProgram(0);
            currentTexture = 1 - currentTexture;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);
        glUniform1f(scaleID, float(1.0/scale));
        glUniform1i(textureID, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textures[!currentTexture]);

        drawQuadVertexBuffer(quad_vertexbuffer);

        glfwSwapBuffers(window);
        glfwPollEvents();
        if(glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            isActive = !isActive;
            printf("space pressed\n");
        }

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    glDeleteFramebuffers(2, FBOs);
    glDeleteTextures(2, textures);
    glDeleteTextures(1, &textureID);
    glDeleteBuffers(1, &quad_vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);

    glfwTerminate();

    return 0;
}

