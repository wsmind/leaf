#include <engine/render/ShaderCache.h>

#include <smhasher/src/MurmurHash3.h>

ShaderCache *ShaderCache::instance = nullptr;

ShaderCache::Hash ShaderCache::registerPrefix(const std::string &code)
{
    Hash hash = computeHash(code);

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

const ShaderVariant *ShaderCache::getVariant(const std::string &shaderName, Hash prefixHash)
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

ShaderCache::Hash ShaderCache::computeHash(const std::string &code) const
{
    Hash hash;
    MurmurHash3_x64_128(code.c_str(), (int)code.size(), 42, &hash);

    return hash;
}
