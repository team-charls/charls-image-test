// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

#include "pch.h"

#include "portable_anymap_file.h"

#include <charls/charls.h>

#include <cassert>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <vector>

using charls::interleave_mode;
using charls::jpegls_decoder;
using charls::jpegls_encoder;
using charls::test::portable_anymap_file;
using std::byte;
using std::cout;
using std::ofstream;
using std::vector;
using std::chrono::steady_clock;
using std::filesystem::path;

namespace {

void triplet_to_planar(vector<byte>& buffer, const size_t width, const size_t height, const int32_t bits_per_sample)
{
    vector<byte> work_buffer(buffer.size());

    const size_t samples_per_plane{width * height};

    if (bits_per_sample > 8)
    {
        const auto* source_buffer16{reinterpret_cast<const uint16_t*>(buffer.data())};
        auto* buffer16{reinterpret_cast<uint16_t*>(work_buffer.data())};
        for (size_t i{}; i < samples_per_plane; ++i)
        {
            buffer16[i] = source_buffer16[i * 3 + 0];
            buffer16[i + 1 * samples_per_plane] = source_buffer16[i * 3 + 1];
            buffer16[i + 2 * samples_per_plane] = source_buffer16[i * 3 + 2];
        }
    }
    else
    {
        for (size_t i{}; i < samples_per_plane; ++i)
        {
            work_buffer[i] = buffer[i * 3 + 0];
            work_buffer[i + 1 * samples_per_plane] = buffer[i * 3 + 1];
            work_buffer[i + 2 * samples_per_plane] = buffer[i * 3 + 2];
        }
    }

    swap(buffer, work_buffer);
}


portable_anymap_file read_anymap_reference_file(const char* filename, const interleave_mode interleave_mode)
{
    portable_anymap_file reference_file(filename);

    if (interleave_mode == interleave_mode::none && reference_file.component_count() == 3)
    {
        triplet_to_planar(reference_file.image_data(), reference_file.width(), reference_file.height(), reference_file.bits_per_sample());
    }

    return reference_file;
}


bool test_by_decoding(const vector<byte>& encoded_source, const vector<byte>& uncompressed_source)
{
    jpegls_decoder decoder;
    decoder.source(encoded_source);
    decoder.read_header();

    vector<byte> destination(decoder.destination_size());
    decoder.decode(destination);

    if (destination.size() != uncompressed_source.size())
    {
        cout << "Pixel data size doesn't match";
        return false;
    }

    if (decoder.near_lossless() == 0)
    {
        for (size_t i = 0; i < uncompressed_source.size(); ++i)
        {
            if (destination[i] != uncompressed_source[i])
            {
                cout << "Pixel data value doesn't match";
                return false;
            }
        }
    }

    return true;
}

path generate_output_filename(const path& source_filename, const interleave_mode interleave_mode)
{
    path output_filename{source_filename};

    const char* mode{""};
    switch (interleave_mode)
    {
    case interleave_mode::none:
        mode = "-none";
        break;

    case interleave_mode::line:
        mode = "-line";
        break;

    case interleave_mode::sample:
        mode = "-sample";
        break;
    }
    assert(strlen(mode));

    output_filename.replace_filename(output_filename.stem().string() + mode);
    output_filename.replace_extension(".jls");

    return output_filename;
}

const char* interleave_mode_to_string(const interleave_mode interleave_mode) noexcept
{
    switch (interleave_mode)
    {
    case interleave_mode::none:
        return "none";

    case interleave_mode::line:
        return "line";

    case interleave_mode::sample:
        return "sample";
    }

    assert(false);
    return "";
}

bool check_monochrome_file(const path& source_filename, const interleave_mode interleave_mode = interleave_mode::none)
{
    const portable_anymap_file reference_file{read_anymap_reference_file(source_filename.string().c_str(), interleave_mode)};

    jpegls_encoder encoder;
    encoder.frame_info({reference_file.width(), reference_file.height(),
                        reference_file.bits_per_sample(), reference_file.component_count()})
        .interleave_mode(interleave_mode);

    vector<byte> charls_encoded_data(encoder.estimated_destination_size());
    encoder.destination(charls_encoded_data);

    const auto start{steady_clock::now()};
    const size_t encoded_size{encoder.encode(reference_file.image_data())};
    const auto encode_duration{steady_clock::now() - start};

    charls_encoded_data.resize(encoded_size);

    ofstream output(generate_output_filename(source_filename, interleave_mode).string().c_str(), ofstream::binary | ofstream::trunc);
    output.exceptions(ofstream::eofbit | ofstream::failbit | ofstream::badbit);
    output.write(reinterpret_cast<const char*>(charls_encoded_data.data()), static_cast<std::streamsize>(charls_encoded_data.size()));

    const double compression_ratio = static_cast<double>(reference_file.image_data().size()) / static_cast<double>(encoded_size);
    cout << "Info: original size = " << reference_file.image_data().size() << ", encoded size = " << encoded_size
         << ", interleave mode = " << interleave_mode_to_string(interleave_mode)
         << ", compression ratio = " << std::setprecision(2) << std::fixed << std::showpoint << compression_ratio << ":1"
         << ", encode time = " << std::setprecision(4) << std::chrono::duration<double, std::milli>(encode_duration).count() << " ms\n";

    return test_by_decoding(charls_encoded_data, reference_file.image_data());
}

bool check_color_file(const path& source_filename)
{
    auto result{check_monochrome_file(source_filename, interleave_mode::none)};
    if (!result)
        return result;

    result = check_monochrome_file(source_filename, interleave_mode::line);
    if (!result)
        return result;

    return check_monochrome_file(source_filename, interleave_mode::sample);
}

} // namespace

int main(const int argc, const char* const argv[]) // NOLINT(bugprone-exception-escape)
{
    if (argc < 2)
    {
        cout << "usage: charls_image_tester <directory-to-test>\n";
        return EXIT_FAILURE;
    }

    try
    {
        for (const auto& entry : std::filesystem::recursive_directory_iterator(argv[1]))
        {
            const bool monochrome_anymap{entry.path().extension() == ".pgm"};
            const bool color_anymap{entry.path().extension() == ".ppm"};

            if (monochrome_anymap || color_anymap)
            {
                cout << "Checking file: " << entry.path() << '\n';
                const bool result{monochrome_anymap ? check_monochrome_file(entry.path()) : check_color_file(entry.path())};
                if (result)
                {
                    cout << "Passed\n";
                }
                else
                {
                    cout << "Failed\n";
                    return EXIT_FAILURE;
                }
            }
        }

        return EXIT_SUCCESS;
    }
    catch (const std::exception& error)
    {
        cout << "Unexpected failure: " << error.what();
    }
}
