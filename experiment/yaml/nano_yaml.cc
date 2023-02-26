// SPDX-Identifier: MIT
// Copyright 2023-Present, Syoyo Fujita.
//
// Simple yaml parser in portable C++11.
//
// - UTF8 only
// - STL only
// - No RTTI and No C++ exeption

#include <vector>
#include <map>
#include <string>
#include <sstream>
#include <cstring>
#include <stack>

namespace naonyaml {

class Value {
 public:
  typedef std::vector<Value> Array;
  typedef std::map<std::string, Value> Object;

  template<typename T> T get() const;

 private:
  double double_val;

};

template<>
double Value::get() const {
  return double_val;
}

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

class Parser {
 public:
  bool parse_from_file(const std::string &filepath);
  bool parse_from_string(const std::string &str);

 private:

  bool read_token(std::string *tok) const {

    if (!tok) {
      return false;
    }

    std::stringstream s;

    // skip white spaces and newlins
    while (sr.eof()) {
      char c;
      if (!sr.read1(&c)) {
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

    while (sr.eof()) {
      char c;
      if (!sr.read1(&c)) {
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

  detail::StreamReader sr;

  struct Cursor {
    uint32_t row{0};
    uint32_t col{0};
  };
  
  struct ErrorDiagnostic {
    std::string err;
    Cursor cursor;
  };

  void PushError(const std::string &msg) {
      ErrorDiagnostic diag;
      diag.cursor.row = curr_cursor_.row;
      diag.cursor.col = curr_cursor_.col;
      diag.err = msg;
      err_stack_.push(diag);
  }

  Cursor curr_cursor_;
  std::stack<ErrorDiagnostic> err_stack_;
  
  
};


} // namespace nanoyaml

#if defined(NANO_YAML_ENABLE_TEST)

#include <iostream>
#include <cstdio>
#include <cstdlib>

int main(int argc, char **argv) {

  std::string filename = "test.yml";
 
  if (argc < 2) {
    std::cout << "Need input.yaml\n";
    return EXIT_FAILURE;
  }

  

  return EXIT_SUCCESS;
}

#endif
