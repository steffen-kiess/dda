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

#include "Exception.hpp"

#include <Core/Util.hpp>
#include <Core/Memory.hpp>
#include <Core/StrError.h>

#include <sstream>
#include <limits>
#include <iostream>
#include <iomanip>

#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cerrno>
#include <cstring>

#include <sys/types.h>

#if OS_UNIX && !defined (__CYGWIN__)
#include <execinfo.h>
#endif

#if OS_UNIX
#include <dlfcn.h>
#elif OS_WIN
#include <windows.h>
#include <imagehlp.h>
#endif

#ifndef _MSC_VER
#include <cxxabi.h>
#endif

#ifndef _MSC_VER
#include <unistd.h>
#else
#define popen _popen
#define pclose _pclose
static inline int MY_vsnprintf (char* str, size_t size, const char* format, va_list ap) {
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s (str, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf (format, ap);

    return count;
}
static inline int MY_snprintf (char* str, size_t size, const char* format, ...) {
    int count;
    va_list ap;

    va_start (ap, format);
    count = MY_vsnprintf (str, size, format, ap);
    va_end (ap);

    return count;
}
#define snprintf MY_snprintf
#endif

#if !OS_UNIX
#ifndef WEXITSTATUS
#define WEXITSTATUS(x) (x)
#endif
#endif

#if OS_WIN
// Define ptrint to be a pointer-size int
#ifdef _WIN64
typedef DWORD64 ptrint;
#else
typedef DWORD ptrint;
#endif
#endif

namespace Core {
  namespace {
#if OS_UNIX
    template <class T> inline T dlcheck (const char* name, T value) {
      if (!value) {
        std::string s = name;
        s += ": ";
        const char* msg = dlerror ();
        if (msg)
          s += msg;
        throw SimpleStdException (s);
      }
      return value;
    }
#endif

    inline int check (const char* name, int value) {
      if (value < 0) {
        std::string s = name;
        s += ": ";
#if OS_UNIX
        int err = errno;
        errno = 0;
        char buf[2048];
        const char* msg;
        if (MY_XSI_strerror_r (err, buf, sizeof (buf)) == 0)
          msg = buf;
        else
          msg = "<strerror_r failed>";
#else
        const char* msg = strerror (errno); // This will break with multiple threads
#endif
        if (msg)
          s += msg;
        throw SimpleStdException (s);
      }
      return value;
    }

#if OS_WIN
    inline ptrint checkWin (const char* name, ptrint value, DWORD ignore = 0) {
      if (!value) {
        DWORD err = GetLastError ();
        if (err == ignore)
          return value;

        std::stringstream str;
        char* lpMsgBuf;
        if (!FormatMessage (FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, err, MAKELANGID (LANG_NEUTRAL, SUBLANG_NEUTRAL), (char*) &lpMsgBuf, 0, NULL)) {
          DWORD err2 = GetLastError ();
          str << "FormatMessage () for " << err << " returned " << err2;
          throw SimpleStdException (str.str ());
        }
        WindowsLocalRefHolder<char> refHolder (lpMsgBuf);
        size_t len = strlen (lpMsgBuf);
        if (len && lpMsgBuf[len - 1] == '\n') {
          lpMsgBuf[len - 1] = 0;
          if ((len > 1) && lpMsgBuf[len - 2] == '\r')
            lpMsgBuf[len - 2] = 0;
        }
        str << name << ": " << lpMsgBuf << " (" << err << ")";
        throw SimpleStdException (str.str ());
      }
      return value;
    }

    std::string getCallingFilename () {
      std::vector<char> filename (128);
      DWORD size;
      while ((size = GetModuleFileNameA (NULL, filename.data (), filename.size ())) == filename.size ())
        filename.resize (filename.size () * 2);
      checkWin ("GetModuleFileNameA", size);
      
      return std::string (filename.data (), size);
    }
#endif

#define SIMPLE_ASSERT_L(x, l)                                           \
    do {                                                                \
      if (!(x)) {                                                       \
        throw SimpleStdException ("Simple assertion `" #x "' failed at " __FILE__ ":" #l); \
      }                                                                 \
    } while (0)
#define SIMPLE_ASSERT_L_(x, l) SIMPLE_ASSERT_L(x, l)
#define SIMPLE_ASSERT(x) SIMPLE_ASSERT_L_ (x, __LINE__)

  }

  SimpleStdException::SimpleStdException (std::string descr) : descr_ (descr) {
  }

  SimpleStdException::~SimpleStdException () throw () {
  }

  const char* SimpleStdException::what () const throw () {
    return descr_.c_str ();
  }

  InlineStackFrame::InlineStackFrame (const std::string& method, const std::string& sourceFile, uint64_t lineNumber) : _method (method), _sourceFile (sourceFile), _lineNumber (lineNumber) {
  }

  StackFrame::StackFrame (void* ptr) :
    _ptr (ptr), _isResolved (false),
    _hasSharedObject (false), _sharedObjectBase (NULL), _hasSymbol (false), _symbolAddr (NULL), // will be reinitialized later, set to something here to make compiler happy
    _hasAddr2line (false)
  {}

  namespace {
    inline std::string pad (const std::string str, size_t* size, bool left = false) {
      size_t len = str.length ();
      size_t missing = 0;
      if (size) {
        if (*size < len)
          *size = len;
        else
          missing = *size - len;
      }
      if (!missing)
        return str;
      if (left)
        return std::string (missing, ' ') + str;
      else
        return str + std::string (missing, ' ');
    }
  }

  std::string StackFrame::toString (int* i, size_t* addrSize, size_t* symbSize, size_t* locSize) const {
    std::stringstream str;
    const std::vector<InlineStackFrame> isfs = inlineStackFrames ();
    size_t size = isfs.size ();
    if (size == 0)
      size = 1;
    for (size_t j = 0; j < size; j++) {
      if (j > 0)
        str << std::endl;
      str.setf (std::ios::dec, std::ios::basefield);
      str.fill ('0');
      if (i) {
        str << " #" << std::setw (2) << (*i)++;
      }
      str.setf (std::ios::hex, std::ios::basefield);
      str << " at";
      {
        std::stringstream str2;
        str2 << ptr ();
        size_t len = str2.str ().length ();
        if (j > 0) {
          str << " [" << pad (std::string (len, ' '), addrSize, true) << "]";
        } else {
          str << " [" << pad (str2.str (), addrSize, true) << "]";
        }
      }

      /*
      if (hasSharedObject ())
        str << " " << sharedObjectName () << "+0x" << sharedObjectOffset ();
      else
        str << " no_shobj";
      */

      bool hasSymb = false;
      std::string symb;
      if (j < isfs.size ()) {
        const InlineStackFrame& isf = isfs[j];
        symb = isf.method ();
        if (symb != "??") {
          hasSymb = true;

          // demangle name
#ifndef _MSC_VER
          if (symb.substr (0, 2) == "_Z") {
            size_t len;
            int status;
            MallocRefHolder<char> demangled = abi::__cxa_demangle (symb.c_str (), NULL, &len, &status);
            SIMPLE_ASSERT (status == 0 || status == -2);
            if (status == 0)
              symb = demangled.p;
          }
#endif
        }
      }
      if (!hasSymb && hasSymbol ()) {
        symb = symbolName ();

        // demangle name
#ifndef _MSC_VER
        if (symb.substr (0, 2) == "_Z") {
          size_t len;
          int status;
          MallocRefHolder<char> demangled = abi::__cxa_demangle (symb.c_str (), NULL, &len, &status);
          SIMPLE_ASSERT (status == 0 || status == -2);
          if (status == 0)
            symb = demangled.p;
        }
#endif

        std::stringstream str2;
        str2 << symb << "+0x" << symbolOffset ();
        symb = str2.str ();
        hasSymb = true;
      }
      if (!hasSymb) {
        if (hasSharedObject () && sharedObjectName () != "") {
          std::stringstream str2;
          str2.setf (std::ios::hex, std::ios::basefield);
          str2 << "<nosymb " + sharedObjectName () + "+0x" << sharedObjectOffset () << ">";
          symb = str2.str ();
        } else {
          symb = "<no symbol information>";
        }
      }
      str << " " << pad (symb, symbSize);

      str.setf (std::ios::dec, std::ios::basefield);
      if (j < isfs.size ()) {
        const InlineStackFrame& isf = isfs[j];

        const int maxPathElements = 2;
        std::string fn = isf.sourceFile ();
        size_t pos = fn.length ();
        for (int i = 0; i < maxPathElements && pos != std::string::npos && pos != 0; i++)
          pos = fn.rfind ('/', pos - 1);
        if (pos != std::string::npos && pos != 0 && pos != fn.length ()) {
          fn = ".../" + fn.substr (pos + 1);
        }

        if (isf.lineNumber ()) {
          std::stringstream str2;
          str2 << fn << ":" << isf.lineNumber ();
          str << " (" << pad (str2.str (), locSize) << ")";
        }
      } else {
        //str << " no_addr2line";
      }
    }

    return str.str ();
  }

  void StackFrame::doResolve () const {
    if (_isResolved)
      abort ();

#if OS_UNIX && !defined (__CYGWIN__)
    Dl_info info;

    //dlerror ();
    //dlcheck ("dladdr", dladdr (ptr (), &info));
    if (!dladdr (ptr (), &info)) {
      _hasSharedObject = false;
      _sharedObjectBase = NULL;
      _hasSymbol = false;
      _symbolAddr = NULL;
    } else {
    
      if (info.dli_fname) {
        _hasSharedObject = true;
        _sharedObjectName = info.dli_fname;
        _sharedObjectBase = info.dli_fbase;
      } else {
        _hasSharedObject = false;
        _sharedObjectBase = NULL;
      }

      if (info.dli_sname) {
        _hasSymbol = true;
        _symbolName = info.dli_sname;
        _symbolAddr = info.dli_saddr;
      } else {
        _hasSymbol = false;
        _symbolAddr = NULL;
      }

    }

#elif OS_WIN
    checkWin ("SymInitialize", SymInitialize (GetCurrentProcess(), 0, TRUE));

    {
      _sharedObjectBase = (void*) checkWin ("SymGetModuleBase", SymGetModuleBase (GetCurrentProcess (), (ptrint) ptr ()));

      std::vector<char> filename (128);
      DWORD size;
      while ((size = GetModuleFileNameA ((HINSTANCE) _sharedObjectBase, filename.data (), filename.size ())) == filename.size ())
        filename.resize (filename.size () * 2);
      checkWin ("GetModuleFileNameA", size);

      _hasSharedObject = true;
      _sharedObjectName = std::string (filename.data (), size);
    }

    const size_t nameLen = 4096;
    const size_t size = sizeof (IMAGEHLP_SYMBOL) + nameLen;
    MallocRefHolder<IMAGEHLP_SYMBOL> buffer ((IMAGEHLP_SYMBOL*) malloc (size));
    buffer.p->SizeOfStruct = sizeof (IMAGEHLP_SYMBOL);
    buffer.p->MaxNameLength = nameLen - 1;
    ptrint disp = 0;
    if (SymInitialize(GetCurrentProcess(), 0, TRUE) && SymGetSymFromAddr (GetCurrentProcess (), (ptrint) ptr (), &disp, buffer.p)) {
      _hasSymbol = true;
      _symbolName = buffer.p->Name;
      _symbolAddr = (void*) buffer.p->Address;
      /*
      if (_symbolAddr != (void*) ((char*) ptr () - disp))
        abort ();
      */
    } else {
      _hasSymbol = false;
      _symbolAddr = NULL;
    }
#endif

    _isResolved = true;
  }

  static std::string escape (const std::string& s) {
    std::stringstream str;
#if OS_UNIX
    str << "'";
    for (size_t i = 0; i < s.length (); i++)
      if (s[i] == '\'')
        str << "'\\''";
      else
        str << s[i];
    str << "'";
#elif OS_WIN
    str << "\"";
    for (size_t i = 0; i < s.length (); i++) {
      SIMPLE_ASSERT (s[i] != '"');
      str << s[i];
    }
    str << "\"";
#else
#error
#endif
    return str.str ();
  }

  void StackFrame::doAddr2line () const {
    if (_hasAddr2line)
      abort ();
    if (_inlineStackFrames.size () != 0)
      abort ();

#if OS_WIN
    // Not implemented
    _hasAddr2line = true;
    return;
#endif

    try {
      // Hack to avoid looking up JITted functions
      if (hasSharedObject () && sharedObjectName ().substr (0, 4) == "mono") {
        _hasAddr2line = true;
        return;
      }

      std::vector<InlineStackFrame> frames;

      bool noinfo = false;

      std::stringstream str;

      bool sharedLibrary = ((uintptr_t) ptr ()) > 0x40000000 && hasSharedObject (); // XTODO: Replace this hack by better way to determine whether this is a shared library

      str << "addr2line -ife ";
      if (hasSharedObject () && sharedObjectName () != "") {
        //str << sharedObjectName ();
        str << escape (sharedObjectName ());
      } else {
#if OS_UNIX
        str << "/proc/" << getpid () << "/exe";
#elif OS_WIN
        str << "\"" << getCallingFilename () << "\"";
#else
#error
#endif
      }

      // buffer[i] points to the return address, subtract 1 to get an address
      // in the call instruction
      str.setf (std::ios::hex, std::ios::basefield);
      if (sharedLibrary)
        str << " 0x" << sharedObjectOffset () - 1;
      else
        str << " " << static_cast<void*> (static_cast<char*> (ptr ()) - 1);
      std::string cmd = str.str ();
      std::string cmdExpl = " (addr2line invocation was `" + cmd + "')";
      //std::cout << cmd << std::endl;
      FILE* pipe = popen (cmd.c_str (), "r");
      check ("popen", pipe == NULL ? -1 : 0);
      bool eof = false;
      bool r = false;
      while (!eof) {
        std::string line;
        int c;
        c = fgetc (pipe);
        if (c == EOF) {
          eof = true;
          if (!r) {
            int ret = check ("pclose", pclose (pipe));
            if (ret > 0) {
              char buf[256] = "";
              snprintf (buf, sizeof (buf), "addr2line returned %d", WEXITSTATUS (ret));
              throw SimpleStdException (buf + cmdExpl);
            }
            throw SimpleStdException ("unexpected EOF from addr2line (0 bytes read) + cmdExpl");
          }
        } else {
          if (noinfo) {
            int ret = check ("pclose", pclose (pipe));
            if (ret > 0) {
              char buf[256] = "";
              snprintf (buf, sizeof (buf), "addr2line returned %d", WEXITSTATUS (ret));
              throw SimpleStdException (buf + cmdExpl);
            }
            throw SimpleStdException ("got line from addr2line after ?? ??:0" + cmdExpl);
          }

          r = true;
          while (c != EOF && c != '\n') {
            line += (char) c;
            c = fgetc (pipe);
          }
          if (c == EOF) {
            int ret = check ("pclose", pclose (pipe));
            if (ret > 0) {
              char buf[256] = "";
              snprintf (buf, sizeof (buf), "addr2line returned %d (mid of line)", WEXITSTATUS (ret));
              throw SimpleStdException (buf + cmdExpl);
            }
            throw SimpleStdException ("unexpected EOF from addr2line" + cmdExpl);
          }

          std::string line2;
          c = fgetc (pipe);
          while (c != EOF && c != '\n') {
            line2 += (char) c;
            c = fgetc (pipe);
          }
          if (c == EOF) {
            int ret = check ("pclose", pclose (pipe));
            if (ret > 0) {
              char buf[256] = "";
              snprintf (buf, sizeof (buf), "addr2line returned %d (second line)", WEXITSTATUS (ret));
              throw SimpleStdException (buf + cmdExpl);
            }
            throw SimpleStdException ("unexpected EOF from addr2line" + cmdExpl);
          }

          if (line == "??" && (line2 == "??:0" || line2 == "??:?")) {
            if (frames.size () != 0) {
              int ret = check ("pclose", pclose (pipe));
              if (ret > 0) {
                char buf[256] = "";
                snprintf (buf, sizeof (buf), "addr2line returned %d", WEXITSTATUS (ret));
                throw SimpleStdException (buf + cmdExpl);
              }
              throw SimpleStdException ("got ?? ??:0 line from addr2line after other lines" + cmdExpl);
            }
            noinfo = true;
          } else {
            long lineNr;
            std::string sourceFile;
            if (line2 == "??:0" || line2 == "??:?") {
              lineNr = 0;
              sourceFile = "";
            } else {
              size_t pos = line2.rfind (':');
              if (pos == std::string::npos) {
                int ret = check ("pclose", pclose (pipe));
                if (ret > 0) {
                  char buf[256] = "";
                  snprintf (buf, sizeof (buf), "addr2line returned %d", WEXITSTATUS (ret));
                  throw SimpleStdException (buf + cmdExpl);
                }
                throw SimpleStdException ("got no line number from addr2line in '" + line2 + "'" + cmdExpl);
              }
              sourceFile = line2.substr (0, pos);
              std::string lineString = line2.substr (pos + 1);

              // Remove " (discriminator 1)" at end of string
              std::size_t lineSpacePos = lineString.find (' ');
              if (lineSpacePos != std::string::npos)
                lineString = lineString.substr (0, lineSpacePos);

              const char* lineStringC = lineString.c_str ();
              char* end = NULL;
              lineNr = strtol (lineStringC, &end, 10);
              if (end != lineStringC + lineString.length () || /*lineNr <= 0*/ lineNr < 0) {
                int ret = check ("pclose", pclose (pipe));
                if (ret > 0) {
                  char buf[256] = "";
                  snprintf (buf, sizeof (buf), "addr2line returned %d", WEXITSTATUS (ret));
                  throw SimpleStdException (buf + cmdExpl);
                }
                throw SimpleStdException ("got invalid line number from addr2line '" + lineString + "'" + cmdExpl);
              }
            }
            frames.push_back (InlineStackFrame (line, sourceFile, lineNr));
          }
        }
      }
      int ret = check ("pclose", pclose (pipe));
      if (ret > 0) {
        char buf[256] = "";
        snprintf (buf, sizeof (buf), "addr2line returned %d (beginning of line)", WEXITSTATUS (ret));
        throw SimpleStdException (buf);
      }
    
      if (!noinfo && frames.size () == 0) {
        throw SimpleStdException ("got no data from addr2line\n");
      }

      swap (_inlineStackFrames, frames);
      _hasAddr2line = true;
    } catch (const SimpleStdException& e) {
      std::cerr << "Error while calling addr2line: " << e.what () << std::endl;
      _hasAddr2line = true;
    }
  }

  const StackTrace::CreateFromCurrentThread_t StackTrace::createFromCurrentThread = StackTrace::CreateFromCurrentThread_t ();
  StackTrace::StackTrace (UNUSED CreateFromCurrentThread_t ignore) {
    std::vector<void*> buffer (16);
#if OS_UNIX && !defined (__CYGWIN__)
    int nr;
    while ((unsigned int) (nr = backtrace (&buffer.front (), static_cast<int> (buffer.size ()))) == buffer.size ()) {
      buffer.resize (buffer.size () * 2);
      assert (buffer.size () <= (unsigned int) std::numeric_limits<int>::max ());
    }

    assert (nr >= 0);

    for (int i = 0; i < nr; i++) {
      _frames.push_back (StackFrame (buffer[i]));
    }
#elif OS_WIN
    ptrint eip, esp, ebp;
#ifdef _MSC_VER
    CONTEXT context;
    RtlCaptureContext (&context);
#ifdef _WIN64
    eip = context.Rip;
    esp = context.Rsp;
    ebp = context.Rbp;
#else
    eip = context.Eip;
    esp = context.Esp;
    ebp = context.Ebp;
#endif
#else
    asm (
         "call L_a%= \n\t"
         "L_a%=: pop %0 \n\t"

#ifdef _WIN64
         "mov %%rsp, %1 \n\t"
         
         "mov %%rbp, %2 \n\t"
#else
         "mov %%esp, %1 \n\t"
         
         "mov %%ebp, %2 \n\t"
#endif
         : "=g" (eip),
           "=g" (esp),
           "=g" (ebp)
         );
#endif
    
    STACKFRAME frame;
    memset (&frame, 0, sizeof (frame));
    frame.AddrPC.Mode = AddrModeFlat;
    frame.AddrPC.Offset = eip;
    frame.AddrStack.Mode = AddrModeFlat;
    frame.AddrStack.Offset = esp;
    frame.AddrFrame.Mode = AddrModeFlat;
    frame.AddrFrame.Offset = ebp;
    
    while (StackWalk (IMAGE_FILE_MACHINE_I386, GetCurrentProcess (), GetCurrentThread (), &frame, NULL, NULL, SymFunctionTableAccess, SymGetModuleBase, NULL)) {
      _frames.push_back (StackFrame ((void*) frame.AddrPC.Offset));
    }

    DWORD err = GetLastError ();
    if (err != ERROR_NOACCESS && err != ERROR_INVALID_ADDRESS) {
      try {
        checkWin ("StackWalk", 0);
      } catch (const SimpleStdException& e) {
        std::cerr << "Error while creating stack trace: " << e.what () << std::endl;
      }
    }
#else
    // Do nothing => empty stack trace
#endif
  }


  std::string StackTrace::toString () const {
    std::stringstream str;

    int i = 0;
    size_t addrSize = 0; //, symbSize = 0, locSize = 0;
    for (std::vector<StackFrame>::const_iterator it = frames ().begin (); it != frames ().end (); it++)
      //it->toString (NULL, &addrSize, &symbSize, &locSize);
      it->toString (NULL, &addrSize, NULL, NULL);
    for (std::vector<StackFrame>::const_iterator it = frames ().begin (); it != frames ().end (); it++)
      //str << it->toString (&i, &addrSize, &symbSize, &locSize) << std::endl;
      str << it->toString (&i, &addrSize, NULL, NULL) << std::endl;
    return str.str ();
  }

  Exception::~Exception () throw () {}

  void Exception::writeTo (std::ostream& stream) const throw () {
    stream << message () << std::endl;

    try {
      stream << stackTrace ().toString ();
    } catch (std::exception& e) {
      stream << "Error getting stack trace: ";
      stream << e.what ();
    } catch (...) {
      stream << "Error getting stack trace";
    }
    stream << std::flush;
  }

  std::string Exception::toString () const throw () {
    std::stringstream str;
    writeTo (str);
    return str.str ();
  }

  const char* Exception::what () const throw () {
    if (whatValueComputed)
      return whatValue.c_str ();

    whatValue = toString ();

    whatValueComputed = true;
    return whatValue.c_str ();
  }
}
