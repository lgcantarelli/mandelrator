# mandelrator
Gerador do fractal de mandelbrot utilizando MPI e OpenMP.

## Para executar o programa
`mpicc -fopenmp mandelrator.c -lm -lpthread -o m.out && mpirun m.out`

