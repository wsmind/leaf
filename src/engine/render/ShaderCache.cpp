#include <engine/render/ShaderCache.h>

#include <string>
#include <fstream>
#include <streambuf>

#include <slang.h>
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

void ShaderCache::update()
{
    bool cacheDirty = this->sourceChanged.exchange(false);
    if (cacheDirty)
        this->invalidateVariants([](const VariantKey &) { return true; });
}

ShaderCache::ShaderCache(const std::string &shaderPath)
    : sourcePath(shaderPath + "/")
{
    printf("Shader source folder: %s\n", this->sourcePath.c_str());

    this->slangSession = spCreateSession(nullptr);

    // start watching the filesystem for source code changes
    this->watcherThread = new std::thread([&]()
    {
        this->watchFileChanges(this->sourcePath);
    });
}

ShaderCache::~ShaderCache()
{
    // clean all remaining variants
    this->invalidateVariants([](const VariantKey &key) { return true; });

    spDestroySession(this->slangSession);
}

ShaderCache::Hash ShaderCache::computeHash(const std::string &code) const
{
    Hash hash;
    MurmurHash3_x64_128(code.c_str(), (int)code.size(), 42, &hash);

    return hash;
}

ShaderVariant *ShaderCache::compileVariant(const std::string &shaderName, Hash prefixHash)
{
    printf("## Compiling shader '%s' (prefix: 0x%llx%llx) ##\n", shaderName.c_str(), prefixHash.first, prefixHash.second);
    SlangCompileRequest *slangRequest = spCreateCompileRequest(slangSession);

    int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_DXBC);
    spSetTargetProfile(slangRequest, targetIndex, spFindProfile(slangSession, "sm_5_0"));

    std::string shaderPath = this->sourcePath + shaderName + ".slang";
    std::ifstream shaderFile(shaderPath.c_str());
    std::string code((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());

    if (prefixHash != Hash{0, 0})
    {
        auto it = this->prefixes.find(prefixHash);
        assert(it != this->prefixes.end());

        const std::string prefixCode = it->second.code;

        code = prefixCode + "\n" + code;
    }

    std::string mainPath = this->sourcePath + "main.slang";
    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    spAddTranslationUnitSourceString(slangRequest, translationUnitIndex, mainPath.c_str(), code.c_str());

    ShaderVariant *variant = new ShaderVariant(slangRequest, translationUnitIndex);

    spDestroyCompileRequest(slangRequest);

    return variant;
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

void ShaderCache::watchFileChanges(std::string path)
{
    HANDLE handle = FindFirstChangeNotification(path.c_str(), TRUE, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME | FILE_NOTIFY_CHANGE_LAST_WRITE);

    while (true)
    {
        WaitForSingleObject(handle, INFINITE);
        this->sourceChanged = true;

        FindNextChangeNotification(handle);
    }
}
