#if __has_include("zlib.h")
#include "D_complession.h"
#include <stdexcept>
#include <vector>
#include <zlib.h>
namespace fasm::data
{
    std::string complession(const std::string &data)
    {
        if (data.empty())
            return {};

        z_stream zs{};
        zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
        zs.avail_in = static_cast<uInt>(data.size());

        // raw DEFLATE : windowBits = -15
        if (deflateInit2(&zs,
                         Z_BEST_COMPRESSION,
                         Z_DEFLATED,
                         -15,
                         8,
                         Z_DEFAULT_STRATEGY) != Z_OK)
        {
            throw std::runtime_error("complession: deflateInit2 failed");
        }

        std::string out;
        out.resize(deflateBound(&zs, zs.avail_in));

        zs.next_out = reinterpret_cast<Bytef *>(&out[0]);
        zs.avail_out = static_cast<uInt>(out.size());

        int ret = deflate(&zs, Z_FINISH);
        if (ret != Z_STREAM_END)
        {
            deflateEnd(&zs);
            throw std::runtime_error("complession: deflate failed");
        }

        out.resize(zs.total_out);
        deflateEnd(&zs);
        return out;
    }

    std::string decomplession(const std::string &data)
    {
        if (data.empty())
            return {};

        z_stream zs{};
        zs.next_in = reinterpret_cast<Bytef *>(const_cast<char *>(data.data()));
        zs.avail_in = static_cast<uInt>(data.size());

        // raw DEFLATE : windowBits = -15
        if (inflateInit2(&zs, -15) != Z_OK)
        {
            throw std::runtime_error("decomplession: inflateInit2 failed");
        }

        std::string out;
        const size_t chunk_size = 64 * 1024;
        char buffer[chunk_size];

        int ret;
        do
        {
            zs.next_out = reinterpret_cast<Bytef *>(buffer);
            zs.avail_out = chunk_size;

            ret = inflate(&zs, Z_NO_FLUSH);

            if (ret != Z_OK && ret != Z_STREAM_END)
            {
                inflateEnd(&zs);
                throw std::runtime_error("decomplession: inflate failed");
            }

            out.append(buffer, chunk_size - zs.avail_out);

        } while (ret != Z_STREAM_END);

        inflateEnd(&zs);
        return out;
    }
}
#endif
