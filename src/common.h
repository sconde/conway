/*******************************************************
 * Copyright (c) 2014, ArrayFire
 * All rights reserved.
 *
 * This file is distributed under 3-clause BSD license.
 * The complete license agreement can be obtained at:
 * http://arrayfire.com/licenses/BSD-3-Clause
 *
 * Authored by Shehzan Mohammed, ArrayFire
 ********************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <arrayfire.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

static const int game_w = 256;
static const int game_h = 256;
static const int buff_w = game_w;
static const int buff_h = game_h;
static const int disp_w = game_w * 4;
static const int disp_h = game_h * 4;

//OpenGL PBO and texture "names"
GLuint gl_PBO, gl_Tex, gl_Shader;

static GLFWwindow* window = NULL;

af::array state;

static const float h_kernel[] = {1, 1, 1, 1, 0, 1, 1, 1, 1};

// gl_Shader for displaying floating-point texture
static const char *shader_code =
    "!!ARBfp1.0\n"
    "TEX result.color, fragment.texcoord, texture[0], 2D; \n"
    "END";

GLuint compileASMShader(GLenum program_type, const char *code)
{
    GLuint program_id;
    glGenProgramsARB(1, &program_id);
    glBindProgramARB(program_type, program_id);
    glProgramStringARB(program_type, GL_PROGRAM_FORMAT_ASCII_ARB, (GLsizei) strlen(code), (GLubyte *) code);

    GLint error_pos;
    glGetIntegerv(GL_PROGRAM_ERROR_POSITION_ARB, &error_pos);

    if (error_pos != -1)
    {
        const GLubyte *error_string;
        error_string = glGetString(GL_PROGRAM_ERROR_STRING_ARB);
        fprintf(stderr, "Program error at position: %d\n%s\n", (int)error_pos, error_string);
        return 0;
    }

    return program_id;
}

static void error_callback(int error, const char* description)
{
    fputs(description, stderr);
}

static void key_callback(GLFWwindow* wind, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(wind, GL_TRUE);
    }
}

void
initGLFW(const int buffer_width, const int buffer_height,
          const int buffer_depth)
{
    if (window == NULL) {
        glfwSetErrorCallback(error_callback);
        if (!glfwInit()) {
            std::cerr << "ERROR: GLFW wasn't able to initalize" << std::endl;
            exit(EXIT_FAILURE);
        }

        if(buffer_depth <=0 || buffer_depth > 4) {
            std::cerr << "ERROR: Depth value must be between 1 and 4" << std::endl;
        }

        glfwWindowHint(GLFW_DEPTH_BITS, buffer_depth * 8);
        glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
        window = glfwCreateWindow(buffer_width, buffer_height,
                                  "Conway's Game of Life Using ArrayFire", NULL, NULL);
        if (!window) {
            glfwTerminate();
            //Comment/Uncomment these lines incase using fall backs
            //return;
            std::cerr << "ERROR: GLFW couldn't create a window." << std::endl;
            exit(EXIT_FAILURE);
        }

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);
        int b_width = buffer_width;
        int b_height = buffer_height;
        int b_depth = buffer_depth;
        glfwGetFramebufferSize(window, &b_width, &b_height);
        glfwSetTime(0.0);

        glfwSetKeyCallback(window, key_callback);

        //GLEW Initialization - Must be done
        GLenum res = glewInit();
        if (res != GLEW_OK) {
            std::cerr << "Error Initializing GLEW | Exiting" << std::endl;
            exit(-1);
        }
        //Put in resize
        glViewport(0, 0, b_width, b_height);
    }
}

void initOpenGL()
{
    printf("Creating GL texture...\n");
    glEnable(GL_TEXTURE_2D);
    glGenTextures(1, &gl_Tex);
    glBindTexture(GL_TEXTURE_2D, gl_Tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, buff_w, buff_h, 0, GL_RED, GL_FLOAT, NULL);
    printf("Texture created.\n");

    printf("Creating PBO...\n");
    glGenBuffers(1, &gl_PBO);
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, gl_PBO);
    glBufferData(GL_PIXEL_UNPACK_BUFFER_ARB, buff_w * buff_h * sizeof(float), NULL, GL_STREAM_COPY);

    printf("PBO created.\n");

    // load shader program
    gl_Shader = compileASMShader(GL_FRAGMENT_PROGRAM_ARB, shader_code);
}

void update(const int frame_count)
{
    // Initialize the kernel array just once
    static const af::array kernel(3, 3, h_kernel, af::afHost);
    static const int reset = 600;

    // Generate a random starting state
    if(frame_count % reset == 0)
        state = (af::randu(game_w, game_h, f32) > 0.33).as(f32);

    // Convolve gets neighbors
    af::array nHood = convolve(state, kernel, false);

    // Generate conditions for life
    // state == 1 && nHood < 2 ->> state = 0
    // state == 1 && nHood > 3 ->> state = 0
    // else if state == 1 ->> state = 1
    // state == 0 && nHood == 3 ->> state = 1
    af::array C0 = (nHood == 2);
    af::array C1 = (nHood == 3);

    // Update state
    state = state * C0 + C1;
}

void draw()
{
    // load texture from PBO
    glBindTexture(GL_TEXTURE_2D, gl_Tex);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, buff_w, buff_h, GL_RED, GL_FLOAT, 0);

    // fragment program is required to display floating point texture
    glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, gl_Shader);
    glEnable(GL_FRAGMENT_PROGRAM_ARB);
    glDisable(GL_DEPTH_TEST);

    // Draw to screen
    // GLFW uses -1 to 1 normalized coordinates
    // Textures go from 0 to 1 normalized coordinates
    glBegin(GL_QUADS);
    glTexCoord2f ( 0.0f,  0.0f);
    glVertex2f   (-1.0f, -1.0f);
    glTexCoord2f ( 1.0f,  0.0f);
    glVertex2f   ( 1.0f, -1.0f);
    glTexCoord2f ( 1.0f,  1.0f);
    glVertex2f   ( 1.0f,  1.0f);
    glTexCoord2f ( 0.0f,  1.0f);
    glVertex2f   (-1.0f,  1.0f);
    glEnd();

    // Unbind textures
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_FRAGMENT_PROGRAM_ARB);

    // Complete render
    glfwSwapBuffers(window);
    glfwPollEvents();
}

void copyArrayToPBO();

void run(int &count)
{
#if 1
            update(count);

            copyArrayToPBO();

            draw();
#else
            af::timer s = af::timer::start();
            af::timer t = af::timer::start();
            update(count);
            double utime = af::timer::stop(t);

            t = af::timer::start();
            copyArrayToPBO();
            double ctime = af::timer::stop(t);

            t = af::timer::start();
            draw();
            double dtime = af::timer::stop(t);
            double ttime = af::timer::stop(s);

            printf("Total Time = %f\t Update Time = %f\t Copy Time = %f\t Draw Time = %f\n", ttime, utime, ctime, dtime);
#endif
            count++;
}

void cleanup()
{
    // Cleanup
    glBindBuffer(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

    glDeleteBuffers(1, &gl_PBO);
    glDeleteTextures(1, &gl_Tex);
    glDeleteProgramsARB(1, &gl_Shader);
    glfwTerminate();
}

