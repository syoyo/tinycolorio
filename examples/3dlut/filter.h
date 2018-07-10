#ifndef EXAMPLE_FILTER_H_
#define EXAMPLE_FILTER_H_

#include <cmath>
#include <algorithm>

#include "tiny-color-io.h"

namespace example {

/// 3D LUT filer.
class LutFilter
{
    static inline float fequal(float x, float y, float eps) {
        if (std::fabs(x-y) < eps) {
            return true;
        }
        return false;
    }

    static inline float fclamp01(float x) {
        if (x < 0.0f) x = 0.0f;
        if (x > 1.0f) x = 1.0f;
        return x;
    }

    static inline void quantize(int* i, float* fracval, float x, int sz) {

        // clamp to 0-1
        x = std::max(x, 0.0f);
        x = std::min(x, 1.0f);

        float px = (sz - 1) * x;
        float s = std::floor(px); // int part
        float u = px - s;    // frac part

        int   x0 = (int)px;
        if (x0 < 0) x0 = 0;
        if (x0 > (sz - 1)) x0 = sz - 1;

        (*fracval) = u;
        (*i) = x0;

    }

    static inline float lerp(float t, float a, float b) {
        return a + (b - a) * t;
    }

  public:
    LutFilter() {};
    ~LutFilter() {
    }

    /// Load 3D LUT.
    bool Load(const char* filename);

    /// If LUT was successfully initialized in Load(), return true.
    bool IsValid() {
        return !data.empty();
    }

    /// Apply 3D LUT to input color.
    inline void Apply(float col[3], float r, float g, float b) const {

        // trilinear interpolation

        int ix0, iy0, iz0;
        float fx, fy, fz;
        quantize(&ix0, &fx, r, dim[0]);
        quantize(&iy0, &fy, g, dim[1]);
        quantize(&iz0, &fz, b, dim[2]);
        //printf("quant: %f -> %d, %f\n", r, ix0, fx);

        int ix1 = ((ix0 + 1) >= dim[0]) ? (dim[0] - 1) : (ix0 + 1);
        int iy1 = ((iy0 + 1) >= dim[1]) ? (dim[1] - 1) : (iy0 + 1);
        int iz1 = ((iz0 + 1) >= dim[2]) ? (dim[2] - 1) : (iz0 + 1);

        for (int i = 0; i < 3; i++) {

            int i000 = iz0 * dim[1] * dim[0] + iy0 * dim[0] + ix0;
            int i001 = iz0 * dim[1] * dim[0] + iy0 * dim[0] + ix1;
            int i010 = iz0 * dim[1] * dim[0] + iy1 * dim[0] + ix0;
            int i011 = iz0 * dim[1] * dim[0] + iy1 * dim[0] + ix1;
            int i100 = iz1 * dim[1] * dim[0] + iy0 * dim[0] + ix0;
            int i101 = iz1 * dim[1] * dim[0] + iy0 * dim[0] + ix1;
            int i110 = iz1 * dim[1] * dim[0] + iy1 * dim[0] + ix0;
            int i111 = iz1 * dim[1] * dim[0] + iy1 * dim[0] + ix1;

            float d00 = lerp(fx, data[3*i000+i], data[3*i001+i]);
            float d10 = lerp(fx, data[3*i010+i], data[3*i011+i]);
            float d01 = lerp(fx, data[3*i100+i], data[3*i101+i]);
            float d11 = lerp(fx, data[3*i110+i], data[3*i111+i]);
            float d0  = lerp(fy, d00, d10);
            float d1  = lerp(fy, d01, d11);
            float d   = lerp(fz, d0, d1);

            col[i] = d;
        } 
    }

    /// For numeric debugging
    static inline void Heatmap(float col[3], float r, float g, float b) {
        // 2^(-8.5) --   2^1   --  2^5
        // blue         green      red
        float blue[3]  = {0.0f, 0.0f, 1.0f};
        float green[3] = {0.0f, 1.0f, 0.0f};
        float red[3]   = {1.0f, 0.0f, 0.0f};

        float gray18 = 0.180f;

        float logCol[3];
        logCol[0] = logf(r) / logf(2.0f);
        logCol[1] = logf(g) / logf(2.0f);
        logCol[2] = logf(b) / logf(2.0f);

        for (int i = 0; i < 3; i++) {
            // near 18% gray -> gray
            if (fequal(r, gray18, 0.05f)) {
                col[i] = 0.5f;
            } else {
                float f = (logCol[i] + 8.5f) / (5.0f + 8.5f);
                if (f < 0.0f) f = 0.0f;
                if (f > 1.0f) f = 1.0f;
                if (f < 0.5f) {
                    col[i] = blue[i] + (green[i] - blue[i]) * 2.0 * f;
                } else {
                    col[i] = green[i] + (red[i] - green[i]) * 2.0 * (f - 0.5f);
                }
            }
        }
    }    

    std::vector<float> data;
    int   dim[3];
};

}

#endif  // __EXAMPLE_FILTER_H__
