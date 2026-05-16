#include "DxLib.h"
#include "BubbleManager.h"
#include <cmath>

class Shooter {

    float angle = DX_PI_F / 2.0f;

    // âÒì]ë¨ìx
    static constexpr float rotationSpeed = 0.03f;

    int playerHandle;

public:
    Shooter() {
        playerHandle = LoadGraph("Data/player.png");
    }

public:
    void Update() {

        // AÉLÅ[Ç≈ç∂âÒì]
        if (CheckHitKey(KEY_INPUT_A)) {
            angle += rotationSpeed;
        }

        // DÉLÅ[Ç≈âEâÒì]
        if (CheckHitKey(KEY_INPUT_D)) {
            angle -= rotationSpeed;
        }

        angle = GameConfig::Clamp(
            angle,
            0.2f,
            DX_PI_F - 0.2f
        );
    }

    void Draw() const {

        float centerX = GameConfig::ScreenWidth / 2.0f;
        float centerY = 420.0f;

        // ÉKÉCÉhÉâÉCÉì
        float tx = centerX + cosf(angle) * 120.0f;
        float ty = centerY - sinf(angle) * 120.0f;

        DrawLineAA(
            centerX,
            centerY,
            tx,
            ty,
            GetColor(255, 255, 255),
            2.0f
        );

        // î≠éÀë‰âÊëúï`âÊ
        DrawRotaGraph(
            centerX,
            centerY - 27.0f,
            0.18,
            0.0,
            playerHandle,
            TRUE
        );
    }

    float GetAngle() const {
        return angle;
    }
};

class Bullet {

    Vector2 pos;
    Vector2 vel;

    BubbleColor color = BubbleColor::None;

    bool active = false;

public:

    void Launch(float angle, BubbleColor c) {

        pos = {
            GameConfig::ScreenWidth / 2.0f,
            420.0f
        };

        vel = {
            cosf(angle) * GameConfig::ShootSpeed,
            -sinf(angle) * GameConfig::ShootSpeed
        };

        color = c;

        active = true;
    }

    void Update() {

        if (!active) return;

        pos.x += vel.x;
        pos.y += vel.y;

        // ï«îΩéÀ
        if (
            pos.x <
            GameConfig::FieldLeft + GameConfig::Radius ||

            pos.x >
            GameConfig::FieldRight - GameConfig::Radius
            ) {

            vel.x *= -1.0f;

            pos.x = GameConfig::Clamp(pos.x,
                GameConfig::FieldLeft + GameConfig::Radius,
                GameConfig::FieldRight - GameConfig::Radius
            );
        }

        // âÊñ äO
        if (pos.y < 0 || pos.y > 480) {
            active = false;
        }
    }

    void Draw(const BubbleManager& bm) const {

        if (active) {

            bm.DrawBubble(pos, color);
        }
    }

    bool IsActive() const 
    {
        return active;
    }

    void Deactivate() 
    {
        active = false;
    }

    Vector2 GetPos() const
    {
        return pos;
    }

    BubbleColor GetColor() const 
    {
        return color;
    }
};

int WINAPI WinMain(HINSTANCE,HINSTANCE,LPSTR,int)
{
    SetGraphMode(GameConfig::ScreenWidth,GameConfig::ScreenHeight,32);

    ChangeWindowMode(TRUE);

    if (DxLib_Init() == -1)
    {
        return -1;
    }

    SetDrawScreen(DX_SCREEN_BACK);

    // îwåiâÊëú
    int bgHandle = LoadGraph("Data/Puzzle.png");

    BubbleManager bm;

    Shooter shooter;

    Bullet bullet;

    BubbleColor currentColor =
        static_cast<BubbleColor>(GetRand(3));

    BubbleColor nextColor =
        static_cast<BubbleColor>(GetRand(3));

    int lastSpaceState = 0;

    // ==================================================
    // î≠éÀÉJÉEÉìÉg
    // ==================================================
    int shotCount = 0;

    while (
        ProcessMessage() == 0 &&!CheckHitKey(KEY_INPUT_ESCAPE)) 
    {

        ClearDrawScreen();

        // îwåiï`âÊ
        DrawGraph(0, 0, bgHandle, TRUE);

        // ì¸óÕèÛë‘éÊìæ
        int currentSpaceState =
            CheckHitKey(KEY_INPUT_SPACE);

        // ÉVÉÖÅ[É^Å[çXêV
        shooter.Update();

        // ==================================================
        // î≠éÀ
        // ==================================================
        if (
            currentSpaceState &&!lastSpaceState &&!bullet.IsActive()) 
        {

            bullet.Launch(shooter.GetAngle(),currentColor);

            // î≠éÀêîâ¡éZ
            shotCount++;

            // éüíeÇåªç›íeÇ÷
            currentColor = nextColor;

            // êVÇµÇ¢éüíe
            nextColor =
                static_cast<BubbleColor>(GetRand(3));
        }

        lastSpaceState = currentSpaceState;

        // ==================================================
        // 8î≠Ç≤Ç∆Ç…ìVà‰â∫ç~
        // ==================================================
        if (shotCount >= 8) {

            bm.DropCeiling();

            shotCount = 0;
        }

        // ==================================================
        // íeçXêV
        // ==================================================
        if (bullet.IsActive()) {

            bullet.Update();

            int r, c;

            if (
                bm.CheckCollision(bullet.GetPos(),r,c))
            {

                // îzíu
                auto pos = bm.SetBubble(bullet.GetPos(),bullet.GetColor());

                // è¡ãéèàóù
                bm.ProcessChain(pos.first,pos.second);

                bullet.Deactivate();
            }
        }

        // ==================================================
        // ÉQÅ[ÉÄÉIÅ[ÉoÅ[
        // ==================================================
        if (bm.IsGameOver()) {

            DrawFormatString(250,220,GetColor(255, 0, 0),"GAME OVER");

            ScreenFlip();

            WaitKey();

            break;
        }

        // ==================================================
        // óéâ∫çXêV
        // ==================================================
        bm.UpdateFalling();

        // ==================================================
        // ï`âÊ
        // ==================================================
        bm.Draw();

        shooter.Draw();

        bullet.Draw(bm);

        // ==================================================
        // éüíeï\é¶
        // ==================================================
        if (!bullet.IsActive()) {

            // åªç›íe
            bm.DrawBubble(
                {
                    GameConfig::ScreenWidth / 2.0f,420.0f
                },
                currentColor
            );

            // éüíe
            bm.DrawBubble(
                {
                    GameConfig::ScreenWidth / 2.0f - 60.0f,445.0f
                },
                nextColor
            );
        }

        ScreenFlip();
    }

    DxLib_End();

    return 0;
}