/**
 * @file BlobGen.cpp
 * @brief Blob generator - converts binary files into embedded C++ byte arrays
 *
 * Usage: dnv-vista-sdk-blobgen <input_file> <output_cpp> <namespace> [blob_name] [blob_id]
 */

#include <cctype>
#include <cstdint>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <vector>

static std::string makeIdentifier(const std::string& str)
{
    std::string result;
    result.reserve(str.length());

    for (char c : str)
    {
        result += std::isalnum(static_cast<unsigned char>(c)) ? c : '_';
    }

    if (!result.empty() && std::isdigit(static_cast<unsigned char>(result[0])))
    {
        result = "_" + result;
    }

    return result.empty() ? "blob" : result;
}

static bool isValidIdentifier(const std::string& id)
{
    if (id.empty())
    {
        return false;
    }

    if (!std::isalpha(static_cast<unsigned char>(id[0])) && id[0] != '_')
    {
        return false;
    }

    for (size_t i = 1; i < id.length(); ++i)
    {
        if (!std::isalnum(static_cast<unsigned char>(id[i])) && id[i] != '_')
        {
            return false;
        }
    }

    return true;
}

static bool isValidNamespace(const std::string& ns)
{
    if (ns.empty())
    {
        return false;
    }

    size_t pos = 0;
    while (pos < ns.length())
    {
        size_t nextColon = ns.find("::", pos);
        std::string part = (nextColon == std::string::npos) ? ns.substr(pos) : ns.substr(pos, nextColon - pos);

        if (!isValidIdentifier(part))
        {
            return false;
        }

        if (nextColon == std::string::npos)
        {
            break;
        }

        pos = nextColon + 2;
    }

    return true;
}

static std::string escapeString(const std::string& str)
{
    std::string result;
    result.reserve(str.length());

    for (char c : str)
    {
        switch (c)
        {
            case '\\':
                result += "\\\\";
                break;
            case '"':
                result += "\\\"";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                result += c;
                break;
        }
    }

    return result;
}

static bool readBinaryFile(const std::filesystem::path& path, std::vector<uint8_t>& data)
{
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input)
    {
        std::cerr << "Error: Cannot open input file: " << path << "\n";
        return false;
    }

    const std::streampos endPos = input.tellg();
    if (endPos < 0)
    {
        std::cerr << "Error: Cannot determine input file size: " << path << "\n";
        return false;
    }

    data.resize(static_cast<std::size_t>(endPos));
    input.seekg(0, std::ios::beg);
    if (!input)
    {
        std::cerr << "Error: Cannot seek in input file: " << path << "\n";
        return false;
    }

    if (!data.empty())
    {
        input.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
        if (!input)
        {
            std::cerr << "Error: Cannot read input file: " << path << "\n";
            return false;
        }
    }

    return true;
}

static void writeByteArray(std::ofstream& out, const std::string& id, const std::vector<uint8_t>& data)
{
    if (data.empty())
    {
        out << "    extern const uint8_t " << id << "_data[1] = { 0 };\n\n";
        return;
    }

    out << "    extern const uint8_t " << id << "_data[] = {\n";
    for (size_t i = 0; i < data.size(); ++i)
    {
        if (i % 16 == 0)
        {
            out << "        ";
        }

        out << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(data[i]);

        if (i + 1 < data.size())
        {
            out << ",";
        }

        if ((i + 1) % 16 == 0 || i + 1 == data.size())
        {
            out << "\n";
        }
        else
        {
            out << " ";
        }
    }
    out << "    };\n\n";
}

static bool generateCppFile(
    const std::filesystem::path& outPath,
    const std::string& identifier,
    const std::string& filename,
    const std::string& ns,
    const std::vector<uint8_t>& data)
{
    const std::filesystem::path parentDir = outPath.parent_path();
    if (!parentDir.empty())
    {
        std::error_code ec;
        std::filesystem::create_directories(parentDir, ec);
        if (ec)
        {
            std::cerr << "Error: Cannot create output directory: " << parentDir << " (" << ec.message() << ")\n";
            return false;
        }
    }

    std::ofstream out(outPath);
    if (!out)
    {
        std::cerr << "Error: Cannot create output file: " << outPath << "\n";
        return false;
    }

    out << "// Auto-generated embedded blob from: " << filename << "\n\n";
    out << "#include <cstdint>\n";
    out << "#include <cstddef>\n\n";
    out << "namespace " << ns << "\n{\n";

    writeByteArray(out, identifier, data);

    out << "    extern const size_t " << identifier << "_size = " << std::dec << data.size() << ";\n\n";
    out << "    extern const char " << identifier << "_name[] = \"" << escapeString(filename) << "\";\n";
    out << "} // namespace " << ns << "\n";

    return true;
}

int main(int argc, char* argv[])
{
    if (argc < 4 || argc > 6)
    {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_cpp> <namespace> [blob_name] [blob_id]\n";
        return 1;
    }

    const std::string inputFile = argv[1];
    const std::string outputCpp = argv[2];
    const std::string ns = argv[3];

    if (!isValidNamespace(ns))
    {
        std::cerr << "Error: Invalid C++ namespace: " << ns << "\n";
        return 1;
    }

    const std::filesystem::path inPath = std::filesystem::path{ inputFile }.lexically_normal();
    const std::filesystem::path outPath = std::filesystem::path{ outputCpp }.lexically_normal();

    const std::string blobName = (argc >= 5) ? argv[4] : inPath.filename().string();
    const std::string identifier = (argc >= 6) ? argv[5] : makeIdentifier(blobName);

    if (!isValidIdentifier(identifier))
    {
        std::cerr << "Error: Could not generate valid C++ identifier from: " << blobName << "\n";
        return 1;
    }

    std::vector<uint8_t> data;
    if (!readBinaryFile(inPath, data))
    {
        return 1;
    }

    if (!generateCppFile(outPath, identifier, blobName, ns, data))
    {
        return 1;
    }

    std::cout << "Generated: " << outPath << " (" << data.size() << " bytes)\n";
    return 0;
}
