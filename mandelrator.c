#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

// Implementar, utilizando MPI e OpenMP, um programa gerador de fractais Mandelbrot.
// A ideia consiste em um distribuir, entre os nós MPI, regiões do fractal a ser calculado e, internamente a cada nó,
// utilizar concorrência  em OpenMP para computação da imagem.

// 1 - Estabelecer o número de nós: Acho que isso pode ser pre-determinado, e até um input do programa
// 2 - Dividir o número de nós desejados pela altura da imagem e criar os nós MPI
// 3 - Dentro de cada nó usar OpenMP para paralelizar a computação da imagem
// 4 - Como escrever de forma paralela na imagem? Talvez vai ser necessário voltar como era antes: popula o array de pixels e escreve tudo no final

static const int ImageWidth    = 800;
static const int ImageHeight   = 800;
static const int MaxColorValue = 1024;
static const int MaxIteration  = 500;

static const double CxMin = -2.0;
static const double CxMax = 2.0;
static const double CyMin = -2.0;
static const double CyMax = 2.0;

void main() {
  typedef unsigned char pixel_t[3]; // colors [R, G ,B]
  pixel_t *pixels = malloc(sizeof(pixel_t) * ImageHeight * ImageWidth);

  FILE* output_image;

  output_image = fopen("mandelbrot.ppm", "wb");
  fprintf(output_image, "P6\n%d %d\n%d\n", ImageWidth, ImageHeight, MaxColorValue);

  double pixel_width  = (CxMax - CxMin) / ImageWidth;  /* scaled x coordinate of pixel (must be scaled to lie somewhere in the Mandelbrot X scale (-2.5, 1.5) */
  double pixel_height = (CyMax - CyMin) / ImageHeight; /* scaled y coordinate of pixel (must be scaled to lie somewhere in the Mandelbrot Y scale (-2.0, 2.0) */

  for (int y = 0; y < ImageHeight; y++) {
    // if (fabs(Cy) < pixel_height / 2) {
    //   Cy = 0.0;
    // }

    for (int x = 0; x < ImageWidth; x++) {
      double Zx  = 0.0;
      double Zy  = 0.0;
      double Zx2 = Zx * Zx;
      double Zy2 = Zy * Zy;

      double Cy = CyMin + y * pixel_height;
      double Cx = CxMin + x * pixel_width;

      int i = 0;
      const double bailout = 2; // bail-out value
      const double circle_radius = bailout * bailout; // circle radius

      while (i < MaxIteration && ((Zx2 + Zy2) < circle_radius)) {
        Zy  = 2 * Zx * Zy + Cy;
        Zx  = Zx2 - Zy2 + Cx;
        Zx2 = Zx * Zx;
        Zy2 = Zy * Zy;
        i++;
      }

      int pixel_position = y * ImageWidth + x;

      if (i == MaxIteration) {
        // Interior of Mandelbrot set = black
        pixels[pixel_position][0] = 0;
        pixels[pixel_position][1] = 0;
        pixels[pixel_position][2] = 0;
      } else {
        pixels[pixel_position][0] = 0;
        pixels[pixel_position][1] = ((double)(i - log2(log2(sqrt(Zx2 + Zy2)))) / MaxIteration) * MaxColorValue;
        pixels[pixel_position][2] = 0;
      }

      fwrite(pixels[pixel_position], 1, sizeof(pixel_t), output_image);
    }
  }

  fclose(output_image);
  free(pixels);
}