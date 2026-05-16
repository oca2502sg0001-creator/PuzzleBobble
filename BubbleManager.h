#pragma once
#include "DxLib.h"
#include "Config.h"
#include <vector>
#include <utility> 
#include <algorithm>

struct FallingBubble {

    Vector2 pos;
    Vector2 vel;

    BubbleColor color;

    float angle;
    float rotSpeed;
};

class BubbleManager {

    BubbleColor grid[GameConfig::GridRows][GameConfig::GridCols];
    int bubbleHandles[4];

    std::vector<FallingBubble> fallingBubbles;

    // ==================================================
    // 天井下降オフセット
    // ==================================================
    float ceilingOffsetY = 0.0f;

public:

    BubbleManager() {

        bubbleHandles[0] = LoadGraph("Data/Red.png");
        bubbleHandles[1] = LoadGraph("Data/Blue.png");
        bubbleHandles[2] = LoadGraph("Data/Green.png");
        bubbleHandles[3] = LoadGraph("Data/Yellow.png");

        for (int r = 0; r < GameConfig::GridRows; ++r) {
            for (int c = 0; c < GameConfig::GridCols; ++c) {

                grid[r][c] =
                    (r < 6)
                    ? static_cast<BubbleColor>((r + c / 2) % 4)
                    : BubbleColor::None;
            }
        }
    }

    // ==================================================
    // 座標取得
    // ceilingOffsetY を追加
    // ==================================================
    inline Vector2 GetPos(int r, int c) const {

        float offsetX = (r & 1) ? GameConfig::Radius : 0.0f;

        return {
            GameConfig::FieldLeft + GameConfig::Radius + c * GameConfig::Diameter + offsetX,

            // ↓ここ重要
            GameConfig::FieldTop +
            GameConfig::Radius +
            r * GameConfig::RowHeight +
            ceilingOffsetY
        };
    }

    bool CheckCollision(Vector2 bPos, int& outR, int& outC) const {

        // ==================================================
        // 天井判定に offset を入れる
        // ==================================================
        if (bPos.y <=
            GameConfig::FieldTop +
            GameConfig::Radius +
            ceilingOffsetY) {

            return true;
        }

        for (int r = 0; r < GameConfig::GridRows; ++r) {
            for (int c = 0; c < GameConfig::GridCols; ++c) {

                if (grid[r][c] == BubbleColor::None) continue;

                Vector2 p = GetPos(r, c);

                float dx = bPos.x - p.x;
                float dy = bPos.y - p.y;

                if (dx * dx + dy * dy <
                    GameConfig::Diameter * GameConfig::Diameter * 0.9f) {

                    outR = r;
                    outC = c;

                    return true;
                }
            }
        }

        return false;
    }

    std::pair<int, int> SetBubble(Vector2 p, BubbleColor color) {

        // ==================================================
        // ceilingOffsetY 
        // ==================================================
        int r = static_cast<int>(
            floor(
                (
                    p.y -
                    GameConfig::FieldTop -
                    ceilingOffsetY
                    )
                / GameConfig::RowHeight
            )
            );

        r = GameConfig::Clamp(r, 0, GameConfig::GridRows - 1);

        float offsetX = (r & 1) ? GameConfig::Radius : 0.0f;

        int c = static_cast<int>(
            floor(
                (
                    p.x -
                    GameConfig::FieldLeft -
                    offsetX
                    )
                / GameConfig::Diameter
            )
            );

        c = GameConfig::Clamp(c, 0, GameConfig::GridCols - 1);

        int finalR = r;
        int finalC = c;

        if (grid[r][c] == BubbleColor::None) {

            grid[r][c] = color;
        }
        else if (r + 1 < GameConfig::GridRows &&
            grid[r + 1][c] == BubbleColor::None) {

            grid[r + 1][c] = color;

            finalR = r + 1;
        }

        return { finalR, finalC };
    }

    void ProcessChain(int r, int c) {

        if (r < 0 || r >= GameConfig::GridRows ||
            c < 0 || c >= GameConfig::GridCols) {
            return;
        }

        BubbleColor targetColor = grid[r][c];

        if (targetColor == BubbleColor::None) {
            return;
        }

        std::vector<std::pair<int, int>> connected;

        bool visited[GameConfig::GridRows][GameConfig::GridCols] = { false };

        FindSameColor(r, c, targetColor, visited, connected);

        if (connected.size() >= 3) {

            for (auto& pos : connected) {

                grid[pos.first][pos.second] = BubbleColor::None;
            }

            DropFloatingBubbles();
        }
    }

    // ==================================================
    // 変更：行追加ではなく天井を下げる
    // ==================================================
    void DropCeiling() {

        ceilingOffsetY += GameConfig::RowHeight;

        // 少しずつ下げたいなら：
        // ceilingOffsetY += 8.0f;
    }

    bool IsGameOver() const {

        constexpr float dangerLine = 380.0f;

        for (int r = 0; r < GameConfig::GridRows; ++r) {
            for (int c = 0; c < GameConfig::GridCols; ++c) {

                if (grid[r][c] == BubbleColor::None) continue;

                Vector2 p = GetPos(r, c);

                if (p.y >= dangerLine) {

                    return true;
                }
            }
        }

        return false;
    }

    void UpdateFalling() {

        for (auto& b : fallingBubbles) {

            // 重力
            b.vel.y += 0.15f;

            b.pos.x += b.vel.x;
            b.pos.y += b.vel.y;

            b.angle += b.rotSpeed;
        }

        // 画面外削除
        fallingBubbles.erase(
            std::remove_if(
                fallingBubbles.begin(),
                fallingBubbles.end(),
                [](const FallingBubble& b) {

                    return b.pos.y >
                        GameConfig::ScreenHeight + 100;
                }
            ),
            fallingBubbles.end()
        );
    }

private:

    void AddFallingBubble(int r, int c, BubbleColor color) {

        FallingBubble b;

        b.pos = GetPos(r, c);

        b.vel.x = (GetRand(100) - 50) * 0.03f;

        // 最初ちょっと上へ
        b.vel.y = -2.5f;

        b.color = color;

        b.angle = 0.0f;

        b.rotSpeed =
            ((GetRand(100) - 50) * 0.002f);

        fallingBubbles.push_back(b);
    }

    void FindSameColor(
        int r,
        int c,
        BubbleColor color,
        bool visited[GameConfig::GridRows][GameConfig::GridCols],
        std::vector<std::pair<int, int>>& result
    ) {

        if (r < 0 || r >= GameConfig::GridRows ||
            c < 0 || c >= GameConfig::GridCols) {
            return;
        }

        if (visited[r][c] || grid[r][c] != color) {
            return;
        }

        visited[r][c] = true;

        result.push_back({ r, c });

        static const int offsets[2][6][2] = {

            { {0,-1}, {0,1}, {-1,-1}, {-1,0}, {1,-1}, {1,0} },
            { {0,-1}, {0,1}, {-1,0}, {-1,1}, {1,0}, {1,1} }
        };

        int type = r & 1;

        for (int i = 0; i < 6; ++i) {

            FindSameColor(
                r + offsets[type][i][0],
                c + offsets[type][i][1],
                color,
                visited,
                result
            );
        }
    }

    void FindConnectedToTop(
        int r,
        int c,
        bool visited[GameConfig::GridRows][GameConfig::GridCols]
    ) {

        if (r < 0 || r >= GameConfig::GridRows ||
            c < 0 || c >= GameConfig::GridCols) {
            return;
        }

        if (visited[r][c]) return;

        if (grid[r][c] == BubbleColor::None) return;

        visited[r][c] = true;

        static const int offsets[2][6][2] = {

            { {0,-1}, {0,1}, {-1,-1}, {-1,0}, {1,-1}, {1,0} },
            { {0,-1}, {0,1}, {-1,0}, {-1,1}, {1,0}, {1,1} }
        };

        int type = r & 1;

        for (int i = 0; i < 6; ++i) {

            FindConnectedToTop(
                r + offsets[type][i][0],
                c + offsets[type][i][1],
                visited
            );
        }
    }

    void DropFloatingBubbles() {

        bool visited[GameConfig::GridRows][GameConfig::GridCols] = { false };

        // 天井と繋がる泡を探索
        for (int c = 0; c < GameConfig::GridCols; ++c) {

            if (grid[0][c] != BubbleColor::None) {

                FindConnectedToTop(0, c, visited);
            }
        }

        // 浮いてる泡を消す
        for (int r = 0; r < GameConfig::GridRows; ++r) {
            for (int c = 0; c < GameConfig::GridCols; ++c) {

                if (grid[r][c] != BubbleColor::None &&
                    !visited[r][c]) {

                    BubbleColor color = grid[r][c];

                    AddFallingBubble(r, c, color);

                    grid[r][c] = BubbleColor::None;
                }
            }
        }
    }

public:

    void Draw() const {

        for (int r = 0; r < GameConfig::GridRows; ++r) {
            for (int c = 0; c < GameConfig::GridCols; ++c) {

                if (grid[r][c] == BubbleColor::None) continue;

                DrawBubble(GetPos(r, c), grid[r][c]);
            }
        }

        for (const auto& b : fallingBubbles) {

            int handle = bubbleHandles[(int)b.color];

            DrawRotaGraph(
                (int)b.pos.x,
                (int)b.pos.y,
                0.13,
                b.angle,
                handle,
                TRUE
            );
        }
    }

    void DrawBubble(Vector2 p, BubbleColor c) const {

        if (c == BubbleColor::None) return;

        int handle = bubbleHandles[(int)c];

        DrawRotaGraph(
            (int)p.x,
            (int)p.y,
            0.13,
            0.0,
            handle,
            TRUE
        );
    }
};