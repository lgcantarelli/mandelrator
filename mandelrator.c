#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <omp.h>
#include "mpi.h"

// Implementar, utilizando MPI e OpenMP, um programa gerador de fractais Mandelbrot.
// A ideia consiste em um distribuir, entre os nós MPI, regiões do fractal a ser calculado e, internamente a cada nó,
// utilizar concorrência  em OpenMP para computação da imagem.

// 1 - Estabelecer o número de nós: Acho que isso pode ser pre-determinado, e até um input do programa
// 2 - Dividir o número de nós desejados pela altura da imagem e criar os nós MPI
// 3 - Dentro de cada nó usar OpenMP para paralelizar a computação da imagem
// 4 - Como escrever de forma paralela na imagem? Talvez vai ser necessário voltar como era antes: popula o array de pixels e escreve tudo no final

const int MASTER = 0, TAG = 1;

void main() {
  int rank, ranks_count;
  typedef unsigned char pixel_type[3]; // colors [R, G, B]

  MPI_Init(NULL, NULL);

  MPI_Comm_size(MPI_COMM_WORLD, &ranks_count);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  int ImageWidth    = 800;
  int ImageHeight   = 800;
  int MaxColorValue = 1024;
  int MaxIteration  = 50;

  double CxMin = -2.0;
  double CxMax = 2.0;
  double CyMin = -2.0;
  double CyMax = 2.0;

  double pixel_width  = (CxMax - CxMin) / ImageWidth;  /* scaled x coordinate of pixel (must be scaled to lie somewhere in the Mandelbrot X scale (-2.5, 1.5) */
  double pixel_height = (CyMax - CyMin) / ImageHeight; /* scaled y coordinate of pixel (must be scaled to lie somewhere in the Mandelbrot Y scale (-2.0, 2.0) */

  if (rank == MASTER) {
    printf("[MASTER] Initializing MASTER\n");

    pixel_type *pixels = malloc(sizeof(pixel_type) * ImageHeight * ImageWidth);

    int range_size = (int)(ImageHeight / (ranks_count - 1));
    int range_pixels_size = sizeof(pixel_type) * range_size * ImageWidth;

    printf("[MASTER] Ranks count: %d\n", ranks_count);
    printf("[MASTER] Range size: %d\n", range_size);
    printf("[MASTER] Total pixels size: %d\n", sizeof(pixel_type) * ImageHeight * ImageWidth);

    for (int i = 1; i < ranks_count; i++) {
      int received_pixels_size = sizeof(pixel_type) * range_size * ImageWidth;
      pixel_type *received_pixels = malloc(received_pixels_size);

      printf("[MASTER] Add Recv from rank: %d\n", i);

      int range_start = i == 1 ? 0 : ((i - 1) * range_size);
      MPI_Recv(received_pixels, range_pixels_size, MPI_UNSIGNED_CHAR, i, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

      printf("[MASTER] Received from rank: %d, Range start: %d\n", i, range_start);

      for (int y = 0; y < range_size; y++) {
        for (int x = 0; x < ImageWidth; x++) {
          int pixel_position = range_start * ImageWidth + x;
          int received_pixel_position = y * ImageWidth + x;

          pixels[pixel_position][0] = received_pixels[received_pixel_position][0];
          pixels[pixel_position][1] = received_pixels[received_pixel_position][1];
          pixels[pixel_position][2] = received_pixels[received_pixel_position][2];
        }
        
        range_start++;
      }

      free(received_pixels);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    FILE* output_image;

    output_image = fopen("mandelbrot.ppm", "wb");
    fprintf(output_image, "P6\n%d %d\n%d\n", ImageWidth, ImageHeight, MaxColorValue);

    for(int y = 0; y < ImageHeight; y++) {
      for(int x = 0; x < ImageWidth; x++) {
        fwrite(pixels[y * ImageWidth + x], 1, sizeof(pixel_type), output_image);
      }
    }

    fclose(output_image);
    free(pixels);
  } else {
    printf("[WORKER] Initializing #%d\n", rank);

    int range_size = (int)(ImageHeight / (ranks_count - 1));
    int range_start = rank == 1 ? 0 : ((rank - 1) * range_size);

    int pixels_size = sizeof(pixel_type) * range_size * ImageWidth;
    pixel_type *pixels = malloc(pixels_size);

    for (int y = 0; y < range_size; y++)
    {
      #pragma omp parallel for shared(pixels)
      for (int x = 0; x < ImageWidth; x++)
      {
        double Zx  = 0.0;
        double Zy  = 0.0;
        double Zx2 = Zx * Zx;
        double Zy2 = Zy * Zy;

        double Cy = CyMin + range_start * pixel_height;
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

        #pragma omp critical
        {
          pixels[pixel_position][0] = i == MaxIteration ? 0 : (((double)(i - log2(log2(sqrt(Zx2 + Zy2)))) / MaxIteration) * MaxColorValue);
          pixels[pixel_position][1] = 0;
          pixels[pixel_position][2] = 0;
        }
      }

      range_start++;
    }

    MPI_Send(pixels, pixels_size, MPI_UNSIGNED_CHAR, MASTER, TAG, MPI_COMM_WORLD);

    free(pixels);

    MPI_Barrier(MPI_COMM_WORLD);
  }

  MPI_Finalize();
}