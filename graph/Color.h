//
// Created by luo-zeqi on 2020/6/29.
//

#ifndef LIBKCHART_COLOR_H
#define LIBKCHART_COLOR_H

#include <cstdint>

typedef uint32_t Color;

#define ColorGetA(color)    (((color) >> 24u) & 0xffu)
#define ColorGetR(color)    (((color) >> 16u) & 0xffu)
#define ColorGetG(color)    (((color) >>  8u) & 0xffu)
#define ColorGetB(color)    ((color) & 0xffu)

inline Color MakeColor(uint8_t r,
                       uint8_t g,
                       uint8_t b,
                       uint8_t a = 255) {
    return ((Color) (
        (((uint32_t) a) << 24u) |
        (((uint32_t) r) << 16u) |
        (((uint32_t) g) << 8u) |
        b)
    );
}

#endif // LIBKCHART_COLOR_H
