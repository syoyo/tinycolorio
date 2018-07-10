#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "filter.h"

unsigned char fclamp(float x) {
  int i = (int)(powf(x, 1.0 / 2.2) * 255.0f); // gamma correct
  if (i > 255)
    i = 255;
  if (i < 0)
    i = 0;

  return (unsigned char)i;
}

bool LoadImage(const char *filename, int *width,
               int *height, std::vector<float> *image) {

  int channels = 0;
  unsigned char *data = stbi_load(filename, width, height, &channels, /* desired channels */3);
  if ((channels != 3) || !data) {
    std::cerr << "Failed to load image" << std::endl;
    return false;
  }

  image->resize((*width) * (*height) * 3);

  for (size_t i = 0; i < (*width) * (*height) * 3; i++) {
    (*image)[i] = data[i] / 255.0f; // [0.0, 1.0]
  }

  free(data);

  return true;
}

void SaveImagePNG(const char *filename, const float *rgb, int width,
                  int height) {

  std::vector<unsigned char> ldr(width * height * 3);
  for (size_t i = 0; i < (size_t)(width * height * 3); i++) {
    ldr[i] = fclamp(rgb[i]);
  }

  int len = stbi_write_png(filename, width, height, 3, &ldr.at(0), width * 3);
  if (len < 1) {
    printf("Failed to save image\n");
    exit(-1);
  }
}

static void Apply(
  const example::LutFilter &filter, 
  const std::vector<float> &src,
  const int width,
  const int height,
  std::vector<float> *dst) {

  dst->resize(width * height * 3);

  for (size_t i = 0; i < width * height; i++) {

    float ret[3];

    float r = src[3 * i + 0];
    float g = src[3 * i + 1];
    float b = src[3 * i + 2];

    filter.Apply(ret, r, g, b);

    (*dst)[3 * i + 0] = ret[0];
    (*dst)[3 * i + 1] = ret[1];
    (*dst)[3 * i + 2] = ret[2];
  }
}

int main(int argc, char **argv)
{
  if (argc < 3) {
    std::cerr << "Requires input.png input.lut output.png" << std::endl;
    return EXIT_FAILURE;
  }

  int width, height;
  std::vector<float> src;

  // load image
  if (!LoadImage(argv[1], &width, &height, &src)) {
    return EXIT_FAILURE;
  }

  // load lut
  example::LutFilter lut_filter;
  if (!lut_filter.Load(argv[2])) {
    return EXIT_FAILURE;
  }

  std::vector<float> dst;

  // Apply 3D LUT
  Apply(lut_filter, src, width, height, &dst);

  // Save color correct image.
  SaveImagePNG(argv[3], dst.data(), width, height);

  return EXIT_SUCCESS;
}
