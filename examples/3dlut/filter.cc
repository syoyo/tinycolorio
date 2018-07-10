#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "filter.h"

#define TINY_COLOR_IO_IMPLEMENTATION
#include "tiny-color-io.h"

using namespace example;

bool
LutFilter::Load(
  const char* filename)
{
  tinycolorio::LUT3Df lut;
  std::string err;
  bool ret = LoadSPI3DFromFile(filename, &lut, &err);
  if (!err.empty()) {
    std::cerr << err << std::endl;
  }
  if (!ret) {
    std::cerr << "Failed to load SPI 3D lut." << std::endl;
    return false;
  }
    
  dim[0] = lut.x_dim();
  dim[1] = lut.y_dim();
  dim[2] = lut.z_dim();

  int num = dim[0] * dim[1] * dim[2];
  data.resize(num*3);

  for (size_t z = 0; z < lut.z_dim(); z++) {
    for (size_t y = 0; y < lut.y_dim(); y++) {
      for (size_t x = 0; x < lut.x_dim(); x++) {

        float col[3];

        lut.get(x, y, z, col);
  
        data[3*(z*dim[1]*dim[0]+y*dim[0]+x)+0] = col[0];
        data[3*(z*dim[1]*dim[0]+y*dim[0]+x)+1] = col[1];
        data[3*(z*dim[1]*dim[0]+y*dim[0]+x)+2] = col[2];
      }
    }
  }

  return true;
}
