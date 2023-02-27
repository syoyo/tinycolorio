#define TINY_COLOR_IO_IMPLEMENTATION
#include "tiny-color-io.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>

int main(int argc, char **argv)
{
  if (argc < 2) {
    std::cerr << "Requires input.ocio(yaml)" << std::endl;
    return EXIT_FAILURE;
  }

  std::string filename = std::string(argv[1]);

  tinycolorio::OCIOConfig config;
  std::string err;
  if (!tinycolorio::LoadOCIOConfigFromFile(filename, &config, &err)) {
    std::cerr << err << std::endl;
  }


  return EXIT_SUCCESS;
}



