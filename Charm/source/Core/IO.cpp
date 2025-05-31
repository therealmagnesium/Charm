#include "Core/IO.h"
#include "Core/Log.h"
#include <fstream>

namespace Charm
{
    namespace Core
    {
        namespace IO
        {
            File ReadFile(const char* path)
            {
                File file;
                std::ifstream stream(path);
                if (!stream.is_open())
                {
                    WARN("Failed to read file \"%s\"", path);
                    return file;
                }

                file.path = path;

                stream.seekg(0, std::ios::end);
                file.size = stream.tellg();
                stream.seekg(0, std::ios::beg);

                file.data.resize(file.size + 1);
                stream.read(file.data.data(), file.size);
                file.data[file.size] = '\0';

                file.isValid = stream.gcount() == file.size;
                stream.close();

                return file;
            }

        }
    }
}
