#pragma once

#include <string>
#include <map>
#include <cassert>
#include <cstdint>

class ShaderVariant;

class ShaderCache
{
    public:
        typedef uint64_t Hash;

        Hash registerPrefix(const std::string &code);
        void unregisterPrefix(Hash prefixHash);

        const ShaderVariant *getVariant(const std::string &shaderName, Hash prefixHash = 0);

    private:
        static ShaderCache *instance;

        std::string repositoryPath;

        struct Prefix
        {
            std::string code;
            int referenceCounter = 0;
        };
        std::map<Hash, Prefix> prefixes;

        ShaderCache();
        ~ShaderCache();

        Hash computeHash(const std::string &code) const;

    public:
        // singleton implementation
        static void create() { assert(!ShaderCache::instance); ShaderCache::instance = new ShaderCache; }
        static void destroy() { assert(ShaderCache::instance); delete ShaderCache::instance; }
        static ShaderCache *getInstance() { assert(ShaderCache::instance); return ShaderCache::instance; }
};