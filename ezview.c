#define GLFW_DLL 1

#define GL_GLEXT_PROTOTYPES
#include <GLES2/gl2.h>
//#include <OpenGL/gl.h>
#include <GLFW/glfw3.h>

#include "linmath.h"

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

typedef struct Pixel
  {
  unsigned char r, g, b;
  } Pixel;

typedef struct {
  float Position[2];
  float TexCoord[2];
} Vertex;

// (-1, 1)  (1, 1)
// (-1, -1) (1, -1)
const GLubyte Indices[] = {
  0, 1, 2,
  2, 3, 0
};

Vertex vertexes[] = {
  {{1, -1}, {0.99999, 0}},
  {{1, 1},  {0.99999, 0.99999}},
  {{-1, 1}, {0, 0.99999}},
  {{-1, -1}, {0, 0}}
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

// 4 x 4 image..
unsigned char image[] = {
  255, 0, 0, 255,
  255, 0, 0, 255,
  255, 0, 0, 255,
  255, 0, 0, 255,

  0, 255, 0, 255,
  0, 255, 0, 255,
  0, 255, 0, 255,
  0, 255, 0, 255,

  0, 0, 255, 255,
  0, 0, 255, 255,
  0, 0, 255, 255,
  0, 0, 255, 255,

  255, 0, 255, 255,
  255, 0, 255, 255,
  255, 0, 255, 255,
  255, 0, 255, 255
};

// Helper method to parse from Ascii
int getAscii(FILE *fr, char *temp)
  {
    int c = fgetc(fr);
    int i = 0;
    while(c != '\n' && c != ' ' && c != '\t') //Collect all characters before whitespace
      {
      temp[i]=c; //Add it to a buffer
      i+=1;
      c = fgetc(fr);
      if(i>3)
        {
        fprintf(stderr, "%s\n", "Error: file not of type declared in format header");
        exit(1);
        }
      }
    int t = atoi(temp); //Convert to an integer
/*
    if(t > 255)
      {
      fprintf(stderr, "%s\n", "Error: multibyte color is not supported.");
      return(1);
      }
      */
    //printf("%i", t); //Debug Output
    return(t);
  }

//Helper method to parse from Raw data
int getByte(FILE *fr, char *temp)
  {
  int c = fgetc(fr); //Grab next character
  /*
  if(c == ' ' || c == '\n')
  {
  fprintf(stderr, "%s\n", "Error: file not of type declared in ppm header.");
  exit(1);
  }
  */
  int t = c; //Store as an int
  /*
  if(t > 255)
    {
    fprintf(stderr, "%s\n", "Error: multibyte color is not supported.");
    return(1);
    }
    */
  //printf("%i", t);//Debug Output
  return(t);
  }


Pixel* parse(FILE* fr,int* w, int* h)
{
  //Parse Header
  int c = fgetc(fr); // Get 'P'
  int fileType = 0;
  if (c != 'P')
    {
    fprintf(stderr, "%s\n", "Error: file not of type ppm or header not formatted correctly.");
    exit(1);
    }
  c = fgetc(fr); // Get 'X' -- Should be 3 or 6
  if( c == '3')
    {
    fileType = 3;
    //printf("Type 3\n"); //Debug Output
    }
  else if( c == '6')
    {
    fileType = 6;
    //printf("Type 6\n"); //Debug Output
    }
  else
    {
    fprintf(stderr, "%s\n", "Error: file not of type ppm or header not formatted correctly.");
    exit(1);
    }

  c = fgetc(fr); //Get '\n'
  //printf("%c\n", c); //Debug Output

  if (c != '\n' && c!= ' ' && c!= '\t')
    {
    fprintf(stderr, "%s\n", "Error: file not of type ppm or header not formatted correctly.");
    exit(1);
    }
  c = fgetc(fr); //Get Next character -- should be comment, or width/height
  int d; //comment character variable
  while (c == '#') //Process the comment line(s)
    {
    d = fgetc(fr);
    while (d!= '\n') //Write the rest of the comment to the output file
      {
      d = fgetc(fr);
      }
      c = fgetc(fr); //Get next char -- either # or width/height
    }

  //Parse Height/Width
  //printf("%s\n", "Height"); //Debug Output
  //printf("%c\n", c); //Debug Output

  char *height; //Create array to hold characters of the height
  height = malloc(16 * 5);
  int i = 0;
  while(c != ' ') //Get all characters before the first space
    {
    height[i] = c; //Add them to the array
    c = fgetc(fr);
    //printf("%c\n", c); //Debug Output
    i+=1;
    }
  *h = atoi(height); //Convert character array to an Integer value


  //printf("%s\n", "width"); //Debug Output

  char *width; //Create array to hold characters of the width
  width = malloc(16 * 5);
  i = 0;
  while(c != '\n') //Get all characters before the end of the line
    {
    width[i] = c; //Add them to the array
    c = fgetc(fr);
    //printf("%c\n", c); //Debug Output
    i+=1;
    }
  *w = atoi(width); //Convert character array to an Integer value

  //printf("%i\n",h); //Debug Output
  //printf("%i\n",w); //Debug Output


  //Parse Max Color Value
  //printf("%s\n", "maxColor"); //Debug Output

  char *maxColor; //Create array to hold characters of the max color
  maxColor = malloc(16 * 5);
  i = 0;
  c = fgetc(fr);
  while(c != '\n') //Get all characters before the end of the line
    {
    maxColor[i] = c; //Add them to the array
    //printf("%c\n", c); //Debug Output
    i+=1;
    c = fgetc(fr);
    }
  int mC = atoi(maxColor); //Convert character array to an Integer value

  if(mC > 255)
    {
    fprintf(stderr, "%s\n", "Error: multibyte color is not supported.");
    exit(1);
    }
  //printf("%i\n",mC); //Debug Output


  //Parsing of image data
  Pixel* image = malloc(sizeof(Pixel) * *w * *h); //Prepare memory for image data
  unsigned char temp[32]; //Create buffer for data

  if (fileType == 3) //Collect Ascii Image data
    {
    int row, col;
    for (row = 0; row < *h; row += 1)
      {
      for (col = 0; col < *w; col += 1) //Itterate through the image and add it to the structures
        {
        image[*w*row + col].r = getAscii(fr,temp);
        //printf("%d ", image[w*row + col].r); //Debug Output
        image[*w*row + col].g = getAscii(fr,temp);
        //printf("%d ", image[w*row + col].g); //Debug Output
        image[*w*row + col].b = getAscii(fr,temp);
        //printf("%d  ", image[w*row + col].b); //Debug Output
        }
      //printf("\n"); //Debug Output
      }
    }
  else //Collect Raw Image data
    {
    int row, col;
    for (row = 0; row < *h; row += 1)
      {
      for (col = 0; col < *w; col += 1) //Itterate through the image and add it to the structures
        {
        image[*w*row + col].r = getByte(fr,temp);
        //printf("%i ", image[w*row + col].r); //Debug Output
        image[*w*row + col].g = getByte(fr,temp);
        //printf("%i ", image[w*row + col].g); //Debug Output
        image[*w*row + col].b = getByte(fr,temp);
        //printf("%i  ", image[w*row + col].b); //Debug Output
        }
      //printf("\n"); //Debug Output
      }

  }
  return image;
}
int main(int argc, char *argv[])
{
    GLFWwindow* window;
    GLuint vertex_buffer, vertex_shader, fragment_shader, program, index_buffer;
    GLint mvp_location, vpos_location, vcol_location;

    int* iw= malloc(sizeof(int));
    int* ih= malloc(sizeof(int));
    FILE* fr = fopen(argv[1], "r"); // File Read
    //Check if input file exists
    if(fr == NULL)
      {
      fprintf(stderr, "%s\n", "Error: input file type not found.");
      return(1);
      }
    Pixel* image = parse(fr,iw,ih);

    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    glfwDefaultWindowHints();
    glfwWindowHint(GLFW_CONTEXT_CREATION_API, GLFW_EGL_CONTEXT_API);
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(640, 480, "Simple example", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);

    glfwMakeContextCurrent(window);
    // gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval(1);

    // NOTE: OpenGL error checks have been omitted for brevity

    glGenBuffers(1, &vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexes), vertexes, GL_STATIC_DRAW);

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
    // more error checking! glLinkProgramOrDie!

    mvp_location = glGetUniformLocation(program, "MVP");
    assert(mvp_location != -1);

    vpos_location = glGetAttribLocation(program, "vPos");
    assert(vpos_location != -1);

    GLint texcoord_location = glGetAttribLocation(program, "TexCoordIn");
    assert(texcoord_location != -1);

    GLint tex_location = glGetUniformLocation(program, "Texture");
    assert(tex_location != -1);

    glEnableVertexAttribArray(vpos_location);
    glVertexAttribPointer(vpos_location,
			  2,
			  GL_FLOAT,
			  GL_FALSE,
                          sizeof(Vertex),
			  (void*) 0);

    glEnableVertexAttribArray(texcoord_location);
    glVertexAttribPointer(texcoord_location,
			  2,
			  GL_FLOAT,
			  GL_FALSE,
                          sizeof(Vertex),
			  (void*) (sizeof(float) * 2));

    int image_width = *iw;
    int image_height = *ih;

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, image_width, image_height, 0, GL_RGB,
		 GL_UNSIGNED_BYTE, image);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID);
    glUniform1i(tex_location, 0);

    while (!glfwWindowShouldClose(window))
    {
        float ratio;
        int width, height;
        mat4x4 m, p, mvp;

        glfwGetFramebufferSize(window, &width, &height);
        ratio = width / (float) height;

        glViewport(0, 0, width, height);
        glClear(GL_COLOR_BUFFER_BIT);


        mat4x4_identity(m);
        //mat4x4_rotate_Z(m, m, (float) glfwGetTime());
        mat4x4_ortho(p, -ratio, ratio, -1.f, 1.f, 1.f, -1.f);
        mat4x4_mul(mvp, p, m);


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
