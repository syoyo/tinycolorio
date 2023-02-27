#ifndef TINY_COLOR_IO_H_
#define TINY_COLOR_IO_H_

#include <cstdint>
#include <cstdlib>
#include <string>
#include <vector>
#include <array>

namespace tinycolorio {

template <typename T>
class LUT1D {
 public:
  LUT1D() : x_range_({0.0f, 1.0f}), version_(1), components_(0) {}

  ~LUT1D() = default;

  void create(size_t length, size_t components, std::array<T, 2> x_range) {
    components_ = components;
    data_.resize(length * components);
    x_range_ = x_range;
  }

  void set(size_t idx, size_t comp, const T val) {
    if ((idx * components_ + comp) < data_.size()) {
      data_[idx * components_ + comp] = val;
    }
  }

  bool get(size_t idx, size_t comp, T &val) const {
    if ((idx * components_ + comp) < data_.size()) {
      val = data_[idx * components_ + comp];
    }
  }

  size_t length() const { return data_.size() / components_; }
  size_t components() const { return components_; }
  uint32_t version() const { return version_; }

  uint32_t version_{1};
  std::array<T, 2> x_range_;
  size_t components_{1};

  std::vector<T> data_; // sz = components_ * length;
};

template <typename T>
class LUT3D {
 public:
  LUT3D() : x_dim_(0), y_dim_(0), z_dim_(0) {}

  ~LUT3D() {}

  void create(size_t x_dim, size_t y_dim, size_t z_dim) {
    size_t len = x_dim * y_dim * z_dim;
    data_.resize(3 * len);

    x_dim_ = x_dim;
    y_dim_ = y_dim;
    z_dim_ = z_dim;
  }

  void set(size_t x, size_t y, size_t z, const T val[3]) {
    if ((x < x_dim_) && (y < y_dim_) && (z < z_dim_)) {
      size_t idx = (x_dim_ * y_dim_) * z + x_dim_ * y + x;
      data_[3 * idx + 0] = val[0];
      data_[3 * idx + 1] = val[1];
      data_[3 * idx + 2] = val[2];
    }
  }

  void set(size_t x, size_t y, size_t z, const T r_val, const T g_val,
           const T b_val) {
    if ((x < x_dim_) && (y < y_dim_) && (z < z_dim_)) {
      size_t idx = (x_dim_ * y_dim_) * z + x_dim_ * y + x;
      data_[3 * idx + 0] = r_val;
      data_[3 * idx + 1] = g_val;
      data_[3 * idx + 2] = b_val;
    }
  }

  void get(size_t x, size_t y, size_t z, T val[3]) const {
    if ((x < x_dim_) && (y < y_dim_) && (z < z_dim_)) {
      size_t idx = (x_dim_ * y_dim_) * z + x_dim_ * y + x;
      val[0] = data_[3 * idx + 0];
      val[1] = data_[3 * idx + 1];
      val[2] = data_[3 * idx + 2];
    }
  }

  size_t x_dim() { return x_dim_; }

  size_t y_dim() { return y_dim_; }

  size_t z_dim() { return z_dim_; }

  size_t x_dim_;
  size_t y_dim_;
  size_t z_dim_;

  std::vector<T> data_;  // RGB
};

using LUT1Df = LUT1D<float>;
using LUT3Df = LUT3D<float>;

class OCIOConfig {
 public:

 private:

};

///
/// Loads OCIO config(YAML format) from a file.
///
/// @param[in] yaml_filepath ocio config filepath.
/// @param[out] config OCIO Config.
/// @param[out] err Error message(when failed to load a LUT).
///
bool LoadOCIOConfigFromFile(const std::string &yaml_filepath, OCIOConfig *config, std::string *err = nullptr);

// C++17 from_chars like user defined function for converting ascii string to double.
//
// return 0 = success
// return non zero = error
// currently no detailed error code defined.
//
typedef int (*FromCharsFun)(const char *first, const char *end, double &value);

// Default ascii -> double converter.
int DoubleFromChars(const char *first, const char *end, double &value);

///
/// Loads SPI1D LUT data(ASCII) from a file.
///
/// @param[in] filename spi1d LUT filename.
/// @param[out] lut 1D LUT table.
/// @param[out] err Error message(when failed to load a LUT).
/// @param[out] from_chars_fun Custom ascii to double function.
/// @return true upon succes.
///
bool LoadSPI1DFromFile(const std::string &filename, LUT1Df *lut,
                       std::string *err = nullptr,
                       FromCharsFun from_chars_fun = DoubleFromChars);

///
/// Loads SPI1D LUT data(ASCII) from string.
///
/// @param[in] str String representation of spi1d LUT data.
/// @param[out] lut 1D LUT table.
/// @param[out] err Error message(when failed to load a LUT).
/// @param[out] from_chars_fun Custom ascii to double function.
/// @return true upon succes.
///
bool LoadSPI1DFromString(const std::string &str, LUT1Df *lut,
                       std::string *err = nullptr,
                       FromCharsFun from_chars_fun = DoubleFromChars);

///
/// Loads SPI3D LUT data(ASCII)
///
/// @param[in] filename spi3d LUT filename.
/// @param[out] lut 3D LUT table.
/// @param[out] err Error message(when failed to load a LUT).
/// @param[out] from_chars_fun Custom ascii to double function.
/// @return true upon succes.
///
bool LoadSPI3DFromFile(const std::string &filename, LUT3Df *lut,
                       std::string *err = nullptr,
                       FromCharsFun from_chars_fun = DoubleFromChars);

}  // namespace tinycolorio

#endif  // TINY_COLOR_IO_H_

#ifdef TINY_COLOR_IO_IMPLEMENTATION

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <cstring>
#include <sstream>

namespace tinycolorio {

namespace detail {

///
/// Simple stream reader
///
class StreamReader {
 public:
  explicit StreamReader(const uint8_t *binary, const uint64_t length)
      : binary_(binary), length_(length), idx_(0) {
  }

  bool seek_set(const uint64_t offset) const {
    if (offset > length_) {
      return false;
    }

    idx_ = offset;
    return true;
  }

  bool seek_from_current(const int64_t offset) const {
    if ((int64_t(idx_) + offset) < 0) {
      return false;
    }

    if (uint64_t((int64_t(idx_) + offset)) > length_) {
      return false;
    }

    idx_ = uint64_t(int64_t(idx_) + offset);
    return true;
  }

  uint64_t read(const uint64_t n, const uint64_t dst_len, uint8_t *dst) const {
    uint64_t len = n;
    if ((idx_ + len) > length_) {
      len = length_ - uint64_t(idx_);
    }

    if (len > 0) {
      if (dst_len < len) {
        // dst does not have enough space. return 0 for a while.
        return 0;
      }

      size_t nbytes = size_t(len); // may shorten size on 32bit platform

      memcpy(dst, &binary_[idx_], nbytes);
      idx_ += nbytes;
      return nbytes;

    } else {
      return 0;
    }
  }

  bool read1(uint8_t *ret) const {
    if ((idx_ + 1) > length_) {
      return false;
    }

    const uint8_t val = binary_[idx_];

    (*ret) = val;
    idx_ += 1;

    return true;
  }

  bool read_bool(bool *ret) const {
    if ((idx_ + 1) > length_) {
      return false;
    }

    const char val = static_cast<const char>(binary_[idx_]);

    (*ret) = bool(val);
    idx_ += 1;

    return true;
  }

  bool read1(char *ret) const {
    if ((idx_ + 1) > length_) {
      return false;
    }

    const char val = static_cast<const char>(binary_[idx_]);

    (*ret) = val;
    idx_ += 1;

    return true;
  }

  bool read_token(std::string *tok) const {

    if (!tok) {
      return false;
    }

    std::stringstream s;

    // skip white spaces and newlins
    while (eof()) {
      char c;
      if (!read1(&c)) {
        return false;
      }

      if (c == '\0') {
        return false;
      }

      // isspace
      if ((c == ' ')  || (c == '\t') || (c == '\r') || (c == '\n') || (c == '\v')) {
        // continue consuming char
        continue;
      }

      break;
    }

    // read chars until while space or newline

    while (eof()) {
      char c;
      if (!read1(&c)) {
        return false;
      }

      if (c == '\0') {
        return false;
      }

      // TODO: Support backslash escaped char?(e.g. '\\')
      // isspace
      if ((c == ' ')  || (c == '\t') || (c == '\r') || (c == '\n') || (c == '\v')) {
        break;
      }

      s << c;
    }

    (*tok) = s.str();

    return true;
  }

  uint64_t tell() const { return uint64_t(idx_); }
  bool eof() const { return idx_ >= length_; }

  const uint8_t *data() const { return binary_; }

  uint64_t size() const { return length_; }

 private:
  const uint8_t *binary_{nullptr};
  const uint64_t length_{0};
  mutable uint64_t idx_{0};
};

} // namespace detail

bool LoadSPI1DFromString(const std::string &str,
  LUT1Df *lut, std::string *err, FromCharsFun from_chars_fun) {

  (void)lut;
  (void)from_chars_fun;

#define SET_ERROR_AND_RETURN(msg) do { \
  if (err) { \
    std::stringstream ss; \
    ss << msg << "\n"; \
    (*err) += ss.str(); \
  } \
  return false; \
} while(0)

  detail::StreamReader sr(reinterpret_cast<const uint8_t*>(str.data()), str.size());

  // `Version 1`
  std::string tok;
  if (!sr.read_token(&tok)) {
    SET_ERROR_AND_RETURN("Failed to parse Version line.");
  }

  if (tok != "Version") {
    SET_ERROR_AND_RETURN("Failed to parse Version line. expected `Version` but got `" + tok + "`");
  }

  std::string ver;
  if (!sr.read_token(&ver)) {
    SET_ERROR_AND_RETURN("Failed to parse Version line.");
  }

  if (ver != "1") {
    SET_ERROR_AND_RETURN("Version must be 1 but got " + ver);
  }

#undef SET_ERROR_AND_RETURN

  return true;
}

bool LoadSPI1DFromFile(const std::string &filename, LUT1Df *lut,
                       std::string *err, FromCharsFun from_chars_fun) {
  std::ifstream ifs(filename);

  if (!ifs) {
    if (err) {
      (*err) = "Failed to open file : " + filename;
    }
    return false;
  }

  ifs.seekg(0, ifs.end);
  size_t sz = static_cast<size_t>(ifs.tellg());
  ifs.seekg(0, ifs.beg);

  // 32 = heuristic
  if (int64_t(sz) < 32) {
    if (err) {
      (*err) = "Invalid file size: " + filename + "(seems not a .spi1d file)\n";
    }
    return false;
  }

  std::vector<char> buf;
  buf.resize(sz);
  ifs.read(reinterpret_cast<char *>(&buf.at(0)),
           static_cast<std::streamsize>(sz));

  std::string s(buf.data(), buf.size());

  return LoadSPI1DFromString(s, lut, err, from_chars_fun);


}

bool LoadSPI3DFromFile(const std::string &filename, LUT3Df *lut,
                       std::string *err, FromCharsFun from_chars_fun) {
  std::ifstream ifs(filename);

  if (!ifs) {
    if (err) {
      (*err) = "Failed to open file : " + filename;
    }
    return false;
  }

  // header
  std::string line;
  std::getline(ifs, line);

  std::string buf;

  // lower string.
  for (auto c : line) {
    buf.push_back(static_cast<char>(std::tolower(c)));
  }
  // std::transform(line.begin(), line.end(), buf.begin(), std::tolower);

  if (buf.find("spilut") == std::string::npos) {
    if (err) {
      (*err) = "Not a SPILUT format. header = " + line;
    }
    return false;
  }

  // ignore 2nd line(assuming 3 3)
  std::getline(ifs, line);

  // lut size
  std::getline(ifs, line);

  int x_size = 0, y_size = 0, z_size = 0;
  if (3 != sscanf(line.c_str(), "%d %d %d", &x_size, &y_size, &z_size)) {
    if (err) {
      (*err) = "Error while reading lut size";
    }
    return false;
  }

  lut->create(size_t(x_size), size_t(y_size), size_t(z_size));

  int read_count = x_size * y_size * z_size;

  while (ifs.good() && read_count > 0) {
    std::getline(ifs, line);

    int x_idx = 0, y_idx = 0, z_idx = 0;
    float r_value = 0.0f, g_value = 0.0f, b_value = 0.0f;

    if (sscanf(line.c_str(), "%d %d %d %f %f %f", &x_idx, &y_idx, &z_idx,
               &r_value, &g_value, &b_value) == 6) {
      lut->set(size_t(x_idx), size_t(y_idx), size_t(z_idx), r_value, g_value,
               b_value);
    }
  }

  return true;
}

}  // namespace tinycolorio

#endif  // TINY_COLOR_IO_IMPLEMENTATION
