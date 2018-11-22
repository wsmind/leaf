#pragma once

#include <string>
#include <map>
#include <cassert>
#include <cstdint>

class ShaderVariant;

class ShaderCache
{
    public:
        typedef std::pair<uint64_t, uint64_t> Hash;

        Hash registerPrefix(const std::string &code);
        void unregisterPrefix(Hash prefixHash);

        const ShaderVariant *getVariant(const std::string &shaderName, Hash prefixHash = { 0, 0 });

    private:
        static ShaderCache *instance;

        std::string repositoryPath;

        struct Prefix
        {
            std::string code;
            int referenceCounter = 0;
        };
        std::map<Hash, Prefix> prefixes;

        struct VariantKey
        {
            std::string shaderName;
            Hash prefixHash;

            bool operator <(const VariantKey &rhs) const
            {
                return (this->prefixHash < rhs.prefixHash) && (strcmp(this->shaderName.c_str(), rhs.shaderName.c_str()) < 0);
            }
        };
        std::map<VariantKey, ShaderVariant *> variants;

        ShaderCache();
        ~ShaderCache();

        Hash computeHash(const std::string &code) const;

        ShaderVariant *compileVariant(const std::string &shaderName, Hash prefixHash);

        template <class Predicate>
        void invalidateVariants(Predicate predicate);

    public:
        // singleton implementation
        static void create() { assert(!ShaderCache::instance); ShaderCache::instance = new ShaderCache; }
        static void destroy() { assert(ShaderCache::instance); delete ShaderCache::instance; }
        static ShaderCache *getInstance() { assert(ShaderCache::instance); return ShaderCache::instance; }
};