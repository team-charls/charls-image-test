# charls-image-test

Simple C++ console application that can verify the CharLS JPEG-LS codec by using input
from a directory filled with AnyMap images (.pgm, .ppm) by encoding them to JPEG-LS and back.

This test application is not part of the main CharLS repository as it uses C\++20
and CharLS is fixed on C++17 until the next major release.

It can be build with MSVC++ with the .vcxproj file or with CMake for GCC, Clang and MSVC(C++17 mode).

Note 1: the AnyMap images are not part of this repository. These images can be downloaded from:
<http://imagecompression.info/test_images/>

Note 2: use git clone --recurse-submodules to ensure that the CharLS sub module is also cloned.

Note 3: Visual Studio 2022 17.6.0 Preview 7.0 or newer is needed to build for Windows.
