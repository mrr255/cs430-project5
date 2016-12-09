#ifndef PPMR
#define PPMR

#ifndef STD_LIB
#define STD_LIB
#include <stdlib.h>
#include <stdio.h>

#include <string.h>
#include <ctype.h>
#endif


typedef struct Pixel {
  unsigned char r, g, b;
} Pixel;

/* Function Prototypes */
static inline  void   readP3(FILE *in, Pixel *buffer, int *width,
                               int *height, int *maxColor);
static inline  void   readP6(FILE *in, Pixel *buffer, int *width,
                               int *height, int *maxColor);
static inline  int    parseH(FILE *fr, int *width, int *height,
                               int *maxColor, int *version);

//parse the header
static inline int parseH(FILE *fr, int *width, int *height,
                           int *maxColor, int *version)
{
  char c; // Character value

  fscanf(fr, "P%c\n", &c);
  if (c != '3' && c != '6') //valid type?
  {
      return 1;
  }
  *version = atoi(&c); //store version

  c = getc(fr);   // handle comments
  while (c == '#')
  {
    do
    {
      c = getc(fr);
    }
    while (c != '\n');
    c = getc(fr);
  }

  //should be a number next
  if (!isdigit(c))
  {
    return 1;
  }
  ungetc(c, fr);


  //grab values from header
  fscanf(fr, "%d%d%d\n", width, height, maxColor);

  return 0;
}


static inline void readP3(FILE *in, Pixel *buffer, int *width, int *height, int *maxColor)
  {
  int c;
  int r, g, b;
  int i = 0;
  int arryMax = *width * *height;


  while (i < arryMax) {
      if ((c = fgetc(in)) != EOF) //if still values left
      {
        ungetc(c, in);

        //get pixel
        if (fscanf(in, "%d%d%d", &r, &g, &b) == 3) {
          buffer[i].r = (unsigned char) r;
          buffer[i].g = (unsigned char) g;
          buffer[i].b = (unsigned char) b;
        }
      i++;
    }
  }
}

static inline void readP6(FILE *in, Pixel *buffer, int *width,
                            int *height, int *maxColor) {
  unsigned char *charBuffer = malloc(sizeof(unsigned char));
  int i = 0;
  int color;
  int arryMax = (*width) * (*height);

  while (fread(charBuffer, 1, 1, in) && i < arryMax) //add pixels to rgb
  {
    color = *charBuffer;
    buffer[i].r = (unsigned char) color;
    fread(charBuffer, 1, 1, in);
    color = *charBuffer;
    buffer[i].g = (unsigned char) color;
    fread(charBuffer, 1, 1, in);
    color = *charBuffer;
    buffer[i].b = (unsigned char) color;
    i++;
  }
}

#endif
