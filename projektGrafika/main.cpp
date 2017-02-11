// Include standard headers
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
    // "Bind" the newly created texture : all future texture functions will modify this texture
    glBindTexture(GL_TEXTURE_2D, renderedTexture);

    // Give an empty image to OpenGL ( the last "0" means "empty" )

    // Poor filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST); 
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, width, height, 0,GL_RGB, GL_UNSIGNED_BYTE, data);
    // Set "renderedTexture" as our colour attachement #0
    return renderedTexture;
}

int main( void )
{
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
    window = glfwCreateWindow( 1024, 768, "Tutorial 14 - Render To Texture", NULL, NULL);
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
    // But on MacOS X with a retina screen it'll be 1024*2 and 768*2, so we get the actual framebuffer size:
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
    // Hide the mouse and enable unlimited mouvement
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint programID = LoadShaders( "StandardShadingRTT.vertexshader", "StandardShadingRTT.fragmentshader" );
    GLuint quad_programID = LoadShaders( "Passthrough.vertexshader", "WobblyTexture.fragmentshader" );
    // ---------------------------------------------
    // Render to Texture - specific code begins here
    // ---------------------------------------------

    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    // The texture we're going to render to

    GLubyte a;
    GLubyte* data = (GLubyte*)malloc(windowWidth*windowHeight*4*sizeof(GLubyte));
    int val;
    for( int i = 0; i < windowWidth*windowHeight; i++)
    {
       // if ((rand() % 100)> 50 ) val = 200; else val = 10;
        val = rand() % 255;
        data[i*3] = data[i*3+1] = data[i*3+2] = val;
    }

//    for(int i = 0; i < windowWidth*windowHeight*3; i++)
//    {
//        printf("[%d] : %d\n",i, data[i]);
//    }
    GLuint front = createTexture(windowWidth, windowHeight, 0);
    GLuint back = createTexture(windowWidth, windowHeight, 0);

    GLuint frontFBO = 0;
    glGenFramebuffers(1, &frontFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, frontFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, front, 0);

    GLuint backFBO = 0;
    glGenFramebuffers(1, &backFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, backFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, back, 0);

//    // Set the list of draw buffers.
//    GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
//    glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;

    // The fullscreen quad's FBO
    static const GLfloat g_quad_vertex_buffer_data[] = { 
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
    };

    GLuint quad_vertexbuffer;
    glGenBuffers(1, &quad_vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_quad_vertex_buffer_data), g_quad_vertex_buffer_data, GL_STATIC_DRAW);

    GLuint backID = glGetUniformLocation(quad_programID, "back");
    GLuint texID = glGetUniformLocation(quad_programID, "front");
    GLuint timeID = glGetUniformLocation(quad_programID, "time");

    int number = 0;
    int counter = 0;
    do{
        // Render to the screen
        if(counter++ % 2 == 0)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, frontFBO);
            // Render on the whole framebuffer, complete from the lower left corner to the upper right
            glViewport(0,0,windowWidth,windowHeight);

            // Clear the screen
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Use our shader
            glUseProgram(quad_programID);

            // Bind our texture in Texture Unit 0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, front);
            // Set our "front" sampler to user Texture Unit 0
            glUniform1i(texID, 0);

            number %= 100;
            glUniform1f(timeID, (float)((number++)/100.0) );
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
            glVertexAttribPointer(
              0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
              3,                  // size
              GL_FLOAT,           // type
              GL_FALSE,           // normalized?
              0,                  // stride
              (void*)0            // array buffer offset
            );

            // Draw the triangles !
            glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

            glDisableVertexAttribArray(0);
            glUseProgram(0);
        }
        else
        {
            glBindFramebuffer(GL_FRAMEBUFFER, backFBO);
            // Render on the whole framebuffer, complete from the lower left corner to the upper right
            glViewport(0,0,windowWidth,windowHeight);

            // Clear the screen
            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Use our shader
            glUseProgram(quad_programID);

            // Bind our texture in Texture Unit 0
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, back);
            // Set our "front" sampler to user Texture Unit 0
            glUniform1i(backID, 0);

            number %= 100;
            glUniform1f(timeID, (float)((number++)/100.0) );
            glEnableVertexAttribArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
            glVertexAttribPointer(
                0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
                3,                  // size
                GL_FLOAT,           // type
                GL_FALSE,           // normalized?
                0,                  // stride
                (void*)0            // array buffer offset
            );

            // Draw the triangles !
            glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

            glDisableVertexAttribArray(0);
            glUseProgram(0);

        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, back);
        // Set our "front" sampler to user Texture Unit 0
        glUniform1i(texID, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, front);
        glEnableVertexAttribArray(0) ;
       glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
        glVertexAttribPointer(
            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,                  // size
            GL_FLOAT,           // type
            GL_FALSE,           // normalized?
            0,                  // stride
            (void*)0            // array buffer offset
        );

        // Draw the triangles !
        glDrawArrays(GL_TRIANGLES, 0, 6); // 2*3 indices starting at 0 -> 2 triangles

        glDisableVertexAttribArray(0);
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader

    glDeleteFramebuffers(1, &frontFBO);
    glDeleteTextures(1, &front);
    glDeleteBuffers(1, &quad_vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

