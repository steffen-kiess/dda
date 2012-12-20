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

// Creates a wrapper .stub.cpp file for a .cl file which contains the source
// code as a string constant

#include <vector>
#include <string>
#include <cstdio>
#include <sstream>
#include <map>

#include <cstdlib>
#include <cstring>

#include <Core/Assert.hpp>
#include <Core/File.hpp>

#include <OpenCL/CLangUtil.hpp>

int main (int argc, char** argv) {
  if (argc < 4) {
    fprintf (stderr, "Usage: OpenCLCreateStubCpp Classname file.inccl file.stub.cpp [opencl compiler options]\n");
    return 1;
  }

  std::string data = Core::readFileOrStdin (argv[2]);
  std::stringstream output;

  std::string cn = argv[1];

  std::string options;
  for (int i = 4; i < argc; i++) {
    if (options != "")
      options += " ";
    options += argv[i];
  }

  output << "#include <cstddef>" << std::endl;
  output << "#include <stdint.h>" << std::endl;
  output << std::endl;
  output << "namespace OpenCL {" << std::endl;
  output << "  template <typename T> class StubBase;" << std::endl;
  output << "}" << std::endl;

  size_t spos = 0, spos2 = 0;
  int ns = 0;
  while ((spos2 = cn.find ("::", spos)) != std::string::npos) {
    std::string n = cn.substr (spos, spos2 - spos);
    output << "namespace " << n << " {" << std::endl;

    spos = spos2 + 2;
    ns++;
  }
  std::string tn = cn.substr (spos);

  output << "class " << tn << "Data {" << std::endl;
  output << "  friend class OpenCL::StubBase<" << tn << "Data>;" << std::endl;
  output << "  " << std::endl;
  output << "  static const char* const dataSource;" << std::endl;
  output << "  static const size_t lenSource;" << std::endl;
  output << "  " << std::endl;
  output << "  static const char* const compileOptions;" << std::endl;
  output << "};" << std::endl;
  output << std::endl;
  output << "const char* const " << tn << "Data::dataSource = " << OpenCL::escapeCString (data) << ";" << std::endl;
  output << "const size_t " << tn << "Data::lenSource = " << data.length () << ";" << std::endl;
  output << "const char* const " << tn << "Data::compileOptions = " << OpenCL::escapeCString (options) << ";" << std::endl;;

  for (int i = ns - 1; i >= 0; i--)
    output << "}" << std::endl;

  Core::writeFileOrStdout (argv[3], output.str ());
}
