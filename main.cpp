/*
Error:
1. i cant get the rendering with source_rectangle and destination_recatangle quite right
   But if choose to simply use NULL, everything works
2. The current program just crashes
3. sprite_map::load_sprite() doesnt work
4. Calling sprite_map::load_sprite() will always result in crashes, no matter if you set any breakpoint in it
5. hitbox::GetCenterX() and hitbox::GetCenterY() is broken
6. bullet lifetime could not excede 1000 milisecond
Solution:
1. it turned out i need to initialize source_rectangle x and y. bcs apparently default value of int is not 0. So i assign it manually
2, 3, 4. it crashes before the sprite loading call were done before the texture loading, so undefined behaviour follows.
5. I rewrote the fucking class
6. When bullet lifetime excede 1000 milisecond, it means the required delay sometimes go into the negative, because...duh, we dont need delay, we need acceleration
   But i digress. Because the operation that was used to determine the required delay are between two UNSIGNED integer, the buffer overflow
   Changing the result to around 4 trillion.
*/

/*
Clues:
1. Operation Priority
2. Unintended implicit conversion, reducing accuracy and maybe change the value altogether
3. try using static_cast or floating point literal
4. there is a possibility of sequantiality being importang in AgeBullet() function.
5. floating point truncation !!!!
    0 - (some float number thats less than zero) = -0.787878 >> truncated >> -0 = 0
    This shit will mess your trajectorial movement
    Case in point, AgeBullet() function
6. Be careful with the detail !!
    I broke my function because it got passed a copied value, not a reference.......
7. Dont fiddle with unsigned int unless you know what you are doing
*/
/*
TO DO:
1.. implement bullet automaton behaviour
2. employ enemy movement
*/



#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <stdio.h>
#include <iostream>
#include <cmath>
#include <math.h>
#include <vector>
#include <string>
#include <map>
#include <iterator>
#include <chrono>
#include <thread>

#include "boilerplate.h"



SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Surface* pTemporarySurface = NULL;
SDL_Event EventBuffer;

int main(int argc, char* args[])
{
    // Initializing sdl, windows, and rendering context
    bool IsInitialized = false;
    if( SDL_Init(SDL_INIT_EVERYTHING) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    else
    {
        TTF_Init();
        IsInitialized = true;
        window = SDL_CreateWindow("Pirel's Whirlwind", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_FULLSCREEN);
        if(window == NULL)
        {
            printf("Cannot Initialize Window\n");
            IsInitialized = false;
        }
        else
        {
            renderer = SDL_CreateRenderer(window, -1, 0);
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
        }
    }

    if(IsInitialized)        // Start The Game
    {
        // Initialize and load game asset
        keystate KeyboardInput;
        const Uint8* KeyboardState = SDL_GetKeyboardState(NULL);

        std::vector<SDL_Texture*> TextureCache;
        sprite_map SpriteCache;
        bullet_map BulletCache;
        hitbox_map HitboxCache;

        int ENTITY_LIFE_TIME = 3000;

        while(!LoadTexture("Bullet1.bmp", TextureCache, renderer)){} 
        while(!LoadTexture("Bullet2.bmp", TextureCache, renderer)){}
        while(!LoadTexture("Bullet3.bmp", TextureCache, renderer)){}
        while(!LoadTexture("Player.bmp", TextureCache, renderer)){}
        while(!LoadTextTexture("Whirlwind, Programmed by Muhammad Pirel", TextureCache, renderer)){}
        while(!LoadTextTexture("WASD to move", TextureCache, renderer)){}
        while(!LoadTextTexture("Right Shift to slow down time", TextureCache, renderer)){}
        while(!LoadTextTexture("Graze bullets to score points", TextureCache, renderer)){}

        // Enemy Character
        SpriteCache.load_sprite(TextureCache[3], 1, 1, 1000000000);
        HitboxCache.load_hitbox(1, 1, 1000000000);
        HitboxCache.find(1, 1)->Hitbox.MoldAgainstSprite(SpriteCache.find(1, 1)->Sprite);
        Vector2 Arm;
        Arm.ChangeX(1);
        Arm.ChangeY(1);
        Arm.normalize();

        // Player Character
        SpriteCache.load_sprite(TextureCache[3], 1, 12, 1000000000);
        HitboxCache.load_hitbox(1, 12, 1000000000);
        HitboxCache.find(1, 12)->Hitbox.MoldAgainstSprite(SpriteCache.find(1, 12)->Sprite);
        

        //Intro
        SpriteCache.load_sprite(TextureCache[4], 1, 13, 1000000000);
        SpriteCache.load_sprite(TextureCache[5], 2, 13, 1000000000);
        SpriteCache.load_sprite(TextureCache[6], 3, 13, 1000000000);
        SpriteCache.load_sprite(TextureCache[7], 4, 13, 1000000000);
        SpriteCache.find(2,13)->Sprite.DestinationRectangle.y += SpriteCache.find(1,13)->Sprite.DestinationRectangle.h + SpriteCache.find(1,13)->Sprite.DestinationRectangle.y;
        SpriteCache.find(3,13)->Sprite.DestinationRectangle.y += SpriteCache.find(2,13)->Sprite.DestinationRectangle.h + SpriteCache.find(2,13)->Sprite.DestinationRectangle.y;
        SpriteCache.find(4,13)->Sprite.DestinationRectangle.y += SpriteCache.find(3,13)->Sprite.DestinationRectangle.h + SpriteCache.find(3,13)->Sprite.DestinationRectangle.y;
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, SpriteCache.find(1, 13)->Sprite.pTexture, &SpriteCache.find(1, 13)->Sprite.SourceRectangle, &SpriteCache.find(1, 13)->Sprite.DestinationRectangle);
        SDL_RenderCopy(renderer, SpriteCache.find(2, 13)->Sprite.pTexture, &SpriteCache.find(2, 13)->Sprite.SourceRectangle, &SpriteCache.find(2, 13)->Sprite.DestinationRectangle);
        SDL_RenderCopy(renderer, SpriteCache.find(3, 13)->Sprite.pTexture, &SpriteCache.find(3, 13)->Sprite.SourceRectangle, &SpriteCache.find(3, 13)->Sprite.DestinationRectangle);
        SDL_RenderCopy(renderer, SpriteCache.find(4, 13)->Sprite.pTexture, &SpriteCache.find(4, 13)->Sprite.SourceRectangle, &SpriteCache.find(4, 13)->Sprite.DestinationRectangle);
        SDL_RenderPresent(renderer);
        SDL_Delay(4000);
        //TextureCache.pop_back();
        //TextureCache.pop_back();
        //TextureCache.pop_back();

        // game loop
        int TICK = 1;
        int Begin;
        int End;
        int Period = 0;             // Period of previous frame
        int IntendedPeriod = 20;    // How long we actually want the frame to last
        int dimension = 0;          // The dimension of a set of bullet in bullet_map
        bool IsWinning = true;
        int Point = 0;
        while(IsWinning)
        {
            Begin = SDL_GetTicks();

            ENTITY_LIFE_TIME = 3000; // Standard Entity Lifetime
            IntendedPeriod = 20;     // Intended Period of a frame

            SDL_PumpEvents();       // Update KeyboardState
            KeyboardInput.reset();  // Reset KeyboardInput data
            if(KeyboardState[SDL_SCANCODE_W]){KeyboardInput.Pressed_W = true;}
            if(KeyboardState[SDL_SCANCODE_A]){KeyboardInput.Pressed_A = true;}
            if(KeyboardState[SDL_SCANCODE_S]){KeyboardInput.Pressed_S = true;}
            if(KeyboardState[SDL_SCANCODE_D]){KeyboardInput.Pressed_D = true;}
            if(KeyboardState[SDL_SCANCODE_RSHIFT]){KeyboardInput.Pressed_RSHIFT = true;}
            if(KeyboardState[SDL_SCANCODE_ESCAPE]){KeyboardInput.Pressed_ESC = true;}

            // Cache aging
            BulletCache.age_cache(IntendedPeriod);
            SpriteCache.age_cache(IntendedPeriod);
            HitboxCache.age_cache(IntendedPeriod);

            // Keyboard Event Management
            if(KeyboardInput.Pressed_W){HitboxCache.find(1, 12)->Hitbox.MoveY(-10);}
            if(KeyboardInput.Pressed_A){HitboxCache.find(1, 12)->Hitbox.MoveX(-10);}
            if(KeyboardInput.Pressed_S){HitboxCache.find(1, 12)->Hitbox.MoveY(10);}
            if(KeyboardInput.Pressed_D){HitboxCache.find(1, 12)->Hitbox.MoveX(10);}
            if(KeyboardInput.Pressed_RSHIFT){IntendedPeriod = 100;}
            SpriteCache.find(1, 12)->Sprite.DestinationRectangle.x = HitboxCache.find(1, 12)->Hitbox.GetSDLRectX();
            SpriteCache.find(1, 12)->Sprite.DestinationRectangle.y = HitboxCache.find(1, 12)->Hitbox.GetSDLRectY();

            // Enemy Movement
            HitboxCache.find(1, 1)->Hitbox.SetCenterX(640);
            HitboxCache.find(1, 1)->Hitbox.SetCenterY(360);
            SpriteCache.find(1, 1)->Sprite.DestinationRectangle.x = HitboxCache.find(1, 1)->Hitbox.GetSDLRectX();
            SpriteCache.find(1, 1)->Sprite.DestinationRectangle.y = HitboxCache.find(1, 1)->Hitbox.GetSDLRectY();
            /*if(TICK % 5 == 0)
            {
                HitboxCache.find(1, 1)->Hitbox.SetCenterX(200.0f);
                HitboxCache.find(1, 1)->Hitbox.SetCenterY(200.0f);
            }
            else if(TICK % 5 == 1)
            {
                HitboxCache.find(1, 1)->Hitbox.SetCenterX(700.0f);
                HitboxCache.find(1, 1)->Hitbox.SetCenterY(100.0f);
            }
            else if(TICK % 5 == 2)
            {
                HitboxCache.find(1, 1)->Hitbox.SetCenterX(100.0f);
                HitboxCache.find(1, 1)->Hitbox.SetCenterY(700.0f);
            }
            else if(TICK % 5 == 3)
            {
                HitboxCache.find(1, 1)->Hitbox.SetCenterX(1000.0f);
                HitboxCache.find(1, 1)->Hitbox.SetCenterY(400.0f);
            }
            else if(TICK % 5 == 4)
            {
                HitboxCache.find(1, 1)->Hitbox.SetCenterX(1000.0f);
                HitboxCache.find(1, 1)->Hitbox.SetCenterY(700.0f);
            }
            else{}
            SpriteCache.find(1, 1)->Sprite.DestinationRectangle.x = HitboxCache.find(1, 1)->Hitbox.GetSDLRectX();
            SpriteCache.find(1, 1)->Sprite.DestinationRectangle.y = HitboxCache.find(1, 1)->Hitbox.GetSDLRectY();*/

            // Bullet Spawning From Player Position | DIMENSION = 2
            dimension = 2;
            SpriteCache.load_sprite(TextureCache[0], TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.load_hitbox(TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.find(TICK, dimension)->Hitbox.MoldAgainstSprite(SpriteCache.find(TICK, dimension)->Sprite);
            BulletCache.load_bullet(TICK, dimension, ENTITY_LIFE_TIME,
            HitboxCache.find(1, 1)->Hitbox.GetCenterX(),
            HitboxCache.find(1, 1)->Hitbox.GetCenterY(), 0.3f, -TICK * 10);

            // Bullet Spawning From Player Position | DIMENSION = 3
            dimension = 3;
            SpriteCache.load_sprite(TextureCache[0], TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.load_hitbox(TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.find(TICK, dimension)->Hitbox.MoldAgainstSprite(SpriteCache.find(TICK, dimension)->Sprite);
            BulletCache.load_bullet(TICK, dimension, ENTITY_LIFE_TIME,
            HitboxCache.find(1, 1)->Hitbox.GetCenterX(),
            HitboxCache.find(1, 1)->Hitbox.GetCenterY(), 0.5f, TICK * 5);
            //BulletCache.find(tick, 2)->Bullet.Mode = 'N';
            //BulletCache.find(tick, 2)->Bullet.SinePower = 20.0f;

            // Bullet Spawning From Player Position | DIMENSION = 4
            dimension = 4;
            SpriteCache.load_sprite(TextureCache[1], TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.load_hitbox(TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.find(TICK, dimension)->Hitbox.MoldAgainstSprite(SpriteCache.find(TICK, dimension)->Sprite);
            BulletCache.load_bullet(TICK, dimension, ENTITY_LIFE_TIME,
            HitboxCache.find(1, 1)->Hitbox.GetCenterX(),
            HitboxCache.find(1, 1)->Hitbox.GetCenterY(), 1.0f, (std::cos((static_cast<float>(TICK * 10) / 180.0f) * PI) + std::sin((static_cast<float>(TICK * 5) / 180.0f) * PI)) * 15);
            BulletCache.find(TICK, dimension)->Bullet.Acceleration = -0.001f;
            BulletCache.find(TICK, dimension)->Bullet.MinimalSpeed = -1.0f;

            // Bullet Spawning From Player Position | DIMENSION = 5
            dimension = 5;
            SpriteCache.load_sprite(TextureCache[1], TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.load_hitbox(TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.find(TICK, dimension)->Hitbox.MoldAgainstSprite(SpriteCache.find(TICK, dimension)->Sprite);
            BulletCache.load_bullet(TICK, dimension, ENTITY_LIFE_TIME,
            HitboxCache.find(1, 1)->Hitbox.GetCenterX(),
            HitboxCache.find(1, 1)->Hitbox.GetCenterY(), 1.0f, (std::cos((static_cast<float>(TICK * 10) / 180.0f) * PI) + std::sin((static_cast<float>(TICK * 5) / 180.0f) * PI)) * 15 - 90);
            BulletCache.find(TICK, dimension)->Bullet.Acceleration = -0.001f;
            BulletCache.find(TICK, dimension)->Bullet.MinimalSpeed = -1.0f;

            // Bullet Spawning From Player Position | DIMENSION = 6
            dimension = 6;
            SpriteCache.load_sprite(TextureCache[1], TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.load_hitbox(TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.find(TICK, dimension)->Hitbox.MoldAgainstSprite(SpriteCache.find(TICK, dimension)->Sprite);
            BulletCache.load_bullet(TICK, dimension, ENTITY_LIFE_TIME,
            HitboxCache.find(1, 1)->Hitbox.GetCenterX(),
            HitboxCache.find(1, 1)->Hitbox.GetCenterY(), 1.0f, (std::cos((static_cast<float>(TICK * 10) / 180.0f) * PI) + std::sin((static_cast<float>(TICK * 5) / 180.0f) * PI)) * 15 - 180);
            BulletCache.find(TICK, dimension)->Bullet.Acceleration = -0.001f;
            BulletCache.find(TICK, dimension)->Bullet.MinimalSpeed = -1.0f;

            // Bullet Spawning From Player Position | DIMENSION = 7
            dimension = 7;
            SpriteCache.load_sprite(TextureCache[1], TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.load_hitbox(TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.find(TICK, dimension)->Hitbox.MoldAgainstSprite(SpriteCache.find(TICK, dimension)->Sprite);
            BulletCache.load_bullet(TICK, dimension, ENTITY_LIFE_TIME,
            HitboxCache.find(1, 1)->Hitbox.GetCenterX(),
            HitboxCache.find(1, 1)->Hitbox.GetCenterY(), 1.0f, (std::cos((static_cast<float>(TICK * 10) / 180.0f) * PI) + std::sin((static_cast<float>(TICK * 5) / 180.0f) * PI)) * 15 - 270);
            BulletCache.find(TICK, dimension)->Bullet.Acceleration = -0.001f;
            BulletCache.find(TICK, dimension)->Bullet.MinimalSpeed = -1.0f;

            // Bullet Spawning From Player Position | DIMENSION = 8
            dimension = 8;
            SpriteCache.load_sprite(TextureCache[2], TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.load_hitbox(TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.find(TICK, dimension)->Hitbox.MoldAgainstSprite(SpriteCache.find(TICK, dimension)->Sprite);
            BulletCache.load_bullet(TICK, dimension, ENTITY_LIFE_TIME,
            HitboxCache.find(1, 1)->Hitbox.GetCenterX(),
            HitboxCache.find(1, 1)->Hitbox.GetCenterY(), 1.0f, (std::cos((static_cast<float>(TICK * 10) / 180.0f) * PI) + std::sin((static_cast<float>(TICK * 5) / 180.0f) * PI)) * 15 + 45);
            BulletCache.find(TICK, dimension)->Bullet.Acceleration = -0.001f;
            BulletCache.find(TICK, dimension)->Bullet.MinimalSpeed = -1.0f;

            // Bullet Spawning From Player Position | DIMENSION = 9
            dimension = 9;
            SpriteCache.load_sprite(TextureCache[2], TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.load_hitbox(TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.find(TICK, dimension)->Hitbox.MoldAgainstSprite(SpriteCache.find(TICK, dimension)->Sprite);
            BulletCache.load_bullet(TICK, dimension, ENTITY_LIFE_TIME,
            HitboxCache.find(1, 1)->Hitbox.GetCenterX(),
            HitboxCache.find(1, 1)->Hitbox.GetCenterY(), 1.0f, (std::cos((static_cast<float>(TICK * 10) / 180.0f) * PI) + std::sin((static_cast<float>(TICK * 5) / 180.0f) * PI)) * 15 + 135);
            BulletCache.find(TICK, dimension)->Bullet.Acceleration = -0.001f;
            BulletCache.find(TICK, dimension)->Bullet.MinimalSpeed = -1.0f;

            // Bullet Spawning From Player Position | DIMENSION = 10
            dimension = 10;
            SpriteCache.load_sprite(TextureCache[2], TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.load_hitbox(TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.find(TICK, dimension)->Hitbox.MoldAgainstSprite(SpriteCache.find(TICK, dimension)->Sprite);
            BulletCache.load_bullet(TICK, dimension, ENTITY_LIFE_TIME,
            HitboxCache.find(1, 1)->Hitbox.GetCenterX(),
            HitboxCache.find(1, 1)->Hitbox.GetCenterY(), 1.0f, (std::cos((static_cast<float>(TICK * 10) / 180.0f) * PI) + std::sin((static_cast<float>(TICK * 5) / 180.0f) * PI)) * 15 + 225);
            BulletCache.find(TICK, dimension)->Bullet.Acceleration = -0.001f;
            BulletCache.find(TICK, dimension)->Bullet.MinimalSpeed = -1.0f;

            // Bullet Spawning From Player Position | DIMENSION = 11
            dimension = 11;
            SpriteCache.load_sprite(TextureCache[2], TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.load_hitbox(TICK, dimension, ENTITY_LIFE_TIME);
            HitboxCache.find(TICK, dimension)->Hitbox.MoldAgainstSprite(SpriteCache.find(TICK, dimension)->Sprite);
            BulletCache.load_bullet(TICK, dimension, ENTITY_LIFE_TIME,
            HitboxCache.find(1, 1)->Hitbox.GetCenterX(),
            HitboxCache.find(1, 1)->Hitbox.GetCenterY(), 1.0f, (std::cos((static_cast<float>(TICK * 10) / 180.0f) * PI) + std::sin((static_cast<float>(TICK * 5) / 180.0f) * PI)) * 15 + 315);
            BulletCache.find(TICK, dimension)->Bullet.Acceleration = -0.001f;
            BulletCache.find(TICK, dimension)->Bullet.MinimalSpeed = -1.0f;




            // Bullet Hitbox Updating
            for(int n = 0; n < HitboxCache.get_size(); n++)
            {
                int Key = HitboxCache.iterative_access(n)->Key;
                for(int Dimension = 2; Dimension <= dimension; Dimension++)
                {
                    if(HitboxCache.iterative_access(n)->Dimension == Dimension)
                    {
                        HitboxCache.iterative_access(n)->Hitbox.SetCenterX(BulletCache.find(Key, Dimension)->Bullet.GetX());
                        HitboxCache.iterative_access(n)->Hitbox.SetCenterY(BulletCache.find(Key, Dimension)->Bullet.GetY());
                    }
                }
            }

            // Bullet Sprite Updating
            for(int n = 0; n < SpriteCache.get_size(); n++)
            {
                int Key = SpriteCache.iterative_access(n)->Key;
                int Dimension = 2; // 2 is the dimension for bullet sprite
                for (int Dimension = 2; Dimension <= dimension; Dimension++)
                {
                    if(SpriteCache.iterative_access(n)->Dimension == Dimension)
                    {
                        SpriteCache.iterative_access(n)->Sprite.DestinationRectangle.x = HitboxCache.find(Key, Dimension)->Hitbox.GetSDLRectX();
                        SpriteCache.iterative_access(n)->Sprite.DestinationRectangle.y = HitboxCache.find(Key, Dimension)->Hitbox.GetSDLRectY();
                    }
                }
            }

            // Player Collision Detection
            for(int n = 0; n < HitboxCache.get_size(); n++)
            {
                int Key = HitboxCache.iterative_access(n)->Key;
                for(int Dimension = 2; Dimension <= dimension; Dimension++)
                {
                    if(HitboxCache.iterative_access(n)->Dimension == Dimension)
                    {
                        hitbox normal_hitbox = HitboxCache.find(1, 12)->Hitbox;
                        hitbox graze_hitbox = HitboxCache.find(1, 12)->Hitbox;
                        graze_hitbox.SetWidth(60);
                        if(hitbox::IsColliding(normal_hitbox, HitboxCache.iterative_access(n)->Hitbox))
                        {
                            IsWinning = false;
                        }
                        if(hitbox::IsColliding(graze_hitbox, HitboxCache.iterative_access(n)->Hitbox))
                        {
                            Point++;
                        }
                    }
                }
            }
            
            SDL_RenderClear(renderer);
            for(int n = 0; n < SpriteCache.get_size(); n++)
            {
                if(SpriteCache.iterative_access(n)->Dimension != 13)
                SDL_RenderCopy(renderer,
                SpriteCache.iterative_access(n)->Sprite.pTexture,
                &SpriteCache.iterative_access(n)->Sprite.SourceRectangle,
                &SpriteCache.iterative_access(n)->Sprite.DestinationRectangle);
            }
            SDL_RenderPresent(renderer);

            End = SDL_GetTicks();
            Period = End - Begin;
            if(static_cast<int>(IntendedPeriod) - static_cast<int>(Period) >= 1)
            {
                SDL_Delay(static_cast<int>(IntendedPeriod) - static_cast<int>(Period));
            }
            else
            {
                SDL_Delay(0);
            }
            if(KeyboardInput.Pressed_ESC){break;}    // when escape is pressed, the main game loop is broken and quit is initialized
            TICK++;
        }

        //Outro
        while(!LoadTextTexture("You Lost ! Your Score Is : " + std::to_string(Point), TextureCache, renderer)){}
        SpriteCache.load_sprite(TextureCache[8], 5, 13, 1000000000);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, SpriteCache.find(5, 13)->Sprite.pTexture, &SpriteCache.find(5, 13)->Sprite.SourceRectangle, &SpriteCache.find(5, 13)->Sprite.DestinationRectangle);        
        SDL_RenderPresent(renderer);
        SDL_Delay(2000);
    }

    ///////////// Quit the Game //////////

    // Free the RAM from all thing SDL
    SDL_DestroyWindow(window);
    SDL_DestroyRenderer(renderer);
    SDL_Quit();

    return 0;
}