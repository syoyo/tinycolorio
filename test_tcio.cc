#define TINY_COLOR_IO_IMPLEMENTATION
#include "tiny-color-io.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

int main(int argc, char **argv)
{
  if (argc < 2) {
    std::cerr << "Requires input.spi3d" << std::endl;
    return EXIT_FAILURE;
  } 

  std::string filename = std::string(argv[1]);

  tinycolorio::LUT3Df lut;
  std::string err;
  if (!tinycolorio::LoadSPI3DFromFile(filename, &lut, &err)) {
    std::cerr << err << std::endl;
  }

  std::cout << "x size " << lut.x_dim() << std::endl;
  std::cout << "y size " << lut.y_dim() << std::endl;
  std::cout << "z size " << lut.z_dim() << std::endl;

  for (size_t z = 0; z < lut.z_dim(); z++) {
    for (size_t y = 0; y < lut.y_dim(); y++) {
      for (size_t x = 0; x < lut.x_dim(); x++) {
        float rgb[3];
        lut.get(x, y, z, rgb);
        std::cout << "x[" << x << "] y[" << y << "] z[" << z << "] = " << rgb[0] << ", " << rgb[1] << ", " << rgb[2] << std::endl;
      }
    }
  }

  return EXIT_SUCCESS;
}



