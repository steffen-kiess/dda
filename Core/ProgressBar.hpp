/*
 * Copyright (c) 2010-2012 Steffen Kieß
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef CORE_PROGRESSBAR_HPP_INCLUDED
#define CORE_PROGRESSBAR_HPP_INCLUDED

// Class which shows the progress in % (not really as a bar currently)

#include <Core/CheckedIntegerAlias.hpp>
#include <Core/OStream.hpp>

#include <string>

namespace Core {
  class ProgressBar {
    Core::OStream stream;
    double min, max;
    cuint64_t items;
    cuint64_t current;
    bool haveOutput;

  public:
    ProgressBar (const Core::OStream& stream, cuint64_t items);
    ~ProgressBar ();

    void updateDouble (double value, const std::string& str);
    void update (cuint64_t current, const std::string& str);
    void finish (const std::string& str);
    void reset (cuint64_t items, double min = 0.0, double max = 1.0);
  
    void cleanup ();
  };
}

#endif // !CORE_PROGRESSBAR_HPP_INCLUDED
