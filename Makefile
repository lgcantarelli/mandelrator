run:
  docker run -i -t --rm -v ${PWD}:/usr/src/app gcc:8 bash

mandelrator:
  gcc mandelrator.c -lm -o m && ./m