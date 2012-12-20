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

#ifndef CORE_BOOSTFILESYSTEM_HPP_INCLUDED
#define CORE_BOOSTFILESYSTEM_HPP_INCLUDED

// Hack to allow writing code which works both with Boost.Filesystem v2 and v3
//
// Use path.BOOST_FILE_STRING or path.BOOST_FILENAME_STRING

#include <boost/filesystem/path.hpp>

#if !defined (BOOST_FILESYSTEM_VERSION) || BOOST_FILESYSTEM_VERSION == 2
#define BOOST_FILE_STRING file_string ()
#define BOOST_FILENAME_STRING filename ()
#elif defined (BOOST_FILESYSTEM_VERSION) && BOOST_FILESYSTEM_VERSION == 3
#define BOOST_FILE_STRING string ()
#define BOOST_FILENAME_STRING filename ().string ()
#else
#error Unknown BOOST_FILESYSTEM_VERSION
#endif

#endif // !CORE_BOOSTFILESYSTEM_HPP_INCLUDED
