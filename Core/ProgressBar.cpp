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

#include "ProgressBar.hpp"

#include <sstream>
#include <iomanip>

namespace Core {
  ProgressBar::ProgressBar (const Core::OStream& stream, cuint64_t items) : stream (stream), items (0), current (0) {
    haveOutput = false;
    reset (items);
  }
  ProgressBar::~ProgressBar () {
  }

  void ProgressBar::updateDouble (double value, const std::string& str) {
    value = min + (max - min) * value;
    std::stringstream s;
    s << "[" << std::fixed << std::setprecision (2) << std::setw (6) << (value * 100.0) << "%] " << str;
    std::string s2 = s.str ();
    if (s2.length () > 79)
      s2 = s2.substr (0, 79 - 3) + "...";
    while (s2.length () < 79)
      s2 += " ";
    stream << s2 << "\r" << std::flush;
    //stream << s2 << std::endl;
    haveOutput = true;
  }

  void ProgressBar::update (cuint64_t current, const std::string& str) {
    this->current = current;
    updateDouble (static_cast<double> (current ()) / static_cast<double> (items ()), str);
  }

  void ProgressBar::finish (const std::string& str) {
    update (items, str);
  }

  void ProgressBar::reset (cuint64_t items, double min, double max) {
    this->items = items;
    this->min = min;
    this->max = max;
  }
  
  void ProgressBar::cleanup () {
    if (haveOutput) {
      stream << std::endl;
      haveOutput = false;
    }
  }
}
