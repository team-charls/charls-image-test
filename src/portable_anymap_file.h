#pragma once

#if !defined __cpp_modules || defined __SANITIZE_ADDRESS__

#include <cstdint>
#include <string_view>
#include <vector>

#endif

/// <summary>
/// This class can read an image stored in the Portable Anymap Format (PNM).
/// The 2 binary formats P5 and P6 are supported:
/// <para>Portable GrayMap: P5 = binary, extension = .pgm, 0-2^16 (gray scale)</para>
/// <para>Portable PixMap: P6 = binary, extension = ppm, range 0-2^16 (RGB)</para>
/// </summary>
/// <remarks>This class is designed for test code, robust error handling and input validation is not done.
/// </remarks>
class portable_anymap_file final
{
public:
    /// <exception cref="ifstream::failure">Thrown when the input file cannot be read.</exception>
    explicit portable_anymap_file(std::string_view filename);

    [[nodiscard]]
    std::uint32_t width() const noexcept
    {
        return width_;
    }

    [[nodiscard]]
    std::uint32_t height() const noexcept
    {
        return height_;
    }

    [[nodiscard]]
    std::int32_t component_count() const noexcept
    {
        return component_count_;
    }

    [[nodiscard]]
    std::int32_t bits_per_sample() const noexcept
    {
        return bits_per_sample_;
    }

    [[nodiscard]]
    std::vector<std::byte>& image_data() noexcept
    {
        return input_buffer_;
    }

    [[nodiscard]]
    const std::vector<std::byte>& image_data() const noexcept
    {
        return input_buffer_;
    }

private:
    std::int32_t component_count_;
    std::uint32_t width_;
    std::uint32_t height_;
    std::int32_t bits_per_sample_;
    std::vector<std::byte> input_buffer_;
};
