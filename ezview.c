#define GLFW_DLL 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>

#include "linmath.h"
#include "ppmr.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct {
  float Position[2];
  float TexCoord[2];
} Vertex;

typedef struct {
  float scale;
  float rotate;
  float translate[2]; // 0 -> x, 1 -> y
  float shear[2];     // 0 -> x, 1 -> y
} transvals;

// (-1, 1)  (1, 1)
// (-1, -1) (1, -1)
const GLubyte Indices[] = {
  0, 1, 2,
  2, 3, 0
};

transvals *trans;

Vertex vertexes[] = {
  {{1, -1}, {0.99999, 0.99999}},
  {{1, 1},  {0.99999, 0}},
  {{-1, 1}, {0, 0}},
  {{-1, -1}, {0, 0.99999}}
};

static const char* vertex_shader_text =
"uniform mat4 MVP;\n"
"attribute vec2 TexCoordIn;\n"
"attribute vec2 vPos;\n"
"varying lowp vec2 TexCoordOut;\n"
"void main()\n"
"{\n"
"    gl_Position = MVP * vec4(vPos, 0.0, 1.0);\n"
"    TexCoordOut = TexCoordIn;\n"
"}\n";

static const char* fragment_shader_text =
"varying lowp vec2 TexCoordOut;\n"
"uniform sampler2D Texture;\n"
"void main()\n"
"{\n"
"    gl_FragColor = texture2D(Texture, TexCoordOut);\n"
"}\n";

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);

    //Translation
    if (key == GLFW_KEY_W && action == GLFW_PRESS)
      trans[0].translate[1] += 0.1;

    if (key == GLFW_KEY_A && action == GLFW_PRESS)
    	trans[0].translate[0] -= 0.1;

    if (key == GLFW_KEY_S && action == GLFW_PRESS)
      trans[0].translate[1] -= 0.1;

    if (key == GLFW_KEY_D && action == GLFW_PRESS)
      trans[0].translate[0] += 0.1;

    // Scale
    if (key == GLFW_KEY_R && action == GLFW_PRESS)
      trans[0].scale *= 1.1;

    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    	trans[0].scale /= 1.1;

    // Rotate Left
    if (key == GLFW_KEY_Q && action == GLFW_PRESS)
      trans[0].rotate += (90 * 3.141592) / 180;

    if (key == GLFW_KEY_E && action == GLFW_PRESS)
      trans[0].rotate -= (90 * 3.141592) / 180;

    // Shear
    if (key == GLFW_KEY_UP && action == GLFW_PRESS)
    	trans[0].shear[1] += 0.1;

    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS)
    	trans[0].shear[1] -= 0.1;

    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS)
    	trans[0].shear[0] -= 0.1;

    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS)
      trans[0].shear[0] += 0.1;

}

void glCompileShaderOrDie(GLuint shader) {
  GLint compiled;
  glCompileShader(shader);
  glGetShaderiv(shader,
		GL_COMPILE_STATUS,
		&compiled);
  if (!compiled) {
    GLint infoLen = 0;
    glGetShaderiv(shader,
		  GL_INFO_LOG_LENGTH,
		  &infoLen);
    char* info = malloc(infoLen+1);
    GLint done;
    glGetShaderInfoLog(shader, infoLen, &done, info);
    printf("Unable to compile shader: %s\n", info);
    exit(1);
  }
}

int main(int argc, char *argv[])
{

  if (argc != 2)
  {
    fprintf(stderr, "Error: Usage ezview input.ppm\n", argc);
    exit(1);
  }

    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program, index_buffer;
    GLint mvp_location, vpos_location, vcol_location;
    Pixel *buffer;
    int        iw, ih, cMax, version;

    FILE* fr = fopen(argv[1], "r"); // File Read
    //Check if input file exists
    if(fr == NULL)
      {
      fprintf(stderr, "%s\n", "Error: input file type not found.");
      return(1);
      }
    if(parseH(fr,&iw,&ih,&cMax,&version))
    {
      fprintf(stderr, "Error: Header parsing unsuccessful\n", argc);
      exit(1);
    }

    buffer = malloc(sizeof(Pixel) * iw * ih);

    if (version == 3)
    {
      readP3(fr, buffer, &iw, &ih, &cMax);
    }
    else
    {
      readP6(fr, buffer, &iw, &ih, &cMax);
    }
    fclose(fr);


    //GLFW SETUP
    glfwSetErrorCallback(error_callback);

    if (!glfwInit()) {
      fprintf(stderr, "Could not initialize GLFW.\n");
      exit(1);
    }

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "EZVIEW", NULL, NULL);
    if (!window)
    {
      fprintf(stderr, "Window init faild.\n");
      glfwTerminate();
      exit(1);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);
    ////
    glGenBuffers(1, &index_buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);

    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_shader_text, NULL);
    glCompileShaderOrDie(vertex_shader);

    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_shader_text, NULL);
    glCompileShaderOrDie(fragment_shader);

    program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    mvp_location = glGetUniformLocation(program, "MVP");
    assert(mvp_location != -1);

    vpos_location = glGetAttribLocation(program, "vPos");
    assert(vpos_location != -1);

    GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
    assert(texcoord_location != -1);

    GLint tex_location = glGetUniformLocation(program, "Texture");
    assert(tex_location != -1);

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*) 0);

    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location,2,GL_FLOAT,GL_FALSE,sizeof(Vertex),(void*) (sizeof(float) * 2));

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, iw, ih, 0, GL_RGB,
		 GL_UNSIGNED_BYTE, buffer);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(tex_location, 0);


    trans =(transvals *) malloc(sizeof(transvals));
    trans[0].translate[0] = 0.0;
    trans[0].translate[1] = 0.0;
    trans[0].shear[0] = 0.0;
    trans[0].shear[1] = 0.0;
    trans[0].rotate = 0.0;
    trans[0].scale = 1.0;

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 translate, rotate, scale, shear, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);
//DO STUFFS
        // Set the Transformation matrices
        mat4x4_identity(translate);
        mat4x4_translate(translate, trans[0].translate[0], trans[0].translate[1], 0);

        mat4x4_identity(rotate);
        mat4x4_rotate_Z(rotate, rotate, trans[0].rotate);

        mat4x4_identity(scale);
        scale[0][0] = scale[0][0] * trans[0].scale;
        scale[1][1] = scale[1][1] * trans[0].scale;

        mat4x4_identity(shear);
        shear[1][0] = trans[0].shear[0];
        shear[0][1] = trans[0].shear[1];

        mat4x4_mul(mvp,translate, rotate);
        mat4x4_mul(mvp,mvp, scale);
        mat4x4_mul(mvp,mvp, shear);



        glUseProgram(program);
        glUniformMatrix4fv(mvp_location, 1, GL_FALSE, (const GLfloat*) mvp);
        //glDrawArrays(GL_TRIANGLES, 0, 3);

        glDrawElements(GL_TRIANGLES,
                       sizeof(Indices) / sizeof(GLubyte),
                       GL_UNSIGNED_BYTE, 0);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

//! [code]
