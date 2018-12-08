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

    ShaderVariant *variant = this->compileVariant(key);
    this->variants[key] = variant;

    return variant;
}

int ShaderCache::exportVariants(const std::string &exportPath, const std::vector<VariantKey> &keys)
{
    std::string filename = exportPath + "/shaders.bin";

    FILE *f = fopen(filename.c_str(), "wb");
    for (const auto &key : keys)
    {
        printf("Exporting shader variant '%s' (prefix: 0x%llx%llx)\n", key.shaderName.c_str(), key.prefixHash.first, key.prefixHash.second);

        uint32_t size = (uint32_t)key.shaderName.size();
        fwrite(&size, sizeof(uint32_t), 1, f);
        fwrite(key.shaderName.c_str(), size, 1, f);
        fwrite(&key.prefixHash, sizeof(key.prefixHash), 1, f);

        ShaderVariant *variant = this->compileVariant(key, f);
        delete variant;
    }
    fclose(f);

    return 0;
}

void ShaderCache::update()
{
    bool cacheDirty = this->sourceChanged.exchange(false);
    if (cacheDirty)
        this->invalidateVariants([](const VariantKey &) { return true; });
}

ShaderCache::ShaderCache(const std::string &shaderPath)
    : sourcePath(shaderPath + "/")
    , sourceChanged(false)
{
    printf("Shader source folder: %s\n", this->sourcePath.c_str());

    if (this->loadVariantCache(this->sourcePath + "../shaders.bin"))
        return;

    // when a precompiled cache is loaded, everything else is disabled
    // (the goal is to catch missing shaders before shipping the demo)

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

    if (this->slangSession != nullptr)
        spDestroySession(this->slangSession);
}

ShaderCache::Hash ShaderCache::computeHash(const std::string &code) const
{
    Hash hash;
    MurmurHash3_x64_128(code.c_str(), (int)code.size(), 42, &hash);

    return hash;
}

ShaderVariant *ShaderCache::compileVariant(const VariantKey &key, FILE *exportStream)
{
    assert(this->slangSession != nullptr);

    printf("## Compiling shader '%s' (prefix: 0x%llx%llx) ##\n", key.shaderName.c_str(), key.prefixHash.first, key.prefixHash.second);
    SlangCompileRequest *slangRequest = spCreateCompileRequest(slangSession);

    int targetIndex = spAddCodeGenTarget(slangRequest, SLANG_DXBC);
    spSetTargetProfile(slangRequest, targetIndex, spFindProfile(slangSession, "sm_5_0"));

    std::string shaderPath = this->sourcePath + key.shaderName + ".slang";
    std::ifstream shaderFile(shaderPath.c_str());
    std::string code((std::istreambuf_iterator<char>(shaderFile)), std::istreambuf_iterator<char>());

    if (key.prefixHash != Hash{0, 0})
    {
        auto it = this->prefixes.find(key.prefixHash);
        assert(it != this->prefixes.end());

        const std::string prefixCode = it->second.code;

        code = prefixCode + "\n" + code;
    }

    std::string mainPath = this->sourcePath + "main.slang";
    int translationUnitIndex = spAddTranslationUnit(slangRequest, SLANG_SOURCE_LANGUAGE_SLANG, nullptr);
    spAddTranslationUnitSourceString(slangRequest, translationUnitIndex, mainPath.c_str(), code.c_str());

    ShaderVariant *variant = new ShaderVariant();
    variant->compileShaders(slangRequest, translationUnitIndex, exportStream);

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

bool ShaderCache::loadVariantCache(std::string path)
{
    FILE *f = fopen(path.c_str(), "rb");
    if (!f)
        return false;

    while (true)
    {
        // read the key
        uint32_t size;
        fread(&size, sizeof(uint32_t), 1, f);

        // EOF will be triggered after the first read failure
        if (feof(f))
            break;

        std::string shaderName((size_t)size, 0);
        fread((void *)shaderName.c_str(), size, 1, f);

        Hash hash;
        fread(&hash, sizeof(hash), 1, f);

        VariantKey key{ shaderName, hash };

        printf("## Loading shader '%s' (prefix: 0x%llx%llx) ##\n", key.shaderName.c_str(), key.prefixHash.first, key.prefixHash.second);

        // load the precompiled variant directly from file
        ShaderVariant *variant = new ShaderVariant;
        variant->loadShaders(f);

        // populate the cache for further queries
        this->variants[key] = variant;
    }

    fclose(f);

    return true;
}
