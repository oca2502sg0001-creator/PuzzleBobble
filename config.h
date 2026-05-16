#pragma once
#include <cmath> //  std::round “™‚Ì‚½‚ß‚É•K—v

enum class BubbleColor : int { Red = 0, Blue, Green, Yellow, None = -1 };
struct Vector2 { float x, y; };

namespace GameConfig {
    static constexpr int ScreenWidth = 620;
    static constexpr int ScreenHeight = 480;

    static constexpr float Radius = 16.0f;
    static constexpr float Diameter = Radius * 2.0f;
    static constexpr float RowHeight = Diameter * 0.8660254f;

    static constexpr int GridRows = 14;
    static constexpr int GridCols = 8;

    static constexpr float FieldLeft = 192.0f;
    static constexpr float FieldRight = FieldLeft + Diameter * GridCols + Radius;
    static constexpr float FieldTop = 40.0f;
    static constexpr float ShootSpeed = 12.0f;

    template<typename T>
    inline constexpr T Clamp(T val, T min, T max) {
        return (val < min) ? min : (val > max) ? max : val;
    }
}