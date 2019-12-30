// charls_image_tester.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <charls/charls.h>

#include "portable_anymap_file.h"

#include <cassert>
#include <filesystem>
#include <iostream>
#include <vector>
#include <chrono>

using charls::interleave_mode;
using charls::jpegls_decoder;
using charls::jpegls_encoder;
using charls_test::portable_anymap_file;
using std::cout;
using std::ofstream;
using std::vector;
using std::filesystem::path;
using std::filesystem::recursive_directory_iterator;
using std::chrono::steady_clock;

namespace {

void triplet_to_planar(vector<uint8_t>& buffer, const uint32_t width, const uint32_t height)
{
    vector<uint8_t> work_buffer(buffer.size());

    const size_t byte_count = static_cast<size_t>(width) * height;
    for (size_t index = 0; index < byte_count; index++)
    {
        work_buffer[index] = buffer[index * 3 + 0];
        work_buffer[index + 1 * byte_count] = buffer[index * 3 + 1];
        work_buffer[index + 2 * byte_count] = buffer[index * 3 + 2];
    }
    swap(buffer, work_buffer);
}


portable_anymap_file read_anymap_reference_file(const char* filename, const interleave_mode interleave_mode)
{
    portable_anymap_file reference_file(filename);

    if (interleave_mode == interleave_mode::none && reference_file.component_count() == 3)
    {
        triplet_to_planar(reference_file.image_data(), reference_file.width(), reference_file.height());
    }

    return reference_file;
}


bool test_by_decoding(const vector<uint8_t>& encoded_source, const vector<uint8_t>& uncompressed_source)
{
    jpegls_decoder decoder;
    decoder.source(encoded_source);
    decoder.read_header();

    vector<uint8_t> destination(decoder.destination_size());
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
            if (destination[i] != uncompressed_source[i]) // AreEqual is very slow, pre-test to speed up 50X
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

    const char* mode;
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

    default:
        assert(false);
        mode = "";
    }

    output_filename.replace_filename(output_filename.stem().string() + mode);
    output_filename.replace_extension(".jls");

    return output_filename;
}

bool check_monochrome_file(const path& source_filename, const interleave_mode interleave_mode = interleave_mode::none)
{
    const portable_anymap_file reference_file = read_anymap_reference_file(source_filename.string().c_str(), interleave_mode);

    jpegls_encoder encoder;
    encoder.frame_info({static_cast<uint32_t>(reference_file.width()), static_cast<uint32_t>(reference_file.height()),
                        reference_file.bits_per_sample(), reference_file.component_count()})
        .interleave_mode(interleave_mode);

    vector<uint8_t> charls_encoded_data(encoder.estimated_destination_size());
    encoder.destination(charls_encoded_data);

    const auto start = steady_clock::now();
    const size_t encoded_size = encoder.encode(reference_file.image_data());
    const auto encode_duration = steady_clock::now() - start;

    charls_encoded_data.resize(encoded_size);

    ofstream output(generate_output_filename(source_filename, interleave_mode).string().c_str(), ofstream::binary);
    output.exceptions(ofstream::eofbit | ofstream::failbit | ofstream::badbit);
    output.write(reinterpret_cast<const char*>(charls_encoded_data.data()), charls_encoded_data.size());

    const double compression_ratio = static_cast<double>(reference_file.image_data().size()) / encoded_size;
    cout << "Info: original size = " << reference_file.image_data().size() << ", encoded size = " << encoded_size
         << ", compression ratio = " << std::setprecision(2) << compression_ratio << ":1"
         << ", encode time = " << std::setprecision(4) << std::chrono::duration<double, std::milli>(encode_duration).count() << " ms\n";

    return test_by_decoding(charls_encoded_data, reference_file.image_data());
}

bool check_color_file(const path& source_filename)
{
    auto result = check_monochrome_file(source_filename, interleave_mode::none);
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
        for (auto& entry : recursive_directory_iterator(argv[1]))
        {
            const bool monochrome_anymap = entry.path().extension() == ".pgm";
            const bool color_anymap = entry.path().extension() == ".ppm";

            if (monochrome_anymap || color_anymap)
            {
                cout << "Checking file: " << entry.path() << '\n';
                const bool result = monochrome_anymap ? check_monochrome_file(entry.path()) : check_color_file(entry.path());
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
