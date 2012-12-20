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

// Process all include directives for a source file, ignoring all other
// directives.
// Includes inside recursive includes are replaced by #error directives
// (recursive includes should be prevented by include guards).
//
// The generated file should be stand-alone and can be compiled without needing
// any header files.

#include <Core/Assert.hpp>
#include <Core/Util.hpp>
#include <Core/BoostFilesystem.hpp>
#include <Core/StringUtil.hpp>
#include <Core/IStream.hpp>
#include <Core/OStream.hpp>

#include <OpenCL/CLangUtil.hpp>

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

#include <vector>
#include <set>
#include <string>
#include <cstdio>

#include <cstdlib>

static boost::filesystem::path canonicalize (const boost::filesystem::path& path) {
  boost::filesystem::path result;

  BOOST_FOREACH (const boost::filesystem::path::iterator::value_type& s, path) {
    if (s == ".")
      ;
    else if (s == ".." && !result.empty () && result.filename () != "/" && result.filename () != "..")
      result = result.parent_path ();
    else
      result /= s;
  }

  if (result.empty ())
    result = ".";

  return result;
}

typedef boost::filesystem::path path;

static void process (const std::vector<path>& includePaths, const path& file, std::vector<path>& stack, std::set<path>& stacks, const Core::OStream& out) {
  path file_canon = canonicalize (file);
  Core::IStream stream = Core::IStream::open (file_canon);
  path path_canon = file_canon.parent_path ();

  bool recursive = (stacks.find (file_canon) != stacks.end ());
  stack.push_back (file_canon);
  stacks.insert (file_canon);

  stream.assertGood ();
  size_t lineNr = 1;
  bool hasLine = false;
  while (!stream->eof ()) {
    std::string line;
    std::getline (*stream, line);
    ASSERT (!stream->bad ());
    std::string trimmed = Core::trim (line);

    std::string target;
    bool isInclude = false;

    if (trimmed.substr (0, strlen ("#")) == "#") {
      std::string directive = Core::trim (trimmed.substr (strlen ("#")));
      if (directive.substr (0, strlen ("include")) == "include") {
        target = Core::trim (directive.substr (strlen ("include")));
        isInclude = true;
      }
    }

    if (isInclude) {
      ASSERT (target.length () > 2);
      ASSERT (target[0] == '<' || target[0] == '"');
      ASSERT (target[0] != '<' || target[target.length () - 1] == '>');
      ASSERT (target[0] != '"' || target[target.length () - 1] == '"');
      std::string t2 = target.substr (1, target.length () - 2);
      path ifile;
      bool found = false;
      if (target[0] == '<') {
        BOOST_FOREACH (const path& p, includePaths) {
          if (!found) {
            path t = p / t2;
            if (boost::filesystem::exists (t)) {
              found = true;
              ifile = t;
            }
          }
        }
        /*
          if (!found) {
          Core::OStream::getStderr () << "Could not find include '" << t2 << "' for file " << file_canon << std::endl;
          ABORT ();
          }
        */
      } else {
        found = true;
        ifile = path_canon / t2;
      }
      if (!found) {
        out << "#error Include " << OpenCL::escapeCString (t2) << " not found\n";
      } else if (recursive) {
        out << "#error Recursive include: file " << OpenCL::escapeCString (file_canon.string ()) << " includes file " << OpenCL::escapeCString (ifile.string ()) << "\n";
      } else {
        process (includePaths, ifile, stack, stacks, out);
        hasLine = false;
      }
    } else {
      if (!hasLine) {
        out << "#line " << lineNr << " " << OpenCL::escapeCString (file_canon.string ()) << "\n";
        hasLine = true;
      }
      out << line << "\n";
    }
    lineNr++;
  }

  ASSERT (stack.size () > 0 && stack[stack.size () - 1] == file_canon);
  stack.pop_back ();
  if (!recursive) {
    size_t count = stacks.erase (file_canon);
    ASSERT (count == 1);
  }
}

int main (int argc, char** argv) {
  if (argc < 2) {
    Core::OStream::getStderr () << "Usage: OpenCLProcessIncludes <filename.cl> [-Ipath ...]" << std::endl;
    return 1;
  }

  std::vector<path> includePaths;
  for (int i = 2; i < argc; i++) {
    std::string s = argv[i];
    if (s.substr (0, 2) != "-I") {
      Core::OStream::getStderr () << "Usage: OpenCLProcessIncludes <filename.cl> [-Ipath ...]" << std::endl;
      return 1;
    }
    includePaths.push_back (s.substr (2));
  }

  std::vector<path> stack;
  std::set<path> stacks;
  process (includePaths, argv[1], stack, stacks, Core::OStream::getStdout ());
  
  return 0;
}
