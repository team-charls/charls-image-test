// Copyright (c) Team CharLS.
// SPDX-License-Identifier: BSD-3-Clause

#if defined __cpp_modules && !defined __SANITIZE_ADDRESS__

export module portable_anymap_file;

import <cstddef>;
import <vector>;
import <string_view>;

export
{
#include "portable_anymap_file.h"
}

#endif
