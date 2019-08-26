#include "stdafx.h"

// NOTE: This is NOT the original v8pp source!
// Some modifications have been made to fit the nekoengine project.

//
// Copyright (c) 2013-2016 Pavel Medvedev. All rights reserved.
//
// This file is part of v8pp (https://github.com/pmed/v8pp) project.
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "v8pp/config.hpp"
#include "v8pp/convert.hpp"

namespace v8pp {

  template struct convert<std::string>;
  template struct convert<char const*>;
#ifdef _WIN32
  template struct convert<std::wstring>;
  template struct convert<wchar_t const*>;
#endif
  template struct convert<bool>;

  template struct convert<char>;
  template struct convert<signed char>;
  template struct convert<unsigned char>;

  template struct convert<short>;
  template struct convert<unsigned short>;

  template struct convert<int>;
  template struct convert<unsigned int>;

  template struct convert<long>;
  template struct convert<unsigned long>;

  template struct convert<long long>;
  template struct convert<unsigned long long>;

  template struct convert<float>;
  template struct convert<double>;
  template struct convert<long double>;

}