#include <engine/render/ShaderCache.h>

#include <smhasher/src/MurmurHash3.h>

#include <engine/render/ShaderVariant.h>

ShaderCache *ShaderCache::instance = nullptr;

ShaderCache::Hash ShaderCache::registerPrefix(const std::string &code)
{
    Hash hash = computeHash(code);

    Prefix &prefix = this->prefixes[hash];
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
    assert(it != this->prefixes.end());

    Prefix &prefix = it->second;
    prefix.referenceCounter--;

    if (prefix.referenceCounter == 0)
    {
        // this prefix can be destroyed
        this->prefixes.erase(it);

        // invalidate cached variants using this hash
        this->invalidateVariants([prefixHash](const VariantKey &key)
        {
            return key.prefixHash == prefixHash;
        });
    }
}

const ShaderVariant *ShaderCache::getVariant(const std::string &shaderName, Hash prefixHash)
{
    VariantKey key = { shaderName, prefixHash };

    auto it = this->variants.find(key);
    if (it != this->variants.end())
    {
        // cache hit
        return it->second;
    }

    ShaderVariant *variant = this->compileVariant(shaderName, prefixHash);
    this->variants[key] = variant;

    return variant;
}

ShaderCache::ShaderCache()
{
    // look for the shaders next to the engine dll
    char leafDllPath[512];
    const std::string dllName = "LeafEngine.dll";
    HMODULE leafModule = GetModuleHandle(dllName.c_str());
    GetModuleFileName(leafModule, leafDllPath, 512);

    std::string folderPath(leafDllPath, strlen(leafDllPath) - dllName.size());
    this->sourcePath = folderPath + "shaders/";

    printf("Shader source folder: %s\n", this->sourcePath.c_str());
}

ShaderCache::~ShaderCache()
{
    // clean all remaining variants
    this->invalidateVariants([](const VariantKey &key) { return true; });
}

ShaderCache::Hash ShaderCache::computeHash(const std::string &code) const
{
    Hash hash;
    MurmurHash3_x64_128(code.c_str(), (int)code.size(), 42, &hash);

    return hash;
}

ShaderVariant *ShaderCache::compileVariant(const std::string &shaderName, Hash prefixHash)
{
    return new ShaderVariant;
}

template <class Predicate>
void ShaderCache::invalidateVariants(Predicate predicate)
{
    for (auto it = this->variants.begin(); it != this->variants.end(); )
    {
        if (predicate(it->first))
        {
            delete it->second;
            it = this->variants.erase(it);
        }
        else
        {
            it++;
        }
    }
}
