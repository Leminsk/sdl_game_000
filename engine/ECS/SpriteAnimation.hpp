#pragma once

struct SpriteAnimation
{
    /*
    index: used to select a sprite row from the sprite sheet
    frames: total number of sprites to be drawn from the sprite sheet row
    delay: amount of time in milliseconds between each sprite drawn
    */
    int index, frames, delay;

    SpriteAnimation() {}
    SpriteAnimation(int i, int f, int d) {
        this->index = i;
        this->frames = f;
        this->delay = d;
    }
};
