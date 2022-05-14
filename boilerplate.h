#ifndef BOILERPLATE

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
#include <thread>
#include <mutex>


const double PI = 3.14159265;
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 760;
std::mutex MutualExclusion;




struct sprite
{
    SDL_Texture* pTexture = NULL;
    SDL_Rect SourceRectangle;
    SDL_Rect DestinationRectangle;
};



struct keystate
{
    bool Pressed_A = false;
    bool Pressed_S = false;
    bool Pressed_D = false;
    bool Pressed_W = false;
    bool Pressed_RSHIFT = false;
    bool Pressed_ESC = false;

    void reset()
    {
        Pressed_A = false;
        Pressed_S = false;
        Pressed_D = false;
        Pressed_W = false;
        Pressed_RSHIFT = false;
        Pressed_ESC = false;
    }
};



class hitbox
{
    private:
    float CenterX;
    float CenterY;
    float Width;
    float Height;
    char Mode;

    public:

    hitbox(int X = 100, int Y = 100, int W = 30, int H = 30, char mode = 'C')
    {
        CenterX = X;
        CenterY = Y;
        Width = W;
        Height = H;
        Mode = mode;
    }

    void SetCenterX(float value){CenterX = value;}
    void SetCenterY(float value){CenterY = value;}
    void SetWidth(float value){Width = value;}
    void SetHeight(float value){Height = value;}
    void SetRectangular(){Mode = 'R';}
    void SetCircular(){Mode = 'C';}

    void MoveX(float value){CenterX += value;}
    void MoveY(float value){CenterY += value;}
    void StretchWidth(float value){Width += value;}
    void StretchHeight(float value){Height += value;}

    float GetCenterX(){return CenterX;}
    float GetCenterY(){return CenterY;}
    float GetWidth(){return Width;}
    float GetHeight(){return Height;}
    char GetMode(){return Mode;}

    SDL_Rect GetSDLRect()
    {
        SDL_Rect buffer;
        buffer.x = CenterX - (Width / 2);
        buffer.y = CenterY - (Height / 2);
        buffer.h = Height;
        buffer.w = Width;
        return buffer;
    }

    int GetSDLRectX(){return CenterX - (Width / 2);}
    int GetSDLRectY(){return CenterY - (Height / 2);}
    int GetSDLRectW(){return Width;}
    int GetSDLRectH(){return Height;}

    void MoldAgainstSprite(sprite Sprite)
    {
        Height = Sprite.DestinationRectangle.h;
        Width = Sprite.DestinationRectangle.w;
        CenterX = Sprite.DestinationRectangle.x + (Width / 2);
        CenterY = Sprite.DestinationRectangle.y + (Height / 2);
    }

    static bool IsColliding(hitbox shape1, hitbox shape2)
    {
        if(shape1.GetMode() == 'C' && shape2.GetMode() == 'C')
        {
            float distance =
            std::sqrt
            (
                (shape1.GetCenterX() - shape2.GetCenterX())*
                (shape1.GetCenterX() - shape2.GetCenterX())+
                (shape1.GetCenterY() - shape2.GetCenterY())*
                (shape1.GetCenterY() - shape2.GetCenterY())
            );

            return (distance <= ((shape1.GetWidth() + shape2.GetWidth()) / 2));   // The width of the bounding box is the diameter of the sphere, so the radius would be half of that
        }
        else if(shape1.GetMode() == 'C' && shape2.GetMode() == 'R')
        {
            return false;
        }
        else if(shape1.GetMode() == 'R' && shape2.GetMode() == 'C')
        {
            return false;
        }
        else if(shape1.GetMode() == 'R' && shape2.GetMode() == 'R')
        {
            return false;
        }
        else
        {
            return false;
        }
    }
};



class Vector2           // # Built for Precision
{
    private:
    double x = 0;
    double y = 0;
    bool IsNormalized = false;

    public:
    Vector2()
    {
        // do nothing
    }

    void normalize()
    {
        double length;
        length = std::sqrt((x * x) + (y * y));
        x /= length;
        y /= length;
        IsNormalized = true;
    }

    void rotate(int degree)
    {
        // Inverse The Rotation
        // This is because normal euclidian rotation that incrementation of y will result in Up wise move
        // But we are using screen coordinate, where the incrementaion of y will result in downward move
        // So to counter that, we reverse the rotation.
        degree = 360 - (degree % 360);   // 30 -> 330 
        // Convert to radians unit
        float radians = degree / 180 * PI;
        // Rotation Matrix
        // x cos theta - y sin theta
        // x sin theta + y cos theta
        float ResultantX = (static_cast<float>(x) * std::cos(radians)) - (static_cast<float>(y) * std::sin(radians));
        float ResultantY = (static_cast<float>(x) * std::sin(radians)) + (static_cast<float>(y) * std::cos(radians));
        // Apply rotation
        x = ResultantX;
        y = ResultantY;
        // We dont need to normalize it further
        // The rotation matrix takes into account the magnitude of the vector
        // So that rotation wont result in magnitude change
    }

    double GetX(){return x;}
    double GetY(){return y;}
    bool IsNormal(){return IsNormalized;}

    void ChangeX(double value)
    {
        x = value;
        IsNormalized = false;
    }

    void ChangeY(double value)
    {
        y = value;
        IsNormalized = false;
    }
};



class bullet
{
    private:
    float PosX;      // i use float here so that AgeBullet() can function propwrly. If i were to use int, miniscule change wont be detected because of how floating point truncation and implicit convertion works, it would result to the coordinate stuck at 0,0
    float PosY;
    Vector2 DirectionVector;
    float Speed;                                  // pixel per milisecond
    int Tick = 0;                          // Milisecond since instantiation

    public:

    float SinePower = 100.0f;
    float Acceleration = 0.0f;
    float MinimalSpeed = 0.0f;
    float MaximalSpeed = 100.0f;

    bullet(int X = 300, int Y = 300, float speed = 1, int direction_degree = 135)
    {
        PosX = X;
        PosY = Y;
        Speed = speed;
        DirectionVector.ChangeX(std::cos((static_cast<float>(direction_degree) / 180.0f) * PI));              // USE FLOATING POINT LITERAL!!!
        DirectionVector.ChangeY(-(std::sin((static_cast<float>(direction_degree) / 180.0f) * PI)));
        DirectionVector.normalize();
    }

    void rotate(int degree)
    {
        DirectionVector.rotate(degree);
    }

    void AgeBullet(int time)
    {
        Tick++;
        // Accelerate
        Speed += Acceleration * time;
        if(Speed <= MinimalSpeed){Speed = MinimalSpeed;}
        if(Speed >= MaximalSpeed){Speed = MaximalSpeed;}
        // Trigonometric Sway Calculation
        float Sway = std::sin(Tick / 180 * PI) * SinePower;  // Sway = 0, fuck
        Vector2 SwayDirection = DirectionVector;             // This part however, works
        SwayDirection.rotate(90);
        // Normal Movement
        PosX = PosX + (DirectionVector.GetX() * static_cast<float>(Speed ) * static_cast<float>(time));
        PosY = PosY + (DirectionVector.GetY() * static_cast<float>(Speed ) * static_cast<float>(time));
        // Sway Added
        PosX += SwayDirection.GetX() * Sway;
        PosY += SwayDirection.GetY() * Sway;
    }

    int GetX(){return static_cast<int>(PosX);}
    int GetY(){return static_cast<int>(PosY);}
    double GetDirectionVectorX()
    {
        return DirectionVector.GetX();
    }
    double GetDirectionVectorY()
    {
        return DirectionVector.GetY();
    }
};



// Mapping model will be based on one to one correlation, no key will be duplicated
class sprite_map
{
    struct pair
    {
        int Key;
        int Dimension;
        sprite Sprite;
        int lifespan_in_miliseconds = 1000;
    };

    private:
    std::vector<pair, std::allocator<pair>> container;

    public:
    sprite_map()
    {
        container.reserve(100000);
    }

    // insert a key-value pair, note that overlapping key is not possible
    bool insert(int key, int Dimension, sprite Sprite, int lifespan = 1000)
    {
        bool duplicated = false;
        for(int n = 0; n < container.size(); n++)
        {
            if(container[n].Key == key && container[n].Dimension == Dimension){duplicated = true; break;}
        }

        if(!duplicated)
        {
            pair buffer;
            buffer.Key = key;
            buffer.Dimension = Dimension;
            buffer.Sprite = Sprite;
            buffer.lifespan_in_miliseconds = lifespan;
            container.push_back(buffer);
        }

        return !duplicated;
    }

    // Find all sprites that associate with "key" key
    pair* find(int Key, int Dimension)
    {
        for(int n = 0; n < container.size(); n++)
        {
            if(container[n].Key == Key && container[n].Dimension == Dimension)
            {
                return &container[n];     // return pointer of the associated pair (based on key)
            }
            else
            {
                // do nothing
            }
        }
        // if pair pointer cant be returned, return null pointer
        return nullptr;
    }

    // Return true if erasement succeed, because maybe there is(are) another element that can be erased. Use while control flow to delete all associated element
    bool erase(int Key, int Dimension)
    {
        bool is_successful = false;
        std::vector<pair>::iterator iterator;

        for(int n = 0; n < container.size(); n++)
        {
            if(container[n].Key == Key && container[n].Dimension == Dimension)
            {
                iterator = container.begin();
                for(int m = 0; m < n; m++)
                {
                    iterator++;
                }
                container.erase(iterator);
                is_successful = true;
                break;   // Stop iteration, one erasementis enough
            }
            else
            {
                // do nothing
            }
        }

        return is_successful;
    }

    pair* iterative_access(int n = 0)
    {
        return &container[n];
    }

    int get_size()
    {
        return container.size();
    }

    void age_cache(int time_ms)
    {
        struct int2{int x; int y;};
        std::vector<int2> dimension_key_pair_bucket;

        // reduce lifespan
        for(int n = 0; n < container.size(); n++)
        {
            container[n].lifespan_in_miliseconds -= time_ms;
            if(container[n].lifespan_in_miliseconds <= 0)
            {
                int2 buffer;
                buffer.x = container[n].Dimension;
                buffer.y = container[n].Key;
                dimension_key_pair_bucket.push_back(buffer);
            }
            else
            {
                // do nothing
            }
        }

        // Erase dying sprite (lifespan <= 0)
        for(int n = 0; n < dimension_key_pair_bucket.size(); n++)
        {
            erase(dimension_key_pair_bucket[n].y, dimension_key_pair_bucket[n].x);
        }
    }

    bool load_sprite(SDL_Texture* ptexture, int key, int Dimension, int lifespan = 1000)
    {
        sprite buffer_sprite;
        buffer_sprite.pTexture = ptexture;
        SDL_QueryTexture(ptexture, NULL, NULL, &buffer_sprite.SourceRectangle.w, &buffer_sprite.SourceRectangle.h);
        buffer_sprite.SourceRectangle.x = 0;
        buffer_sprite.SourceRectangle.y = 0;
        buffer_sprite.DestinationRectangle.w = buffer_sprite.SourceRectangle.w;
        buffer_sprite.DestinationRectangle.h = buffer_sprite.SourceRectangle.h;
        buffer_sprite.DestinationRectangle.x = 10;
        buffer_sprite.DestinationRectangle.y = 10;
        insert(key, Dimension, buffer_sprite, lifespan);
        return true;
    }
};



// Mapping model will be based on one to one correlation, no key will be duplicated
class bullet_map
{
   struct pair
    {
        int Key;
        int Dimension;
        bullet Bullet;
        int lifespan_in_miliseconds = 1000;
    };

    private:
    std::vector<pair, std::allocator<pair>> container;

    public:

    bullet_map()
    {
        container.reserve(100000);
    }

    // insert a key-value pair, note that overlapping key is not possible
    bool insert(int key, int dimension, bullet buffer_bullet, int lifespan = 1000)
    {
        bool duplicated = false;
        for(int n = 0; n < container.size(); n++)
        {
            if(container[n].Key == key && container[n].Dimension == dimension){duplicated = true; break;}
        }

        if(!duplicated)
        {
            pair buffer;
            buffer.Key = key;
            buffer.Dimension = dimension;
            buffer.Bullet = buffer_bullet;
            buffer.lifespan_in_miliseconds = lifespan;
            container.push_back(buffer);
        }

        return !duplicated;
    }

    // Find all sprites that associate with "key" key
    pair* find(int Key, int Dimension)
    {
        for(int n = 0; n < container.size(); n++)
        {
            if(container[n].Key == Key && container[n].Dimension == Dimension)
            {
                return &container[n];     // return pointer of the associated pair (based on key)
            }
            else
            {
                // do nothing
            }
        }
        // if pair pointer cant be returned, return null pointer
        return nullptr;
    }

    bool erase(int Key, int Dimension)
    {
        bool is_successful = false;
        std::vector<pair>::iterator iterator;

        for(int n = 0; n < container.size(); n++)
        {
            if(container[n].Key == Key && container[n].Dimension == Dimension)
            {
                iterator = container.begin();
                for(int m = 0; m < n; m++)
                {
                    iterator++;
                }
                container.erase(iterator);
                is_successful = true;
                break;   // Stop iteration, one erasementis enough
            }
            else
            {
                // do nothing
            }
        }

        return is_successful;
    }

    pair* iterative_access(int n = 0)
    {
        return &container[n];
    }

    int get_size()
    {
        return container.size();
    }

    void age_cache(int time_ms)
    {
        struct int2{int x; int y;};
        std::vector<int2> dimension_key_pair_bucket;

        // reduce lifespan
        for(int n = 0; n < container.size(); n++)
        {
            container[n].lifespan_in_miliseconds -= time_ms;
            container[n].Bullet.AgeBullet(time_ms);
            if(container[n].lifespan_in_miliseconds <= 0)
            {
                int2 buffer;
                buffer.x = container[n].Dimension;
                buffer.y = container[n].Key;
                dimension_key_pair_bucket.push_back(buffer);
            }
            else
            {
                // do nothing
            }
        }

        // Erase dying bullet (lifespan <= 0)
        for(int n = 0; n < dimension_key_pair_bucket.size(); n++)
        {
            erase(dimension_key_pair_bucket[n].y, dimension_key_pair_bucket[n].x);
        }
    }

    bool load_bullet(int Key, int Dimension, int Lifespan = 1000, int X = 100, int Y = 100, float Speed = 1, int Direction = 135)
    {
        bullet bullet_buffer(X, Y, Speed, Direction);
        insert(Key, Dimension, bullet_buffer, Lifespan);
        return true;
    }
};



// Mapping model will be based on one to one correlation, no key will be duplicated
class hitbox_map
{
    struct pair
    {
        int Key;
        int Dimension;
        hitbox Hitbox;
        int lifespan_in_miliseconds = 1000;
    };

    private:
    std::vector<pair, std::allocator<pair>> container;

    public:

    hitbox_map()
    {
        container.reserve(100000);
    }

    // insert a key-value pair, note that overlapping key is not possible
    bool insert(int key, int Dimension, hitbox Hitbox, int lifespan = 1000)
    {
        bool duplicated = false;
        for(int n = 0; n < container.size(); n++)
        {
            if(container[n].Key == key && container[n].Dimension == Dimension){duplicated = true; break;}
        }

        if(!duplicated)
        {
            pair buffer;
            buffer.Key = key;
            buffer.Dimension = Dimension;
            buffer.lifespan_in_miliseconds = lifespan;
            buffer.Hitbox = Hitbox;
            container.push_back(buffer);
        }

        return !duplicated;
    }

    // Find all sprites that associate with "key" key
    pair* find(int Key, int Dimension)
    {
        for(int n = 0; n < container.size(); n++)
        {
            if(container[n].Key == Key && container[n].Dimension == Dimension)
            {
                return &container[n];     // return pointer of the associated pair (based on key)
            }
            else
            {
                // do nothing
            }
        }
        // if pair pointer cant be returned, return null pointer
        return nullptr;
    }

    // Return true if erasement succeed, because maybe there is(are) another element that can be erased. Use while control flow to delete all associated element
    bool erase(int Key, int Dimension)
    {
        bool is_successful = false;
        std::vector<pair>::iterator iterator;

        for(int n = 0; n < container.size(); n++)
        {
            if(container[n].Key == Key && container[n].Dimension == Dimension)
            {
                iterator = container.begin();
                for(int m = 0; m < n; m++)
                {
                    iterator++;
                }
                container.erase(iterator);
                is_successful = true;
                break;   // Stop iteration, one erasementis enough
            }
            else
            {
                // do nothing
            }
        }

        return is_successful;
    }

    pair* iterative_access(int n = 0)
    {
        return &container[n];
    }

    int get_size()
    {
        return container.size();
    }

    void age_cache(int time_ms)
    {
        struct int2{int x; int y;};
        std::vector<int2> dimension_key_pair_bucket;

        // reduce lifespan
        for(int n = 0; n < container.size(); n++)
        {
            container[n].lifespan_in_miliseconds -= time_ms;
            if(container[n].lifespan_in_miliseconds <= 0)
            {
                int2 buffer;
                buffer.x = container[n].Dimension;
                buffer.y = container[n].Key;
                dimension_key_pair_bucket.push_back(buffer);
            }
            else
            {
                // do nothing
            }
        }

        // Erase dying sprite (lifespan <= 0)
        for(int n = 0; n < dimension_key_pair_bucket.size(); n++)
        {
            erase(dimension_key_pair_bucket[n].y, dimension_key_pair_bucket[n].x);
        }
    }

    bool load_hitbox(int Key, int Dimension, int Lifespan = 1000, int CenterX = 100, int CenterY = 100, int Width = 100, int Height = 100, char Mode = 'C')
    {
        hitbox buffer_hitbox(CenterX, CenterY, Width, Height, Mode);
        insert(Key, Dimension, buffer_hitbox, Lifespan);
        return true;
    }
};



bool LoadTexture(std::string FileName, std::vector<SDL_Texture*> &TextureCache, SDL_Renderer* renderer, bool IsCHromaKeyed = true, int R = 255, int G = 255, int B = 255)
{
    bool IsLoaded1 = false;
    bool IsLoaded2 = false;
    SDL_Surface* canvas;
    SDL_Texture* texture;

    canvas = SDL_LoadBMP(FileName.c_str());
    if(canvas != NULL)
    {
        SDL_SetColorKey(canvas, IsCHromaKeyed, SDL_MapRGB(canvas->format, R, G, B));
        IsLoaded1 = true;
    }

    texture = SDL_CreateTextureFromSurface(renderer, canvas);
    if(texture != NULL){IsLoaded2 = true;}

    SDL_FreeSurface(canvas);

    if(IsLoaded1 && IsLoaded2)
    {
        TextureCache.push_back(texture);
        return true;
    }
    else{return false;}
}

bool LoadTextTexture(std::string Text, std::vector<SDL_Texture*> &TextureCache, SDL_Renderer* renderer, bool IsCHromaKeyed = false, int R = 255, int G = 255, int B = 255)
{
    bool IsLoaded1 = false;
    bool IsLoaded2 = false;
    SDL_Surface* canvas;
    SDL_Texture* texture;

    static TTF_Font* Font = TTF_OpenFont("arial.ttf", 50);
    static SDL_Color Color = {255, 255, 255};
    canvas = TTF_RenderUTF8_Solid(Font, Text.c_str(), Color);
    if(canvas != NULL)
    {
        SDL_SetColorKey(canvas, IsCHromaKeyed, SDL_MapRGB(canvas->format, R, G, B));
        IsLoaded1 = true;
    }

    texture = SDL_CreateTextureFromSurface(renderer, canvas);
    if(texture != NULL){IsLoaded2 = true;}

    SDL_FreeSurface(canvas);

    if(IsLoaded1 && IsLoaded2)
    {
        TextureCache.push_back(texture);
        return true;
    }
    else{return false;}

}



#define BOILERPLATE ACTIVATED
#endif