#include <engine/render/Text.h>

const std::string Text::resourceClassName = "Text";
const std::string Text::defaultResourceData = "float map(float3 position) { return 0.0; }";

void Text::load(const unsigned char *buffer, size_t size)
{
    this->contents = std::string((const char *)buffer, size);

    // assume that all texts are shader prefixes (e.g. for distance fields)
    this->prefixHash = ShaderCache::getInstance()->registerPrefix(this->contents);
}

void Text::unload()
{
    ShaderCache::getInstance()->unregisterPrefix(this->prefixHash);
    this->prefixHash = { 0, 0 };

    this->contents.clear();
}
