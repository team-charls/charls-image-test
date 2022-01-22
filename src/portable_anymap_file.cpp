// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#ifdef __cpp_modules

module portable_anymap_file;

import <string>;
import <sstream>;
import <fstream>;
import <utility>;

#else

#include "portable_anymap_file.h"

#include <string>
#include <sstream>
#include <fstream>
#include <utility>
#include <vector>

#endif

using std::vector;
using std::istream;
using std::ifstream;

namespace {

vector<int> read_header(istream& pnm_file)
{
    vector<int> result;

    const auto first{static_cast<char>(pnm_file.get())};

    // All portable anymap format (PNM) start with the character P.
    if (first != 'P')
        throw istream::failure("Missing P");

    while (result.size() < 4)
    {
        std::string bytes;
        std::getline(pnm_file, bytes);
        std::stringstream line(bytes);

        while (result.size() < 4)
        {
            int value{-1};
            line >> value;
            if (value <= 0)
                break;

            result.push_back(value);
        }
    }
    return result;
}

constexpr int32_t log_2(const int32_t n) noexcept
{
    int32_t x{};
    while (n > (1 << x))
    {
        ++x;
    }
    return x;
}

void convert_to_little_endian_if_needed(const int32_t bits_per_sample, vector<std::byte>& input_buffer) noexcept
{
    // Anymap files with multi byte pixels are stored in big endian format in the file.
    if (bits_per_sample > 8)
    {
        for (size_t i{}; i < input_buffer.size() - 1; i += 2)
        {
            std::swap(input_buffer[i], input_buffer[i + 1]);
        }
    }
}

} // namespace

portable_anymap_file::portable_anymap_file(const char* filename)
{
    ifstream pnm_file;
    pnm_file.exceptions(ifstream::eofbit | ifstream::failbit | ifstream::badbit);
    pnm_file.open(filename, ifstream::in | ifstream::binary);

    vector<int> header_info{read_header(pnm_file)};
    if (header_info.size() != 4)
        throw ifstream::failure("Incorrect PNM header");

    component_count_ = header_info[0] == 6 ? 3 : 1;
    width_ = static_cast<uint32_t>(header_info[1]);
    height_ = static_cast<uint32_t>(header_info[2]);
    bits_per_sample_ = log_2(header_info[3] + 1);

    const int bytes_per_sample{(bits_per_sample_ + 7) / 8};
    input_buffer_.resize(static_cast<size_t>(width_) * height_ * bytes_per_sample * component_count_);
    pnm_file.read(reinterpret_cast<char*>(input_buffer_.data()), static_cast<std::streamsize>(input_buffer_.size()));

    convert_to_little_endian_if_needed(bits_per_sample_, input_buffer_);
}
