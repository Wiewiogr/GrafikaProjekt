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

    // Poor filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA, width, height, 0,GL_RGBA, GL_UNSIGNED_BYTE, data);
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

    glDisable(GL_DITHER);
    glDisable(GL_POINT_SMOOTH);
    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);
    glHint(GL_POINT_SMOOTH, GL_DONT_CARE);
    glHint(GL_LINE_SMOOTH, GL_DONT_CARE);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_DONT_CARE);
    glDisable( GL_MULTISAMPLE_ARB) ;

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
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

//    // Cull triangles which normal is not towards the camera
//    glEnable(GL_CULL_FACE);

    GLuint programID = LoadShaders( "StandardShadingRTT.vertexshader", "StandardShadingRTT.fragmentshader" );
    GLuint quad_programID = LoadShaders( "Passthrough.vertexshader", "WobblyTexture.fragmentshader" );
    // ---------------------------------------------
    // Render to Texture - specific code begins here
    // ---------------------------------------------
    // The framebuffer, which regroups 0, 1, or more textures, and 0 or 1 depth buffer.
    //
    // The texture we're going to render to

    GLubyte* data = (GLubyte*)malloc(windowWidth*windowHeight*4*sizeof(GLubyte));
    int val;
    for( int i = 0; i < windowWidth*windowHeight; i++)
    {
        if ((rand() % 100)> 50 ) val = 255; else val = 0;
       // val = rand() % 255;
        data[i*4] = data[i*4+1] = data[i*4+2] = val;
        data[i*4+3]= 255;
    }

//    for(int i = 0; i < windowWidth*windowHeight*3; i++)
//    {
//        printf("[%d] : %d\n",i, data[i]);
//    }

//    GLenum DrawBuffers[2] = {GL_COLOR_ATTACHMENT0_EXT,GL_COLOR_ATTACHMENT1_EXT};
//    glDrawBuffers(2, DrawBuffers); // "1" is the size of DrawBuffers
//    /////start
//    GLuint FboID;
//    GLuint TexID[2];
//    int CurrActiveBuffer = 0;  // Current active buffer index
//
//    glGenFramebuffersEXT( 1, &FboID );
//    glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, FboID );
//
//    // Create 2 textures for input/output.
//    glGenTextures( 2, TexID );
//
//    for( int i=0; i<2; i++ )
//    {
//        glBindTexture( GL_TEXTURE_2D, TexID[i] );
//        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
//        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
//        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
//        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
//        // RGBA8 buffer
//        if( i == 0)
//            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );
//        else
//            glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, windowWidth, windowHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, data );
//
//        //if( _hasMipmapping )  glGenerateMipmapEXT( GL_TEXTURE_2D );
//    }
//
//    // Now attach textures to FBO
//    int src = CurrActiveBuffer;
//    int dest = 1-CurrActiveBuffer;
//    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, 
//            GL_COLOR_ATTACHMENT0_EXT, 
//            GL_TEXTURE_2D, TexID[src], 0 );
//    glFramebufferTexture2DEXT( GL_FRAMEBUFFER_EXT, 
//            GL_COLOR_ATTACHMENT1_EXT, 
//            GL_TEXTURE_2D, TexID[dest], 0 );

    ///end
    GLuint front = createTexture(windowWidth, windowHeight, 0);
    GLuint back = createTexture(windowWidth, windowHeight, data);

    GLuint frontFBO = 0;
    glGenFramebuffers(1, &frontFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, frontFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, front, 0);

    GLuint backFBO = 0;
    glGenFramebuffers(1, &backFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, backFBO);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, back, 0);

    // Set the list of draw buffers.

    // Always check that our framebuffer is ok
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        return false;
    // The fullscreen quad's FBO
    static const GLfloat g_quad_vertex_buffer_data[] = { 
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,

//        -1.0f, -1.0f, 0.0f,
//         3.0f, -1.0f, 0.0f,
//        -1.0f,  3.0f, 0.0f,

//        -1.0f, -1.0f, 0.0f,
//         1.0f, -1.0f, 0.0f,
//        -1.0f,  1.0f, 0.0f,
//        -1.0f,  1.0f, 0.0f,
//         1.0f, -1.0f, 0.0f,
//         1.0f,  1.0f, 0.0f,
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
//        int src = CurrActiveBuffer;
//        int dest = 1-CurrActiveBuffer;
//        {
//            glDrawBuffer(DrawBuffers[dest]);
//            glBindFramebuffer(GL_FRAMEBUFFER, FboID);
//
//            // Render on the whole framebuffer, complete from the lower left corner to the upper right
//
//            glViewport(0,0,windowWidth,windowHeight);
//            // Clear the screen
//            glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//            // Use our shader
//            glUseProgram(quad_programID);
//
//            // Bind our texture in Texture Unit 0
//            glActiveTexture(GL_TEXTURE0 +src);
//            glBindTexture(GL_TEXTURE_2D, TexID[src]);
//            // Set our "front" sampler to user Texture Unit 0
//            glUniform1i(texID, 0);
//
//            number %= 100;
//            glUniform1f(timeID, (float)((number++)/100.0) );
//            //glDrawBuffer(DrawBuffers[dest]);
//            glEnableVertexAttribArray(0);
//            glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
//            glVertexAttribPointer(
//              0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
//              3,                  // size
//              GL_FLOAT,           // type
//              GL_FALSE,           // normalized?
//              0,                  // stride
//              (void*)0            // array buffer offset
//            );
//
//            // Draw the triangles !
//            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 2*3 indices starting at 0 -> 2 triangles
//
//            glDisableVertexAttribArray(0);
//            glUseProgram(0);
//        }
//
//        glBindFramebuffer(GL_FRAMEBUFFER, 0);
//        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//        glUseProgram(programID);
//
//        // Bind our texture in Texture Unit 0
//        glActiveTexture(GL_TEXTURE0+ src );
//        glBindTexture(GL_TEXTURE_2D, TexID[src]);
//        // Set our "front" sampler to user Texture Unit 0
//        glUniform1i(texID, 0);
//
////        glActiveTexture(GL_TEXTURE0);
////        glBindTexture(GL_TEXTURE_2D, TexID[src]);
//        glEnableVertexAttribArray(0) ;
//        glBindBuffer(GL_ARRAY_BUFFER, quad_vertexbuffer);
//        glVertexAttribPointer(
//            0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
//            3,                  // size
//            GL_FLOAT,           // type
//            GL_FALSE,           // normalized?
//            0,                  // stride
//            (void*)0            // array buffer offset
//        );
//
//        // Draw the triangles !
//        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 2*3 indices starting at 0 -> 2 triangles
//
//        glDisableVertexAttribArray(0);
//        // Swap buffers
//        glfwSwapBuffers(window);
//        glfwPollEvents();
//        CurrActiveBuffer = 1 - CurrActiveBuffer;

      // Render to the screen
        if(number++ % 50 == 0)
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
            glBindTexture(GL_TEXTURE_2D, back);
            // Set our "front" sampler to user Texture Unit 0
            glUniform1i(texID, 0);

            number %= 100;
            glUniform1f(timeID, 0.5);
            //glUniform1f(timeID, (float)((number++)/100.0) );
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
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 2*3 indices starting at 0 -> 2 triangles

            glDisableVertexAttribArray(0);
            glUseProgram(0);
        }
        else
        {

            printf("dupa\n");
            glBindFramebuffer(GL_FRAMEBUFFER, backFBO);
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
            glUniform1i(backID, 0);

            number %= 100;
            glUniform1f(timeID, 2);
            //glUniform1f(timeID, (float)((number++)/100.0) );
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

            // Draw the triangles 4
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 2*3 indices starting at 0 -> 2 triangles

            glDisableVertexAttribArray(0);
            glUseProgram(0);

        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(programID);

        // Bind our texture in Texture Unit 0
//        glActiveTexture(GL_TEXTURE0);
//        glBindTexture(GL_TEXTURE_2D, back);
        // Set our "front" sampler to user Texture Unit 0
        glUniform1i(texID, 0);

        glActiveTexture(GL_TEXTURE0);

        if(counter % 2 == 1)
            glBindTexture(GL_TEXTURE_2D, front);
        else
            glBindTexture(GL_TEXTURE_2D, back);

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
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); // 2*3 indices starting at 0 -> 2 triangles

        glDisableVertexAttribArray(0);
        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader

    glDeleteFramebuffers(1, &frontFBO);
    glDeleteTextures(1, &texID);
    //glDeleteTextures(2, TexID);
    glDeleteBuffers(1, &quad_vertexbuffer);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

