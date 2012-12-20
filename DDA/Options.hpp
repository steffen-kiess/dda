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

#ifndef DDA_OPTIONS_HPP_INCLUDED
#define DDA_OPTIONS_HPP_INCLUDED

// Command line parameter parsing code

#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/optional.hpp>
#include <boost/scoped_ptr.hpp>

namespace DDA {
  class Options {
  public:
    typedef boost::program_options::positional_options_description PositionalDescription;
    typedef boost::program_options::options_description Description;
    typedef boost::program_options::variables_map Map;
    typedef boost::program_options::command_line_style::style_t Style;

    static const Style style = static_cast<Style> (boost::program_options::command_line_style::unix_style & ~boost::program_options::command_line_style::allow_guessing);

  private:
    PositionalDescription positional_;
    Description description_;
    boost::scoped_ptr<Description> descriptionAll_;

  public:
    Options ();
    ~Options ();

    const PositionalDescription& positional () const { return positional_; }
    const Description& description () const { return description_; }
    const Description& descriptionAll () const { return *descriptionAll_; }

    boost::optional<Map> parse (int argc, const char* const* argv) const;

    void help () const;

    std::vector<std::string> getParameters (const Map& map) const;
    std::string getParameterString (const Map& map) const;
  };
}

#endif // !DDA_OPTIONS_HPP_INCLUDED
