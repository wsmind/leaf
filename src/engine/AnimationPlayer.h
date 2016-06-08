#pragma once

#include <cassert>
#include <string>
#include <vector>

class AnimationData;

class AnimationPlayer
{
    public:
        void registerAnimation(AnimationData *animation);
        void unregisterAnimation(AnimationData *animation);

        void update(float time);

        // Some animations (e.g. material animations) are not attached to a
        // particular scene. In this case, they are stepped directly through the global player.
        static AnimationPlayer globalPlayer;

    private:
        std::vector<AnimationData *> animations;
};
