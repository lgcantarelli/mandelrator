#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

static const int ImageWidth    = 800;
static const int ImageHeight   = 800;
static const int MaxColorValue = 1024;
static const int MaxIteration  = 1000;

static const double CxMin = -2.0;
static const double CxMax = 0.47;
static const double CyMin = -1.12;
static const double CyMax = 1.12;

void main() {
  double pixel_width  = (CxMax - CxMin) / ImageWidth;  /* scaled x coordinate of pixel (must be scaled to lie somewhere in the Mandelbrot X scale (-2.5, 1.5) */
  double pixel_height = (CyMax - CyMin) / ImageHeight; /* scaled y coordinate of pixel (must be scaled to lie somewhere in the Mandelbrot Y scale (-2.0, 2.0) */

  typedef unsigned char pixel_t[3]; // colors [R, G ,B]
  pixel_t *pixels = malloc(sizeof(pixel_t) * ImageHeight * ImageWidth);

  FILE* fp;

  for (int y = 0; y < ImageHeight; y++) {
    double Cy = CyMin + y * pixel_height;

    if (fabs(Cy) < pixel_height / 2) {
      Cy = 0.0;
    }

    for (int x = 0; x < ImageWidth; x++) {
      double Zx  = 0.0;
      double Zy  = 0.0;
      double Zx2 = Zx * Zx;
      double Zy2 = Zy * Zy;
      double Cx  = CxMin + x * pixel_width;

      int i;
      const double bailout = 2; // bail-out value
      const double circle_radius = bailout * bailout; // circle radius

      for (i = 0; i < MaxIteration && ((Zx2 + Zy2) < circle_radius); i++) {
        Zy  = 2 * Zx * Zy + Cy;
        Zx  = Zx2 - Zy2 + Cx;
        Zx2 = Zx * Zx;
        Zy2 = Zy * Zy;
      }

      int pixel_position = y * ImageWidth + x;

      if (i == MaxIteration) {
        // Interior of Mandelbrot set = black
        pixels[pixel_position][0] = 0;
        pixels[pixel_position][1] = 0;
        pixels[pixel_position][2] = 0;
      } else {
        pixels[pixel_position][0] = ((double)(i - log2(log2(sqrt(Zx2 + Zy2)))) / MaxIteration) * MaxColorValue;
        pixels[pixel_position][1] = 0;
        pixels[pixel_position][2] = 0;
      }
    }
  }
  
  fp = fopen("MandelbrotSet.ppm", "wb");
  fprintf(fp, "P6\n%d %d\n%d\n", ImageWidth, ImageHeight, MaxColorValue);

  for(int y = 0; y < ImageHeight; y++)
    for(int x = 0; x < ImageWidth; x++)
        fwrite(pixels[y * ImageWidth + x], 1, sizeof(pixel_t), fp);

  fclose(fp);
  free(pixels);
}