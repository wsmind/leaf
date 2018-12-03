#include <engine/render/Text.h>

const std::string Text::resourceClassName = "Text";
const std::string Text::defaultResourceData = "";

void Text::load(const unsigned char *buffer, size_t size)
{
    this->contents = std::string((const char *)buffer, size);

    // assume that all texts are shader prefixes (e.g. for distance fields)
    this->prefixHash = ShaderCache::getInstance()->registerPrefix(this->contents);
}

void Text::unload()
{
    ShaderCache::getInstance()->unregisterPrefix(this->prefixHash);

    this->contents.clear();
}
