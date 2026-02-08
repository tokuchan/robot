static_assert(__cplusplus > 2020'00);
#pragma once

#include <array>
#include <cmath>
#include <algorithm>

namespace robot::src::detail::math::inline exports
{
    using Float = float;

    struct Vec2
    {
        Float x = 0;
        Float y = 0;

        constexpr Vec2() = default;
        constexpr Vec2(Float theX, Float theY) : x(theX), y(theY) {}
    };

    inline constexpr Vec2 operator+(Vec2 a, Vec2 b) { return {a.x + b.x, a.y + b.y}; }
    inline constexpr Vec2 operator-(Vec2 a, Vec2 b) { return {a.x - b.x, a.y - b.y}; }
    inline constexpr Vec2 operator*(Vec2 v, Float s) { return {v.x * s, v.y * s}; }
    inline constexpr Vec2 operator*(Float s, Vec2 v) { return v * s; }
    inline constexpr Vec2 operator/(Vec2 v, Float s) { return {v.x / s, v.y / s}; }

    inline constexpr Vec2 &operator+=(Vec2 &a, Vec2 b)
    {
        a = a + b;
        return a;
    }
    inline constexpr Vec2 &operator-=(Vec2 &a, Vec2 b)
    {
        a = a - b;
        return a;
    }
    inline constexpr Vec2 &operator*=(Vec2 &v, Float s)
    {
        v = v * s;
        return v;
    }

    inline constexpr Float dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }

    // 2D “cross” helpers:
    // cross( a, b ) returns scalar (signed area / z-component)
    inline constexpr Float cross(Vec2 a, Vec2 b) { return a.x * b.y - a.y * b.x; }
    // cross( s, v ) and cross( v, s ) return perpendicular vectors
    inline constexpr Vec2 cross(Float s, Vec2 v) { return {-s * v.y, s * v.x}; }
    inline constexpr Vec2 cross(Vec2 v, Float s) { return {s * v.y, -s * v.x}; }

    inline constexpr Vec2 perp(Vec2 v) { return {-v.y, v.x}; }

    inline Float lengthSq(Vec2 v) { return dot(v, v); }
    inline Float length(Vec2 v) { return std::sqrt(lengthSq(v)); }

    inline Vec2 normalized(Vec2 v, Float eps = 1e-8f)
    {
        auto const len = length(v);
        if (len <= eps)
            return {0, 0};
        return v / len;
    }

    inline Vec2 clampVec2(Vec2 v, Vec2 minV, Vec2 maxV)
    {
        return {
            std::max(minV.x, std::min(v.x, maxV.x)),
            std::max(minV.y, std::min(v.y, maxV.y)),
        };
    }

    struct Mat3;
    inline Mat3 operator*(Mat3 const &a, Mat3 const &b);
    inline Vec2 operator*(Mat3 const &m, Vec2 v);
    inline Vec2 operator*(Vec2 v, Mat3 const &m);

    // ------------------------------------------------------------
    // Mat3 (homogeneous 2D affine)
    // Column-major storage: m[ col*3 + row ]
    // ------------------------------------------------------------
    struct Mat3
    {
        std::array<Float, 9>
            m{{1, 0, 0,
               0, 1, 0,
               0, 0, 1}};

        static Mat3 identity() { return Mat3{}; }

        Float &operator()(int row, int col) { return m[col * 3 + row]; }
        Float operator()(int row, int col) const { return m[col * 3 + row]; }

        static Mat3 translation(Vec2 t)
        {
            Mat3 r = identity();
            r(0, 2) = t.x;
            r(1, 2) = t.y;
            return r;
        }

        auto &translate(Vec2 t)
        {
            *this = Mat3::translation(t) * *this;
            return *this;
        }

        static Mat3 scaled(Vec2 s)
        {
            Mat3 r = identity();
            r(0, 0) = s.x;
            r(1, 1) = s.y;
            return r;
        }

        auto &scale(Vec2 s)
        {
            *this = Mat3::scaled(s) * *this;
            return *this;
        }

        static Mat3 rotation(Float radians)
        {
            Mat3 r = identity();
            auto const c = std::cos(radians);
            auto const s = std::sin(radians);

            // [ c -s 0
            //   s  c 0
            //   0  0 1 ]
            r(0, 0) = c;
            r(0, 1) = -s;
            r(1, 0) = s;
            r(1, 1) = c;
            return r;
        }

        auto &rotate(Float radians)
        {
            *this = Mat3::rotation(radians) * *this;
            return *this;
        }
    };

    inline Mat3 operator*(Mat3 const &a, Mat3 const &b)
    {
        Mat3 out = Mat3::identity();
        for (int col = 0; col < 3; ++col)
        {
            for (int row = 0; row < 3; ++row)
            {
                Float sum = 0;
                for (int k = 0; k < 3; ++k)
                {
                    sum += a(row, k) * b(k, col);
                }
                out(row, col) = sum;
            }
        }
        return out;
    }

    inline Vec2 operator*(Mat3 const &m, Vec2 v)
    {
        // [x',y',1]^T = M * [x,y,1]^T
        return {
            m(0, 0) * v.x + m(0, 1) * v.y + m(0, 2),
            m(1, 0) * v.x + m(1, 1) * v.y + m(1, 2),
        };
    }

    inline Vec2 operator*(Vec2 v, Mat3 const &m)
    {
        // [x',y',1] = [x,y,1] * M^T
        return {
            m(0, 0) * v.x + m(1, 0) * v.y + m(2, 0),
            m(0, 1) * v.x + m(1, 1) * v.y + m(2, 1),
        };
    }

    // ------------------------------------------------------------
    // Transform2D (TRS stored, matrix built on demand)
    // Convention: column vectors; compose as T * R * S
    // ------------------------------------------------------------
    struct Transform2D
    {
        Vec2 translation{0, 0};
        Float rotationRadians = 0;
        Vec2 scale{1, 1};

        Mat3 toMatrix() const
        {
            return Mat3()
                .translate(translation)
                .rotate(rotationRadians)
                .scale(scale);
        }
    };

    // ------------------------------------------------------------
    // Canvas export
    // Canvas setTransform( a, b, c, d, e, f ) corresponds to:
    // [ a c e
    //   b d f
    //   0 0 1 ]
    // ------------------------------------------------------------
    struct CanvasXform
    {
        Float a, b, c, d, e, f;
    };

    inline CanvasXform toCanvas(Mat3 const &m)
    {
        return {
            m(0, 0), // a
            m(1, 0), // b
            m(0, 1), // c
            m(1, 1), // d
            m(0, 2), // e
            m(1, 2), // f
        };
    }

    struct AxisAlignedBoundingBox
    {
        Vec2 min;
        Vec2 max;

        bool intersects(const AxisAlignedBoundingBox &other) const
        {
            return !(max.x < other.min.x || min.x > other.max.x ||
                     max.y < other.min.y || min.y > other.max.y);
        }
    };
}

namespace robot::src::inline exports::inline math
{
    using namespace detail::math::exports;
}