#include <engine/render/ShaderCache.h>

ShaderCache *ShaderCache::instance = nullptr;

ShaderCache::Hash ShaderCache::registerPrefix(const std::string &code)
{
    Hash hash = computePrefixHash(code);

    Prefix &prefix = this->prefixes.find(hash)->second;
    prefix.referenceCounter++;

    if (prefix.referenceCounter == 1)
    {
        // first time initialization
        prefix.code = code;
    }

    return hash;
}

void ShaderCache::unregisterPrefix(Hash prefixHash)
{
    auto it = this->prefixes.find(prefixHash);

    Prefix &prefix = it->second;
    prefix.referenceCounter--;

    if (prefix.referenceCounter == 0)
    {
        // this prefix can be destroyed
        this->prefixes.erase(it);
    }
}

const ShaderVariant *ShaderCache::getVariant(const std::string &shaderName, unsigned long prefixHash)
{
    return nullptr;
}

ShaderCache::ShaderCache()
{
    this->repositoryPath = "shaders/";
}

ShaderCache::~ShaderCache()
{
}

ShaderCache::Hash ShaderCache::computePrefixHash(const std::string &code) const
{
    return 0;
}
