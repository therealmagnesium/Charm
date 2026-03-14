#pragma once
#include "Core/Base.h"
#include "Graphics/Shapes.h"
#include <string>
#include <vector>
#include <yaml-cpp/yaml.h>
#include <glm/glm.hpp>

namespace Charm::Core
{
    struct File
    {
        bool isValid = false;
        u64 size = 0;
        std::string path;
        std::vector<char> data;

        inline std::string asString() const
        {
            return (isValid) ? std::string(data.begin(), data.end()) : "";
        }

        inline const char* asCString() const
        {
            return (isValid) ? data.data() : "";
        }
    };

    namespace IO
    {
        File ReadFile(const char* path);
    }
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
    return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
    return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
    return out;
}

inline YAML::Emitter& operator<<(YAML::Emitter& out, const Charm::Graphics::Rectangle& r)
{
    out << YAML::Flow;
    out << YAML::BeginSeq << r.x << r.y << r.width << r.height << YAML::EndSeq;
    return out;
}

namespace YAML
{
    template <>
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2& v)
        {
            Node node;
            node.push_back(v.x);
            node.push_back(v.y);

            return node;
        }

        static bool decode(const Node& node, glm::vec2& v)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            v.x = node[0].as<float>();
            v.y = node[1].as<float>();

            return true;
        }
    };

    template <>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& v)
        {
            Node node;
            node.push_back(v.x);
            node.push_back(v.y);
            node.push_back(v.z);

            return node;
        }

        static bool decode(const Node& node, glm::vec3& v)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            v.x = node[0].as<float>();
            v.y = node[1].as<float>();
            v.z = node[2].as<float>();

            return true;
        }
    };

    template <>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& v)
        {
            Node node;
            node.push_back(v.r);
            node.push_back(v.g);
            node.push_back(v.b);
            node.push_back(v.a);

            return node;
        }

        static bool decode(const Node& node, glm::vec4& v)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            v.r = node[0].as<float>();
            v.g = node[1].as<float>();
            v.b = node[2].as<float>();
            v.a = node[3].as<float>();

            return true;
        }
    };

    template <>
    struct convert<Charm::Graphics::Rectangle>
    {
        static Node encode(const Charm::Graphics::Rectangle& r)
        {
            Node node;
            node.push_back(r.x);
            node.push_back(r.y);
            node.push_back(r.width);
            node.push_back(r.height);

            return node;
        }

        static bool decode(const Node& node, Charm::Graphics::Rectangle& r)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            r.x = node[0].as<float>();
            r.y = node[1].as<float>();
            r.width = node[2].as<float>();
            r.height = node[3].as<float>();

            return true;
        }
    };
}
