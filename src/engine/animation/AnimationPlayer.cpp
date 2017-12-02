#include <engine/animation/AnimationPlayer.h>

#include <engine/animation/AnimationData.h>

AnimationPlayer AnimationPlayer::globalPlayer;

void AnimationPlayer::registerAnimation(AnimationData *animation)
{
    assert(std::find(this->animations.begin(), this->animations.end(), animation) == this->animations.end());

    this->animations.push_back(animation);
}

void AnimationPlayer::unregisterAnimation(AnimationData *animation)
{
    auto it = std::find(this->animations.begin(), this->animations.end(), animation);
    assert(it != this->animations.end());

    // because order is irrelevant, we can just replace the
    // element to remove with the last element
    *it = this->animations.back();
    this->animations.pop_back();
}

void AnimationPlayer::update(float time)
{
    for (auto animation: this->animations)
    {
        animation->update(time);
    }
}
