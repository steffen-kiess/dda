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

// Creates a wrapper .stub.hpp file for a .cl file which contains wrappers for
// the kernel calls

#include <Core/Assert.hpp>
#include <Core/File.hpp>
#include <Core/OStream.hpp>

#include <vector>
#include <string>
#include <cstdio>
#include <sstream>
#include <map>

#include <cstdlib>
#include <cstring>

const bool crDebug = false;

inline bool isEof (const std::string& data, size_t pos) {
  return pos == data.length ();
}

inline bool isIdChar (const std::string& data, size_t pos) {
  return !isEof (data, pos) && (data[pos] == '_' ||
                                (data[pos] >= '0' && data[pos] <= '9') ||
                                (data[pos] >= 'A' && data[pos] <= 'Z') ||
                                (data[pos] >= 'a' && data[pos] <= 'z'));
}

inline bool isWsChar (const std::string& data, size_t pos) {
  return !isEof (data, pos) && (data[pos] == ' ' || data[pos] == '\r' || data[pos] == '\n' || data[pos] == '\t');
}

inline void skipToEol (const std::string& data, size_t& pos) {
  while (!isEof (data, pos) && data[pos] != '\n')
    pos++;
  if (!isEof (data, pos))
    pos++;
}

inline void skipToEoc (const std::string& data, size_t& pos) {
  while (!isEof (data, pos)) {
    if (data[pos] == '*') {
      pos++;
      if (!isEof (data, pos)) {
        if (data[pos] == '/') { 
          pos++;
          return;
        }
      }
    } else {
      pos++;
    }
  }
}

inline void skipWs (const std::string& data, size_t& pos) {
  for (;;) {
    if (isEof (data, pos))
      return;
    if (data[pos] == '#' || (data[pos] == '/' && !isEof (data, pos + 1) && data[pos + 1] == '/'))
      skipToEol (data, pos);
    else if (data[pos] == '/' && !isEof (data, pos + 1) && data[pos + 1] == '*')
      skipToEoc (data, pos);
    else if (isWsChar (data, pos))
      pos++;
    else
      return;
  }
}

inline void skipNonId (const std::string& data, size_t& pos) {
  skipWs (data, pos);
  while (!isEof (data, pos) && !isIdChar (data, pos)) {
    pos++;
    skipWs (data, pos);
  }
}

inline std::string readId (const std::string& data, size_t& pos) {
  skipWs (data, pos);
  if (!isIdChar (data, pos)) {
    if (isEof (data, pos))
      Core::OStream::getStderr () << "Expected ID, got EOF" << std::endl;
    else
      Core::OStream::getStderr () << "Expected ID, got '" << data[pos] << "'" << std::endl;
    exit (1);
  }
  std::string id;
  while (isIdChar (data, pos))
    id += data[pos++];
  skipWs (data, pos);
  return id;
}

inline void expectId (const std::string& data, size_t& pos, const std::string& exp) {
  std::string id = readId (data, pos);
  if (id != exp) {
    Core::OStream::getStderr () << "Expected `" << exp << "', got `" << id << "'" << std::endl;
    exit (1);
  }
}

inline void expectStr (const std::string& data, size_t& pos, std::string exp) {
  skipWs (data, pos);
  for (size_t p = 0; p < exp.length (); p++) {
    if (isEof (data, pos)) {
      Core::OStream::getStderr () << "Expected '" << exp[p] << "' of `" << exp << "', got EOF" << std::endl;
      exit (1);
    }
    if (data[pos] != exp[p]) {
      Core::OStream::getStderr () << "Expected '" << exp[p] << "' of `" << exp << "', got '" << data[pos] << "'" << std::endl;
      exit (1);
    }
    pos++;
  }
  skipWs (data, pos);
}

inline void expectNEof (const std::string& data, size_t& pos) {
  skipWs (data, pos);
  if (isEof (data, pos)) {
    Core::OStream::getStderr () << "Got unexpected EOF" << std::endl;
    exit (1);
  }
}

struct Arg {
  std::string name, ty;
  bool isPointer;
  bool isLocal;
  bool isVoid;
  bool isStruct;
  bool isImage;
  bool isSampler;
  bool isGenArg;
  bool isConst;

  mutable bool isWrapped;

  Arg (std::string name, std::string ty, bool isPointer, bool isLocal, bool isVoid, bool isStruct, bool isImage, bool isSampler, bool isConst) : name (name), ty (ty), isPointer (isPointer), isLocal (isLocal), isVoid (isVoid), isStruct (isStruct), isImage (isImage), isSampler (isSampler), isGenArg (false), isConst (isConst), isWrapped (false) {}

  bool isDouble () const {
    return ty == "double" || ty == "cdouble" || ty == "cl_double";
  }
};

static bool isEqual (const Arg& a1, const Arg& a2) {
  return a1.name == a2.name
    && a1.ty == a2.ty
#define I(x) && a1.x == a2.x
  I(isPointer) I(isLocal) I(isVoid) I(isStruct) I(isImage) I(isSampler)
  I(isGenArg)
#undef I
    ;
}

static std::ostream& operator<< (std::ostream& out, const Arg& a) {
  out << "`" << a.ty << "' `" << a.name << "'";
#define I(x) if (a.x) out << " " #x;
  I(isPointer) I(isLocal) I(isVoid) I(isStruct) I(isImage) I(isSampler)
  I(isGenArg)
#undef I
  return out;
}

struct Kern {
  std::string name;
  std::vector<Arg> a;
  
  Kern () {}
  Kern (std::string name, std::vector<Arg> a) : name (name), a (a) {}

  bool hasDouble () const {
    for (std::vector<Arg>::const_iterator it = a.begin (); it != a.end (); it++)
      if (it->isDouble ())
        return true;

    return
      name.find ("__double") != std::string::npos || name.find ("__typename_double") != std::string::npos
      || name.find ("__cl_double") != std::string::npos || name.find ("__typename_cl_double") != std::string::npos
      || name.find ("__cdouble") != std::string::npos || name.find ("__typename_cdouble") != std::string::npos;
  }
};

struct TemplateFunc {
  std::string fullname;
  std::string tmplname;
  std::vector<std::string> args;
  std::vector<std::string> argVal;
  Kern kern;
  std::vector<TemplateFunc> impls;
};

static TemplateFunc parseTemplateFunc (Kern kern) {
  TemplateFunc f;

  const std::string& name = kern.name;

  f.fullname = name;

  size_t pos = name.find ("__");
  ASSERT (pos != std::string::npos);
  f.tmplname = name.substr (0, pos);

  pos += 2;
  size_t pos2;
  do {
    pos2 = name.find ("__", pos);
    std::string a;
    if (pos2 == std::string::npos) {
      a = name.substr (pos);
      pos = name.length ();
    } else {
      a = name.substr (pos, pos2 - pos);
      pos = pos2 + 2;
    }

    size_t upos = a.find ("_");
    if (upos == std::string::npos) {
      ASSERT (a != "");
      bool foundNonDigit = false;
      for (std::string::iterator it = a.begin (); it != a.end (); it++)
        if (*it < '0' || *it > '9')
          foundNonDigit = true;
      f.args.push_back (foundNonDigit ? "typename" : "int");
      f.argVal.push_back (a);
    } else {
      f.args.push_back (a.substr (0, upos));
      f.argVal.push_back (a.substr (upos + 1));
    }
  } while (pos2 != std::string::npos);

  Kern newKern;
  newKern.name = f.tmplname;
  for (size_t i = 0; i < kern.a.size (); i++) {
    Arg a = kern.a[i];
    if (!a.isImage && !a.isSampler && !a.isVoid) {
      for (size_t j = 0; j < f.args.size (); j++) {
        if (f.args[j] == "typename" && f.argVal[j] == a.ty) {
          std::stringstream str;
          str << "Arg" << j;
          a.ty = str.str ();
          a.isGenArg = true;
        } else if (f.args[j] == "typename" && (f.argVal[j] == "float" || f.argVal[j] == "double") && "c" + f.argVal[j] == a.ty) {
          std::stringstream str;
          str << "std::complex<Arg" << j << "> ";
          a.ty = str.str ();
          a.isGenArg = true;
        }
      }
    }
    newKern.a.push_back (a);
  }
  f.kern = newKern;

  return f;  
}

inline Arg readArg (const std::string& data, size_t& pos) {
  std::string s1 = readId (data, pos);

  bool isConst = false;

  if (s1 == "const" || s1 == "__const") {
    s1 = readId (data, pos);
    isConst = true;
  }

  if (s1 == "__global" || s1 == "__constant" || s1 == "__local" || s1 == "global" || s1 == "constant" || s1 == "local") {
    if (s1 == "constant" || s1 == "__constant")
      isConst = true;
    std::string ty = readId (data, pos);
    while (ty == "const" || ty == "__const" || ty == "struct") {
      if (ty == "const" || ty == "__const")
        isConst = true;
      ty = readId (data, pos);
    }
    if (ty == "unsigned")
      ty = "u" + readId (data, pos);
    expectStr (data, pos, "*");
    std::string name = readId (data, pos);

    return Arg (name, ty, true, s1 == "__local" || s1 == "local", false, false, false, false, isConst);
  } else if (s1 == "void") {
    return Arg ("", "", false, false, true, false, false, false, isConst);
  } else if (s1 == "struct") {
    std::string ty = readId (data, pos);
    std::string name = readId (data, pos);
    return Arg (name, ty, false, false, false, true, false, false, isConst);
  } else if (s1 == "read_only" || s1 == "__read_only" || s1 == "write_only" || s1 == "__write_only") {
    std::string ty = readId (data, pos);
    ASSERT (ty == "image2d_t" || ty == "image3d_t");
    std::string name = readId (data, pos);
    return Arg (name, ty, false, false, false, false, true, false, isConst);
  } else if (s1 == "image2d_t" || s1 == "image3d_t") {
    std::string name = readId (data, pos);
    return Arg (name, s1, false, false, false, false, true, false, isConst);
  } else if (s1 == "sampler_t") {
    std::string name = readId (data, pos);
    return Arg (name, "sampler_t", false, false, false, false, false, true, isConst);
  } else {
    if (s1 == "unsigned")
      s1 = "u" + readId (data, pos);
    std::string name = readId (data, pos);
    return Arg (name, s1, false, false, false, false, false, false, isConst);
  }
}

inline std::string typeConv (const Arg& a, bool ref = true, bool nest = false) {
  if (a.isPointer) {
    if (!a.isLocal) {
      //return std::string(ref ? "const " : "") + "cl::Buffer" + (ref ? "&" : "");
      Arg b = a;
      b.isPointer = false;
      b.isConst = false;
      //return std::string(ref ? "const " : "") + "OpenCL::StubHelper::BufferWr<" + typeConv (b, ref) + ">" + (ref ? "&" : "");
      a.isWrapped = true;
      if (nest)
        ASSERT (0 && "Nested pointer parameter");
      return std::string () + "OpenCL::StubHelper::" + (a.isConst ? "Const" : "") + "BufferWr<" + typeConv (b, false, true) + ">";
    } else {
      return "cl::LocalSpaceArg";
    }
  }

  if (a.isImage)
    return std::string(ref ? "const " : "") + "cl::Image" + (ref ? "&" : "");

  if (a.isSampler)
    return std::string(ref ? "const " : "") + "cl::Sampler" + (ref ? "&" : "");

  if (a.isStruct)
    return "struct " + a.ty;

#define S(x) if (a.ty == #x || a.ty == "cl_" #x) return nest ? "cl_" #x : (a.isWrapped = true, "OpenCL::StubHelper::IntWr<cl_" #x ">")
#define D(x) S(x); S(x##2); S(x##4); S(x##8); S(x##16)
  D(char); D(uchar);
  D(short); D(ushort);
  D(int); D(uint);
  D(long); D(ulong);

#undef S
#define S(x) if (a.ty == #x || a.ty == "cl_" #x) return "cl_" #x
  S(half); D(float); D(double);
#undef D
#undef S
  if (a.ty.substr (0, 3) == "cl_")
    return a.ty;

  //Core::OStream::getStderr () << "Unknown type `" << a.ty << "'" << std::endl;
  //exit (1);
  return a.ty;
}

static std::string getConstName (const Kern& kern) {
  ASSERT (kern.a.size () == 0);

  const std::string& name = kern.name;
  ASSERT (name.substr (0, strlen ("globalConstant_")) == "globalConstant_");
  std::string n = name.substr (strlen ("globalConstant_"));
  size_t pos = n.find ("_Name_");
  ASSERT (pos != std::string::npos);

  return n.substr (pos + strlen ("_Name_"));
}

static std::string getConstType (const Kern& kern) {
  const std::string& name = kern.name;
  ASSERT (name.substr (0, strlen ("globalConstant_")) == "globalConstant_");
  std::string n = name.substr (strlen ("globalConstant_"));
  size_t pos = n.find ("_Name_");
  ASSERT (pos != std::string::npos);
  n = n.substr (0, pos);

  ASSERT (kern.a.size () == 0);
  
#define S(a, b) if (n == #a) return "cl_" #b
#define D(x) S(x, x)
  D(char); D(uchar);
  D(short); D(ushort);
  D(int); D(uint);
  D(long); D(ulong);

  S(size_t, ulong);
#undef S
#undef D
  Core::OStream::getStderr () << "Unknown constant type `" + n + "' in const `" << name << "'" << std::endl;
  exit (1);
}

#define P "kernelStub_"

static const bool eventDef = false;

static void emit (std::string prefix, bool emitBody, std::string forward, std::ostream& out, const Kern& kern, const std::string& identi, const std::string& ident, const std::string& proto, const std::string& prolog, bool events, bool event) {
  const std::string& name = kern.name;
  const std::vector<Arg>& args = kern.a;
  out << ident << prefix << (event ? "cl::Event " : "void ") << name << " (const cl::CommandQueue& "P"launchQueue" << proto;
  for (size_t i = 0; i < args.size (); i++) {
    const Arg a = args[i];
    out << ", " ;
    out << typeConv (a) << " " << a.name;
  }
  if (events)
    out << ", const std::vector<cl::Event>& "P"launchEvents";
  if (event)
    out << ", OpenCL::StubHelper::GetEvent_t "P"eventPresArg" << (eventDef && forward == "" ? " = OpenCL::StubHelper::GetEvent_t ()" : "");
  else
    out << ", OpenCL::StubHelper::NoEvent_t "P"eventPresArg" << (!eventDef && forward == "" ? " = OpenCL::StubHelper::NoEvent_t ()" : "");
  out << ")";
  if (!emitBody) {
    out << ";" << std::endl;
    return;
  }
  out << " {" << std::endl;
  if (prolog != "")
    out << ident << identi << prolog << std::endl;
  if (forward != "") {
    out << ident << identi << "return " << forward << " ("P"launchQueue, "P"launchOffset, "P"launchGlobal, "P"launchLocal";
    for (size_t i = 0; i < args.size (); i++)
      out << ", " << args[i].name;
    if (events)
      out << ", "P"launchEvents";
    out << ", "P"eventPresArg);" << std::endl;
    out << ident << "}" << std::endl;
    return;
  }
  if (kern.hasDouble ()) {
    out << ident << identi << "if ("P"doubleSupport) {" << std::endl;
  } else {
    out << ident << identi << "if (1) {" << std::endl;
  }
  out << ident << identi << identi << "(void)"P"eventPresArg;" << std::endl;
  out << ident << identi << identi << "ASSERT ("P"launchGlobal.dimensions () >= 1 && "P"launchGlobal.dimensions () <= 3);" << std::endl;
  out << ident << identi << identi << "ASSERT ("P"launchLocal.dimensions () == 0 || "P"launchLocal.dimensions () == "P"launchGlobal.dimensions ());" << std::endl;
  out << ident << identi << identi << "ASSERT ("P"launchOffset.dimensions () == 0 || "P"launchOffset.dimensions () == "P"launchGlobal.dimensions ());" << std::endl;

  //std::string kernelRef = "this->"P"kernel_" + name;
  out << ident << identi << identi << "OpenCL::StubHelper::KernelContainer::Handle "P"handle (this->"P"kernel_" << name << ", this->"P"programRef, \"" << name << "\");" << std::endl;
  std::string kernelRef = ""P"handle ()";

  bool foundO = false;
  for (size_t i = 0; i < args.size (); i++) {
    const Arg a = args[i];
    if (!a.isLocal && (a.isPointer || a.isImage || a.isSampler))
      foundO = true;
  }
  if (foundO) {
    out << ident << identi << identi << "cl_context "P"context;" << std::endl;
    out << ident << identi << identi << "cl::detail::errHandler (clGetKernelInfo (" << kernelRef << " (), CL_KERNEL_CONTEXT, sizeof ("P"context), &"P"context, NULL), \"clGetKernelInfo\");" << std::endl;
  }
  for (size_t i = 0; i < args.size (); i++) {
    const Arg a = args[i];
    if (!a.isLocal && (a.isImage || a.isSampler))
      out << ident << identi << identi << "ASSERT (" << a.name << " () != NULL);" << std::endl;
    if (!a.isLocal && (a.isPointer || a.isImage || a.isSampler)) {
      const char* sl;
      const char* su;
      if (a.isPointer || a.isImage) {
        sl = "MemObject"; su = "MEM";
      } else if (a.isSampler) {
        sl = "Sampler"; su = "SAMPLER";
      } else {
        abort ();
      }
      out << ident << identi << identi << "if (" << a.name << ( a.isPointer ? ".value" : "" ) << " () != NULL) {" << std::endl;
      out << ident << identi << identi << identi << "cl_context "P"context2;" << std::endl;
      out << ident << identi << identi << identi << "cl::detail::errHandler (clGet" << sl << "Info (" << a.name << ( a.isPointer ? ".value" : "" ) << " (), CL_" << su << "_CONTEXT, sizeof ("P"context2), &"P"context2, NULL), \"clGet" << sl << "Info\");" << std::endl;
      out << ident << identi << identi << identi << "ASSERT_MSG ("P"context == "P"context2, \"Parameter `" << a.name << "' has invalid context\");" << std::endl;
      out << ident << identi << identi << "}" << std::endl;
    }
  }
  for (size_t i = 0; i < args.size (); i++) {
    const Arg a = args[i];
    typeConv (a);
    if (a.isLocal)
      out << ident << identi << identi << "const_cast<cl::Kernel&> (" << kernelRef << ").setArg (" << i << ", " << a.name << ".size_, NULL);" << std::endl;
    else if (a.isImage || a.isSampler)
      out << ident << identi << identi << "const_cast<cl::Kernel&> (" << kernelRef << ").setArg (" << i << ", sizeof (" << typeConv (a, false) << "), (" << a.name << " ()) ? &const_cast <" << typeConv (a, false) << "&> (" << a.name << ") () : NULL);" << std::endl;
    else if (/*a.isPointer*/ a.isWrapped)
      out << ident << identi << identi << "const_cast<cl::Kernel&> (" << kernelRef << ").setArg (" << i << ", sizeof (" << typeConv (a, false) << "), &" << a.name << ".value ());" << std::endl;
    else
      out << ident << identi << identi << "const_cast<cl::Kernel&> (" << kernelRef << ").setArg (" << i << ", sizeof (" << typeConv (a, false) << "), &" << a.name << ");" << std::endl;
  }
  //out << ");" << std::endl;
  std::string i2 = ident + identi + identi;
  if (event)
    out << ident << identi << identi << "cl::Event "P"event;" << std::endl;
  out << ident << identi << identi << "cl::detail::errHandler (clEnqueueNDRangeKernel (" << std::endl;
  out << i2 << P"launchQueue (), " << std::endl;
  out << i2 << kernelRef << " (), " << std::endl;
  out << i2 << "static_cast<cl_uint> ("P"launchGlobal.dimensions ()), " << std::endl;
  out << i2 << P"launchOffset.dimensions () != 0 ? static_cast<const size_t*> ("P"launchOffset) : NULL, " << std::endl;
  out << i2 << "static_cast<const size_t*> ("P"launchGlobal), " << std::endl;
  out << i2 << P"launchLocal.dimensions () != 0 ? static_cast<const size_t*> ("P"launchLocal) : NULL, " << std::endl;
  if (events) {
    out << i2 << "static_cast<cl_uint> ("P"launchEvents.size ())," << std::endl;
    out << i2 << "reinterpret_cast<const cl_event*> ("P"launchEvents.data ())," << std::endl;
  } else {
    out << i2 << "0," << std::endl;
    out << i2 << "NULL," << std::endl;
  }
  if (event)
    out << i2 << "&"P"event ()" << std::endl;
  else
    out << i2 << "NULL" << std::endl;
  out << ident << identi << identi << "), \"clEnqueueNDRangeKernel\");" << std::endl;
  if (event)
    out << ident << identi << identi << "return "P"event;" << std::endl;
  out << ident << identi << "} else {" << std::endl;
  out << ident << identi << identi << "ABORT_MSG (\"No double support\");" << std::endl;
  out << ident << identi << "}" << std::endl;
  out << ident << "}" << std::endl;
}

static void emit (std::string prefix, bool emitBody, std::string forward, std::ostream& out, const Kern& kern, const std::string& identi, const std::string& ident) {
  for (int j = 0; j <= 1; j++) {
    for (int i = 0; i <= 1; i++) {
      emit (prefix, emitBody, forward, out, kern, identi, ident, "", "cl::NDRange "P"launchOffset, "P"launchGlobal (1), "P"launchLocal (1);", i, j);
      emit (prefix, emitBody, forward, out, kern, identi, ident, ", cl::NDRange "P"launchGlobal", "cl::NDRange "P"launchOffset, "P"launchLocal;", i, j);
      emit (prefix, emitBody, forward, out, kern, identi, ident, ", cl::NDRange "P"launchGlobal, cl::NDRange "P"launchLocal", "cl::NDRange "P"launchOffset;", i, j);
      emit (prefix, emitBody, forward, out, kern, identi, ident, ", cl::NDRange "P"launchOffset, cl::NDRange "P"launchGlobal, cl::NDRange "P"launchLocal", "", i, j);
    }
  }
  out << ident << std::endl;
}

inline std::string repeat (const std::string& s, int count) {
  std::string r;
  for (int i = 0; i < count; i++)
    r += s;
  return r;
}

inline std::string findReplace (const std::string& input, const std::string& find, const std::string& replace) {
  ASSERT (find.length () != 0);

  std::string s = input;
  for (size_t pos = 0; (pos = s.find (find, pos)) != std::string::npos;) {
    s.replace (pos, find.length (), replace);
    pos += replace.length ();
  }
  return s;
}

int main (int argc, char** argv) {
  if (argc != 2 && argc != 3 && argc != 4 && argc != 5) {
    fprintf (stderr, "Usage: OpenCLCreateStubHpp Classname [file.cl [file.icl [output.hpp]]]\n");
    return 1;
  }

  std::string rdata = Core::readFileOrStdin (argc > 2 ? argv[2] : "-");
  std::string data = Core::readFileOrStdin (argc > 3 ? argv[3] : "-");
  size_t pos = 0;
  std::stringstream output;

  std::string identi = "  ";

  std::string cn = argv[1];
  
  std::vector<Kern> kerns;
  skipNonId (data, pos);
  while (!isEof (data, pos)) {
    std::string kid = readId (data, pos);
    if (kid == "__kernel" || kid == "kernel") {
      expectId (data, pos, "void");
      std::string name = readId (data, pos);
      expectStr (data, pos, "(");
      std::vector<Arg> v;
      for (;;) {
        Arg a = readArg (data, pos);
        v.push_back (a);
        expectNEof (data, pos);
        if (data[pos] != ',' && data[pos] != ')') {
          Core::OStream::getStderr () << "Expected ',' or ')', got '" << data[pos] << "'" << std::endl;
          exit (1);
        } else if (data[pos] == ')') {
          break;
        }
        pos++;
      }
      for (size_t i = 0; i < v.size (); i++)
        if (v[i].isVoid && v.size () != 1) {
          Core::OStream::getStderr () << "void is not only argument" << std::endl;
          exit (1);
        }
      if (v[0].isVoid)
        v.clear ();
      kerns.push_back (Kern (name, v));
    }
    skipNonId (data, pos);
  }

  bool needDoubleCheck = false;
  for (size_t i = 0; i < kerns.size (); i++)
    if (kerns[i].hasDouble ())
      needDoubleCheck = true;

  std::string guardName = cn;
  for (size_t i = 0; i < guardName.length (); i++)
    if (!isIdChar (guardName, i))
      guardName[i] = '_';
    else if (guardName[i] >= 'a' && guardName[i] <= 'z')
      guardName[i] = (char) (guardName[i] - 'a' + 'A');
  guardName += "_HPP";
  
  output << "#ifndef " << guardName << std::endl;
  output << "#define " << guardName << std::endl;
  output << std::endl;

  output << "#include <OpenCL/StubHelper.hpp>" << std::endl;
  output << std::endl;
  {
    size_t ipos = rdata.find ("#include ");
    if (ipos != 0)
      ipos = rdata.find ("\n#include ");
    while (ipos != std::string::npos) {
      if (rdata[ipos] == '\n')
        ipos++;
      size_t epos = rdata.find ("\n", ipos);
      std::string line = rdata.substr (ipos, epos == std::string::npos ? std::string::npos : epos - ipos);
      std::string inc = line.substr (std::string ("#include").length ());
      {
        size_t p = 0;
        while (p < inc.length () && (inc[p] == ' ' || inc[p] == '\t'))
          p++;
        inc = inc.substr (p);
      }
      {
        size_t p = inc.length ();
        while (p > 0 && (inc[p - 1] == ' ' || inc[p - 1] == '\t'))
          p--;
        inc = inc.substr (0, p);
      }
      ASSERT (inc.length () > 2);
      ASSERT ((inc[0] == '<' && inc[inc.length () - 1] == '>')
              || (inc[0] == '\"' && inc[inc.length () - 1] == '\"'));
      inc = inc.substr (1, inc.length () - 2);
      if (inc.length () > 2 && inc.substr (inc.length () - 2) == ".h")
        output << line << std::endl;
      ipos = rdata.find ("\n#include ", ipos);
    }
  }
  output << std::endl;
  if (crDebug) {
    output << "#include <Core/OStream.hpp>" << std::endl;
    output << std::endl;
  }

  size_t spos = 0, spos2 = 0;
  int ns = 0;
  while ((spos2 = cn.find ("::", spos)) != std::string::npos) {
    std::string n = cn.substr (spos, spos2 - spos);
    output << repeat (identi, ns) << "namespace " << n << " {" << std::endl;

    spos = spos2 + 2;
    ns++;
  }
  std::string tn = cn.substr (spos);

  output << repeat (identi, ns) << "class " << tn << "Data {" << std::endl;
  output << repeat (identi, ns + 1) << "friend class OpenCL::StubBase<" << tn << "Data>;" << std::endl;
  output << repeat (identi, ns + 1) << "" << std::endl;
  output << repeat (identi, ns + 1) << "static const char* const dataSource;" << std::endl;
  output << repeat (identi, ns + 1) << "static const size_t lenSource;" << std::endl;
  output << repeat (identi, ns + 1) << "" << std::endl;
  output << repeat (identi, ns + 1) << "static const char* const compileOptions;" << std::endl;
  output << repeat (identi, ns) << "};" << std::endl;
  output << repeat (identi, ns) << std::endl;

  output << repeat (identi, ns) << "class " << tn << " : public OpenCL::StubBase<" << tn << "Data> {" << std::endl;
  output << repeat (identi, ns + 1) << "NO_COPY_CLASS (" << tn << ");" << std::endl;
  output << repeat (identi, ns + 1) << std::endl;

  output << repeat (identi, ns + 1) << "cl::Program "P"programRef;" << std::endl;
  output << repeat (identi, ns + 1) << "cl::Context "P"contextRef;" << std::endl;
  if (needDoubleCheck)
    output << repeat (identi, ns + 1) << "bool "P"doubleSupport;" << std::endl;
  for (size_t i = 0; i < kerns.size (); i++) {
    if (kerns[i].name.substr (0, strlen ("globalConstant_")) != "globalConstant_")
      output << repeat (identi, ns + 1) << "OpenCL::StubHelper::KernelContainer "P"kernel_" << kerns[i].name << ";" << std::endl;
    else
      output << repeat (identi, ns + 1) << getConstType (kerns[i]) << " "P"constValue_" << kerns[i].name << ";" << std::endl;
  }
  output << repeat (identi, ns + 1) << std::endl;
  output << repeat (identi, ns + 1) << tn << " (const cl::Program& "P"program) : "P"programRef ("P"program), "P"contextRef (::OpenCL::clProgramGetContext ("P"programRef)) {" << std::endl;
  if (crDebug)
    output << repeat (identi, ns + 2) << "Core::OStream::getStdout () << \"Calling " << cn << "::" << tn << "(cl::Program)\" << std::endl;" << std::endl;
  if (needDoubleCheck)
    output << repeat (identi, ns + 2) << ""P"doubleSupport = OpenCL::StubHelper::contextHasDoubleSupport ("P"contextRef);" << std::endl;
  for (size_t i = 0; i < kerns.size (); i++) {
    if (kerns[i].hasDouble ())
      output << repeat (identi, ns + 2) << "if ("P"doubleSupport) {" << std::endl;
    else
      output << repeat (identi, ns + 2) << "{" << std::endl;
    if (kerns[i].name.substr (0, strlen ("globalConstant_")) != "globalConstant_") {
      output << repeat (identi, ns + 3) << "OpenCL::StubHelper::KernelContainer::Handle "P"handle_" << kerns[i].name << " (this->"P"kernel_" << kerns[i].name << ", "P"programRef, \"" << kerns[i].name << "\");" << std::endl;
      output << repeat (identi, ns + 3) << "ASSERT ("P"handle_" << kerns[i].name << " ().getInfo <CL_KERNEL_NUM_ARGS> () == " << kerns[i].a.size () << ");" << std::endl;
    } else {
      output << repeat (identi, ns + 3) << "cl::Kernel "P"kern_" << kerns[i].name << " ("P"programRef, \"" << kerns[i].name << "\");" << std::endl; 
      output << repeat (identi, ns + 3) << "ASSERT ("P"kern_" << kerns[i].name << ".getInfo <CL_KERNEL_NUM_ARGS> () == 0);" << std::endl;
      output << repeat (identi, ns + 3) << "std::vector<cl_device_id> "P"devs = "P"programRef.getInfo<CL_PROGRAM_DEVICES> ();" << std::endl; 
      output << repeat (identi, ns + 3) << "ASSERT ("P"devs.size () > 0);" << std::endl; 
      output << repeat (identi, ns + 3) << "size_t "P"gsize[3] = {0, 0, 0};" << std::endl; 
      output << repeat (identi, ns + 3) << "size_t "P"size[3];" << std::endl; 
      output << repeat (identi, ns + 3) << "for (size_t "P"i = 0; "P"i < "P"devs.size (); "P"i++) {" << std::endl; 
      output << repeat (identi, ns + 4) << "cl::detail::errHandler (cl::detail::getInfo (&clGetKernelWorkGroupInfo, "P"kern_" << kerns[i].name << " (), "P"devs["P"i], CL_KERNEL_COMPILE_WORK_GROUP_SIZE, &"P"size), \"clGetKernelWorkGroupInfo\");" << std::endl;
      output << repeat (identi, ns + 4) << "if ("P"i) {" << std::endl; 
      output << repeat (identi, ns + 5) << "ASSERT ("P"size[0] == "P"gsize[0]);" << std::endl; 
      output << repeat (identi, ns + 5) << "ASSERT ("P"size[1] == "P"gsize[1]);" << std::endl; 
      output << repeat (identi, ns + 5) << "ASSERT ("P"size[2] == "P"gsize[2]);" << std::endl; 
      output << repeat (identi, ns + 4) << "} else {" << std::endl;
      output << repeat (identi, ns + 5) << ""P"gsize[0] = "P"size[0];" << std::endl; 
      output << repeat (identi, ns + 5) << ""P"gsize[1] = "P"size[1];" << std::endl; 
      output << repeat (identi, ns + 5) << ""P"gsize[2] = "P"size[2];" << std::endl; 
      output << repeat (identi, ns + 4) << "}" << std::endl;
      output << repeat (identi, ns + 3) << "}" << std::endl;
      output << repeat (identi, ns + 3) << "ASSERT (("P"gsize[0] | 0xffff) == 0x100ffff);" << std::endl;
      output << repeat (identi, ns + 3) << "ASSERT (("P"gsize[1] | 0xffffff) == 0x1ffffff);" << std::endl;
      output << repeat (identi, ns + 3) << "ASSERT (("P"gsize[2] | 0xffffff) == 0x1ffffff);" << std::endl;
      output << repeat (identi, ns + 3) << "uint64_t "P"val = (((uint64_t) "P"gsize[0] & 0xffff) << 48) | (((uint64_t) "P"gsize[1] & 0xffffff) << 24) | (((uint64_t) "P"gsize[2] & 0xffffff) << 0);" << std::endl;
      output << repeat (identi, ns + 3) << ""P"constValue_" << kerns[i].name << " = static_cast<" << getConstType (kerns[i]) << "> ("P"val);" << std::endl;
    }
    output << repeat (identi, ns + 2) << "}" << std::endl;
  }
  output << repeat (identi, ns + 1) << "}" << std::endl;
  output << repeat (identi, ns + 1) << std::endl;

  output << repeat (identi, ns) << "public:" << std::endl;
  if (crDebug) {
    output << repeat (identi, ns + 1) << "~" << tn << " () {" << std::endl;
    output << repeat (identi, ns + 2) << "Core::OStream::getStdout () << \"Calling " << cn << "::~" << tn << "()\" << std::endl;" << std::endl;
    output << repeat (identi, ns + 1) << "}" << std::endl;
    output << repeat (identi, ns + 1) << std::endl;
  }

  output << repeat (identi, ns + 1) << "static boost::shared_ptr<" << tn << "> create (const cl::Program& "P"program) {" << std::endl;
  output << repeat (identi, ns + 2) << "return boost::shared_ptr<" << tn << "> (new " << tn << " ("P"program));" << std::endl;
  output << repeat (identi, ns + 1) << "}" << std::endl;
  
  output << repeat (identi, ns + 1) << "static boost::shared_ptr<" << tn << "> create (const cl::Context& "P"context) {" << std::endl;
  output << repeat (identi, ns + 2) << "return create (getProgram ("P"context));" << std::endl;
  output << repeat (identi, ns + 1) << "}" << std::endl;

  output << repeat (identi, ns + 1) << std::endl;

  output << repeat (identi, ns + 1) << "const cl::Context& context () {" << std::endl;
  output << repeat (identi, ns + 2) << "return "P"contextRef;" << std::endl;
  output << repeat (identi, ns + 1) << "}" << std::endl;
  output << repeat (identi, ns + 1) << std::endl;


  for (size_t i = 0; i < kerns.size (); i++)
    if (kerns[i].name.substr (0, strlen ("globalConstant_")) != "globalConstant_")
      emit ("", true, "", output, kerns[i], identi, repeat (identi, ns + 1));
    else
      output << repeat (identi, ns + 1) << getConstType (kerns[i]) << " " << getConstName (kerns[i]) << " () const { return this->"P"constValue_" << kerns[i].name << "; }" << std::endl;

  std::map<std::string, TemplateFunc> templ;
  for (size_t i = 0; i < kerns.size (); i++) {
    if (kerns[i].name.substr (0, strlen ("globalConstant_")) == "globalConstant_")
      continue;
    const Kern& k = kerns[i];
    size_t pos = k.name.find ("__");
    if (pos != std::string::npos) {
      TemplateFunc f = parseTemplateFunc (k);
      std::map<std::string, TemplateFunc>::iterator it = templ.find (f.tmplname);
      if (it == templ.end ()) {
        templ[f.tmplname] = f;
        it = templ.find (f.tmplname);
        ASSERT (it != templ.end ());
      }
      if (f.args.size () != it->second.args.size ()) {
        Core::OStream::getStderr () << "Template `" << f.tmplname << "' has " << f.args.size () << " template arguments in `" << f.fullname << "' and " << it->second.args.size () << " in `" << it->second.fullname << "'" << std::endl;
        exit (1);
      }
      for (size_t i = 0; i < f.args.size (); i++) {
        if (f.args[i] != it->second.args[i]) {
          Core::OStream::getStderr () << "Template `" << f.tmplname << "' has at position " << i << " template argument of type `" << f.args[i] << "' in `" << f.fullname << "' and of type `" << it->second.args[i] << "' in `" << it->second.fullname << "'" << std::endl;
          exit (1);
        }
      }
      for (size_t i = 0; i < f.kern.a.size (); i++) {
        if (!isEqual (f.kern.a[i], it->second.kern.a[i])) {
          Core::OStream::getStderr () << "Template `" << f.tmplname << "' has at position " << i << " argument of type `" << f.kern.a[i] << "' in `" << f.fullname << "' and of type `" << it->second.kern.a[i] << "' in `" << it->second.fullname << "'" << std::endl;
          exit (1);
        }
      }
      it->second.impls.push_back (f);
    }
  }

  std::map<std::string, TemplateFunc> templConst;
  for (size_t i = 0; i < kerns.size (); i++) {
    if (kerns[i].name.substr (0, strlen ("globalConstant_")) != "globalConstant_")
      continue;
    const Kern& k = kerns[i];
    size_t pos = k.name.find ("__");
    if (pos != std::string::npos) {
      TemplateFunc f = parseTemplateFunc (k);
      std::map<std::string, TemplateFunc>::iterator it = templConst.find (f.tmplname);
      if (it == templConst.end ()) {
        templConst[f.tmplname] = f;
        it = templConst.find (f.tmplname);
        ASSERT (it != templConst.end ());
      }
      for (size_t i = 0; i < f.args.size (); i++) {
        if (f.args[i] != it->second.args[i]) {
          Core::OStream::getStderr () << "Template constant `" << f.tmplname << "' has at position " << i << " template argument of type `" << f.args[i] << "' in `" << f.fullname << "' and of type `" << it->second.args[i] << "' in `" << it->second.fullname << "'" << std::endl;
          exit (1);
        }
      }
      it->second.impls.push_back (f);
    }
  }

  for (std::map<std::string, TemplateFunc>::iterator it = templ.begin (); it != templ.end (); it++) {
    const TemplateFunc& f = it->second;
    std::stringstream str;
    str << "template <";
    for (size_t i = 0; i < f.args.size (); i++) {
      if (i)
        str << ", ";
      str << f.args[i] << " Arg" << i;
    }
    str << "> ";
    //str << "__attribute__ ((error (\"Invalid instantiation, valid ones are: ";
    str << "ERROR_ATTRIBUTE (\"Invalid instantiation, valid ones are: ";
    for (std::vector<TemplateFunc>::const_iterator it2 = f.impls.begin (); it2 != f.impls.end (); it2++) {
      if (it2 != f.impls.begin ())
        str << ", ";
      str << "(";
      for (size_t i = 0; i < it2->argVal.size (); i++) {
        str << "Arg" << i << " = " << it2->argVal[i];
      }
      str << ")";
    }
    //str << "\"))) ";
    str << "\") ";
    emit (str.str (), false, "", output, f.kern, identi, repeat (identi, ns + 1));
  }

  for (std::map<std::string, TemplateFunc>::iterator it = templConst.begin (); it != templConst.end (); it++) {
    const TemplateFunc& f = it->second;
    std::stringstream str;
    str << "template <";
    for (size_t i = 0; i < f.args.size (); i++) {
      if (i)
        str << ", ";
      str << f.args[i] << " Arg" << i;
    }
    str << "> ";
    //str << "__attribute__ ((error (\"Invalid instantiation, valid ones are: ";
    str << "ERROR_ATTRIBUTE (\"Invalid instantiation, valid ones are: ";
    for (std::vector<TemplateFunc>::const_iterator it2 = f.impls.begin (); it2 != f.impls.end (); it2++) {
      if (it2 != f.impls.begin ())
        str << ", ";
      str << "(";
      for (size_t i = 0; i < it2->argVal.size (); i++) {
        str << "Arg" << i << " = " << it2->argVal[i];
      }
      str << ")";
    }
    //str << "\"))) ";
    str << "\") inline " << getConstType (f.kern) << " " << getConstName (f.kern) << " () const;" << std::endl;
    output << repeat (identi, ns + 1) << str.str ();
  }

  output << repeat (identi, ns) << "};" << std::endl;

  for (std::map<std::string, TemplateFunc>::iterator it = templ.begin (); it != templ.end (); it++) {
    const TemplateFunc& f = it->second;
    for (std::vector<TemplateFunc>::const_iterator it2 = f.impls.begin (); it2 != f.impls.end (); it2++) {
      Kern kern2 = it2->kern;
      std::stringstream str;
      str << "<";
      for (size_t i = 0; i < it2->argVal.size (); i++) {
        if (i)
          str << ", ";
        str << typeConv (Arg ("", it2->argVal[i], false, false, false, false, false, false, false));
      }
      str << ">";
      kern2.name = tn + "::" + kern2.name + str.str ();
      for (size_t i = 0; i < kern2.a.size (); i++) {
        if (kern2.a[i].isGenArg) {
          if (kern2.a[i].ty.substr (0, 3) == "Arg") {
            int nr = atoi (kern2.a[i].ty.substr (3).c_str ());
            kern2.a[i].ty = it2->argVal[nr];
          } else {
            ASSERT (kern2.a[i].ty.substr (0, 16) == "std::complex<Arg");
            int nr = atoi (kern2.a[i].ty.substr (16).c_str ());
            kern2.a[i].ty = "std::complex<" + it2->argVal[nr] + "> ";
          }
        }
      }
      emit ("template <> inline ", true, it2->fullname, output, kern2, identi, repeat (identi, ns));
    }
  }

  for (std::map<std::string, TemplateFunc>::iterator it = templConst.begin (); it != templConst.end (); it++) {
    const TemplateFunc& f = it->second;
    for (std::vector<TemplateFunc>::const_iterator it2 = f.impls.begin (); it2 != f.impls.end (); it2++) {
      Kern kern2 = it2->kern;
      std::stringstream str;
      str << "<";
      for (size_t i = 0; i < it2->argVal.size (); i++) {
        if (i)
          str << ", ";
        str << typeConv (Arg ("", it2->argVal[i], false, false, false, false, false, false, false));
      }
      str << ">";
      kern2.name = it2->fullname;
      output << repeat (identi, ns) << "template <> inline " << getConstType (f.kern) << " " << tn << "::" << getConstName (f.kern) << str.str () << " () const { return this->" << getConstName (kern2) << " (); }" << std::endl;
    }
  }

  for (int i = ns - 1; i >= 0; i--)
    output << repeat (identi, i) << "}" << std::endl;
    

  output << std::endl;
  output << "#endif // " << guardName << std::endl;

  Core::writeFileOrStdout (argc > 4 ? argv[4] : "-", output.str ());
}
