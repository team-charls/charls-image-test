// charls_image_tester.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "charls/include/charls/charls.h"

#include "portable_anymap_file.h"

#include <iostream>
#include <filesystem>
#include <vector>

using std::cout;
using std::filesystem::path;
using std::filesystem::recursive_directory_iterator;
using std::vector;
using std::ofstream;
using std::tuple;
using charls::jpegls_encoder;
using charls::jpegls_decoder;
using charls::interleave_mode;
using charls_test::portable_anymap_file;

namespace {

portable_anymap_file read_anymap_reference_file(const char* filename, const interleave_mode interleave_mode)
{
    portable_anymap_file reference_file(filename);

    if (interleave_mode == interleave_mode::none && reference_file.component_count() == 3)
    {
        //triplet_to_planar(reference_file.image_data(), reference_file.width(), reference_file.height());
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

tuple<bool, size_t, size_t> check_monochrome_file(path source_filename)
{
    const portable_anymap_file reference_file = read_anymap_reference_file(source_filename.string().c_str(), interleave_mode::none);

    jpegls_encoder encoder;
    encoder.frame_info({
        static_cast<uint32_t>(reference_file.width()), static_cast<uint32_t>(reference_file.height()),
        reference_file.bits_per_sample(), reference_file.component_count() });

    vector<uint8_t> charls_encoded_data(encoder.estimated_destination_size());
    encoder.destination(charls_encoded_data);

    const size_t encoded_size = encoder.encode(reference_file.image_data());
    charls_encoded_data.resize(encoded_size);

    source_filename.replace_extension(".jls");
    ofstream output(source_filename.string().c_str(), ofstream::binary);
    output.exceptions(ofstream::eofbit | ofstream::failbit | ofstream::badbit);
    output.write(reinterpret_cast<const char*>(charls_encoded_data.data()), charls_encoded_data.size());

    return {test_by_decoding(charls_encoded_data,reference_file.image_data()), reference_file.image_data().size(), encoded_size};
}

}

int main(const int argc, const char* const argv[])
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
            if (entry.path().extension() == ".pgm")
            {
                cout << "Checking file: " << entry.path() << '\n';
                auto [result, original_size, encoded_size] = check_monochrome_file(entry.path());
                if (result)
                {
                    cout << "Passed, original size = " << original_size << ", encoded size = " << encoded_size << '\n';
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
