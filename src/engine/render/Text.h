#pragma once

#include <string>

#include <engine/render/ShaderCache.h>
#include <engine/resource/Resource.h>

class Text: public Resource
{
    public:
        static const std::string resourceClassName;
        static const std::string defaultResourceData;

        virtual void load(const unsigned char *buffer, size_t size) override;
        virtual void unload() override;

        ShaderCache::Hash getPrefixHash() const { return this->prefixHash; }

    private:
        std::string contents;
        
        // assume all texts are shader prefixes
        ShaderCache::Hash prefixHash;
};
