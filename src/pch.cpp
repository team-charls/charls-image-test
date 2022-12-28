// Copyright (c) Team CharLS.
// SPDX-License-Identifier: MIT

// The purpose of this file is to build the header units. It is marked with ScanSourceForModuleDependencies = true in the vcxproj file.

#if defined __cpp_modules && !defined __SANITIZE_ADDRESS__

import <chrono>;
import <filesystem>;
import <fstream>;
import <iostream>;
import <sstream>;
import <string>;
import <vector>;
import <utility>;
import <cassert>;
import <format>;
import <span>;
import <string_view>;
import <cstddef>;

import <charls/charls.h>;

#endif
