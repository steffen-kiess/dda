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

// Code for calculating the difference between the fields in two text files

#include <Core/Assert.hpp>
#include <Core/StringUtil.hpp>
#include <Core/IStream.hpp>
#include <Core/OStream.hpp>

#include <Math/Float.hpp>

#include <sstream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <stdint.h>

#include <boost/lexical_cast.hpp>

static bool readNonemptyLine (const Core::IStream& stream, std::string& line) {
  line = "";
  while (Core::trim (line) == "") {
    if (stream->eof ())
      return false;
    stream.assertGood ();
    getline (*stream, line);
    ASSERT (!stream->bad ());
  }
  return true;
}

struct GroupInfo {
  size_t fieldCount;
  ldouble factor1;

  size_t values;
  ldouble diff;
  ldouble squaredSum1;
  ldouble squaredSum2;
  ldouble normalized;
};

static bool readFiles (const std::string& filename1, const std::string& filename2, const Core::IStream& stream1, const Core::IStream& stream2, std::vector<GroupInfo>& groups, size_t& n) {
  std::string line1;
  std::string line2;
  //bool had180 = false;

  bool read1, read2;
  read1 = readNonemptyLine (stream1, line1);
  read2 = readNonemptyLine (stream2, line2);
  while (read1 && read2) {
    n++;
    
    std::stringstream l1 (line1);
    std::stringstream l2 (line2);
    uint32_t pos = 0;
    for (size_t groupIdx = 0; groupIdx < groups.size (); groupIdx++) {
      GroupInfo& group = groups[groupIdx];
      for (size_t field = 0; field < group.fieldCount; field++) {
        ASSERT (l1.good ());
        ASSERT (l2.good ());
        ldouble d1 = 0.0l, d2 = 0.0l;
        l1 >> d1;
        l2 >> d2;
        //if (!l1.eof () && !l2.eof ()) {
        //ASSERT (l1.good ());
        //ASSERT (l2.good ());
        ASSERT (!l1.bad ());
        ASSERT (!l2.bad ());
        d1 *= group.factor1;
        group.values++;
        group.squaredSum1 += d1 * d1;
        group.squaredSum2 += d2 * d2;
        ldouble d = d2 - d1;
        d *= d;
        group.diff += d;
        //Core::OStream::getStderr () << d << std::endl;
        //}
      }
      pos++;
    }
    if (!l1.eof ()) {
      Core::OStream::getStderr () << "`" << filename1 << "' has extra fields on line " << n << std::endl;
      return false;
    }
    if (!l2.eof ()) {
      Core::OStream::getStderr () << "`" << filename2 << "' has extra fields on line " << n << std::endl;
      return false;
    }
    /*
      std::string s180 = "180.00 ";
      std::string s358 = "358.00 ";
      std::string s359 = "359.00 ";
      had180 = (line1.substr (0, s180.length ()) == s180 && line2.substr (0, s180.length ()) == s180)
      || (line1.substr (0, s358.length ()) == s358 && line2.substr (0, s358.length ()) == s358)
      || (line1.substr (0, s359.length ()) == s359 && line2.substr (0, s359.length ()) == s359);
    */
    read1 = readNonemptyLine (stream1, line1);
    read2 = readNonemptyLine (stream2, line2);
  }
  /*
    if ((read1 || read2) && had180) {
    Core::OStream::getStderr () << "`" << (read1 ? filename2 : filename1) << "' ends at line " << n << " after '180.00 ...'" << std::endl;
    return true;
  }
  */
  if (read1) {
    Core::OStream::getStderr () << "`" << filename1 << "' is longer than `" << filename2 << "'" << std::endl;
    return false;
  }
  if (read2) {
    Core::OStream::getStderr () << "`" << filename2 << "' is longer than `" << filename1 << "'" << std::endl;
    return false;
  }
  return true;
}

int main (int argc, const char** argv) {
  bool printOnly = false;
  ldouble maxError = 1e-4;
  if (argc >= 2 && std::string (argv[1]) == "--print-only") {
    printOnly = true;
    argc--;
    argv++;
  } else if (argc >= 3 && std::string (argv[1]) == "--max-error") {
    argc--;
    argv++;
    maxError = boost::lexical_cast<ldouble> (argv[1]);
    argc--;
    argv++;
  }
  if (argc < 4) {
    Core::OStream::getStderr () << "Usage: FieldDiff [--print-only | --max-error 1e-4] <filename1> <filename2> <group1len>:<group1factor1> ..." << std::endl;
    return 1;
  }
  std::string filename1 = argv[1];
  std::string filename2 = argv[2];
  Core::IStream stream1 = Core::IStream::open (filename1);
  Core::IStream stream2 = Core::IStream::open (filename2);
  std::vector<GroupInfo> groups;
  for (int i = 3; i < argc; i++) {
    std::vector<std::string> parts = Core::split (argv[i], ":");
    ASSERT (parts.size () == 1 || parts.size () == 2);

    GroupInfo group;

    group.fieldCount = boost::lexical_cast<uint32_t> (parts[0]);
    group.factor1 = parts.size () > 1 ? boost::lexical_cast<ldouble> (parts[1]) : 1;

    group.values = 0;
    group.diff = 0;
    group.squaredSum1 = 0;
    group.squaredSum2 = 0;
    group.normalized = 0.0 / 0.0;

    groups.push_back (group);
  }

  std::string line1;
  std::string line2;

  // ignore first line
  getline (*stream1, line1);
  getline (*stream2, line2);

  size_t n = 1;
  if (!readFiles (filename1, filename2, stream1, stream2, groups, n))
    return 1;

  // Divide by average squared size
  for (size_t i = 0; i < groups.size (); i++)
    groups[i].normalized = std::sqrt (groups[i].diff / ((groups[i].squaredSum1 + groups[i].squaredSum2) / 2));

  for (size_t i = 0; i < groups.size (); i++) {
    GroupInfo& group = groups[i];
    if (printOnly)
      Core::OStream::getStdout () << std::fixed << std::setprecision (15) << group.normalized << std::endl;
    else
      Core::OStream::getStdout () << "Read " << group.values << " values for group " << i << " on " << (n - 1) << " lines, got a squared difference of " << group.diff << ", avg " << (group.diff / group.values) << ", squared sum 1 is " << group.squaredSum1 << ", squared sum 2 is " << group.squaredSum2 << ", normalized error is " << std::fixed << std::setprecision (15) << (group.normalized * 100) << "%" << std::endl;
  }

  for (size_t i = 0; i < groups.size (); i++) {
    GroupInfo& group = groups[i];

    if (!printOnly && !(group.normalized < maxError)) {
      Core::OStream::getStderr () << "Avg difference of group " << i << " too large" << std::endl;
      return 1;
    }
  }

  return 0;
}
