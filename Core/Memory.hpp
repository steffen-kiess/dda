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

#ifndef CORE_MEMORY_HPP_INCLUDED
#define CORE_MEMORY_HPP_INCLUDED

// Helper classes which will free() / LocalFree() a pointer on destruction

#include <Core/Util.hpp>

#include <cstdlib>

namespace Core {
  // Holds a pointer and calls free() on destruction
  template <typename T> class MallocRefHolder {
  public:
    T* p;
    MallocRefHolder (T* p) : p (p) {}
    ~MallocRefHolder () {
      free (p);
    }
  };

#if OS_WIN
  namespace Intern {
    // Moved to Memory.cpp to avoid having to include windows.h here
    void windowsLocalFree (void* v);
  }

  // Holds a pointer and calls LocalFree() on destruction
  template <typename T> class WindowsLocalRefHolder {
  public:
    T* p;
    WindowsLocalRefHolder (T* p) : p (p) {}
    ~WindowsLocalRefHolder () {
      Intern::windowsLocalFree (p);
    }
  };
#endif // OS_WIN
}

#endif // !CORE_MEMORY_HPP_INCLUDED
