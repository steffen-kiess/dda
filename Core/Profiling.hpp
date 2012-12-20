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

#ifndef CORE_PROFILING_HPP_INCLUDED
#define CORE_PROFILING_HPP_INCLUDED

// Profiling classes which record the time while a ProfileHandle is existing
//
// Can be disabled at compile time by setting the preprocessor constant
// USE_PROFILING to 0

#include <Core/Time.hpp>
#include <Core/Util.hpp>
#include <Core/Assert.hpp>

#include <stack>
#include <map>
#include <string>

#ifndef USE_PROFILING
#define USE_PROFILING 1
#endif

namespace Core {
  struct ProfilingTimes {
    TimeSpan real;
    TimeSpan user;
    TimeSpan system;

    ProfilingTimes () : real (0), user (0), system (0) {
    }

    ProfilingTimes (TimeSpan real, TimeSpan user, TimeSpan system) : real (real), user (user), system (system) {
    }

    std::string toString () const;

    static ProfilingTimes get () {
      return ProfilingTimes (getCurrentTime (), getCpuUserTime (), getCpuSystemTime ());
    }
  };
  
  static inline ProfilingTimes operator +(ProfilingTimes o1, ProfilingTimes o2) {
    return ProfilingTimes (o1.real + o2.real, o1.user + o2.user, o1.system + o2.system);
  }

  static inline ProfilingTimes operator -(ProfilingTimes o1, ProfilingTimes o2) {
    return ProfilingTimes (o1.real - o2.real, o1.user - o2.user, o1.system - o2.system);
  }

  static inline ProfilingTimes operator +=(ProfilingTimes& o1, ProfilingTimes o2) {
    return o1 = o1 + o2;
  }

  static inline ProfilingTimes operator -=(ProfilingTimes& o1, ProfilingTimes o2) {
    return o1 = o1 - o2;
  }

  class ProfilingData {
    NO_COPY_CLASS (ProfilingData);

    friend class ProfileHandle;

#if USE_PROFILING
    ProfilingTimes lastTime;

    bool enabled; // when false disable everything
    uint32_t lastnr;
    std::stack<std::pair<std::string, std::string> > nameStack;
    std::map<uint32_t, ProfilingTimes> times;
    std::map<std::string, uint32_t> names;
    std::map<uint32_t, std::string> namesR;
#endif

    std::string doToString ();
    std::string doToStringNop () {
      return "Profiling support disabled during compilation\n";
    }

    public:
    // Create a new ProfilingData instance. If enabled is false, everything
    // is disabled.
    ProfilingData (bool enabled);
  
    // Push a new name to the stack. Can be called on method entry to
    // record the time needed by every method.
    void push (std::string name);
    // Pop a name. name must be the uppermost name on the stack.
    // Record the time since the push without the times where other names
    // where pushed over this name.
    void pop (std::string name);
  
    // Get a string describing the results.
    std::string toString ();

    ProfilingTimes getOverallTimes ();
  };

  class ProfilingDataPtr {
#if USE_PROFILING
    ProfilingData* ptr;
#endif
    static ProfilingData unused;

  public:
    ProfilingDataPtr (UNUSED ProfilingData& data)
#if USE_PROFILING
      : ptr (&data)
#endif
    {}

    ProfilingDataPtr ()
#if USE_PROFILING
      : ptr (&unused)
#endif
    {}

    ProfilingData& operator * () {
#if USE_PROFILING
      return *ptr;
#else
      return unused;
#endif
    }

    ProfilingData* operator -> () {
#if USE_PROFILING
      return ptr;
#else
      return &unused;
#endif
    }
  };

  // A Handle which on construction will push a name and on destruction will
  // pop the name. (Destruction normally occurs when a variable is leaving scope).
  class ProfileHandle {
    NO_COPY_CLASS (ProfileHandle);

#if USE_PROFILING
    ProfilingData& data;
    std::string name;
#endif

  public:
    // Create a profile handle, push name
    ProfileHandle (ProfilingData& data, std::string name);
    ProfileHandle (ProfilingDataPtr data, std::string name);
    // pop name
    ~ProfileHandle ();
  };

#if USE_PROFILING

  inline ProfilingData::ProfilingData (bool enabled) {
    this->enabled = enabled;
    lastnr = 0;

    if (!enabled) 
      return;

    lastTime = ProfilingTimes::get ();
  }

  inline void ProfilingData::push (std::string name) {
    if (!enabled) 
      return;

    ProfilingTimes val = ProfilingTimes::get ();
    //Core::OStream::getStdout () << "push " << name << " " << val << std::endl;
    std::string lname = name;
    if (!nameStack.empty ()) {
      //Core::OStream::getStdout () << "X" << name << "ZZZZ" << std::endl;
      lname = nameStack.top ().second + "." + lname;
      ProfilingTimes diff = val - lastTime;
      times[names[nameStack.top ().second]] += diff;
    }
    nameStack.push (std::pair<std::string, std::string> (name, lname));
    if (names.find (lname) == names.end ()) {
      uint32_t nr = lastnr++;
      names[lname] = nr;
      namesR[nr] = lname;
      times[nr] = ProfilingTimes ();
      //Core::OStream::getStdout () << "X" << lname << "Y" << std::endl;
    }
    //lastTime = ProfilingTimes::get (); // This will exclude profiling time
    lastTime = val; // This will include profiling time
  }

  inline void ProfilingData::pop (std::string name) {
    if (!enabled) 
      return;

    ProfilingTimes val = ProfilingTimes::get ();
    //Core::OStream::getStdout () << "pop " << name << " " << val << std::endl;
    ASSERT (!nameStack.empty ());
    std::pair<std::string, std::string> t = nameStack.top ();
    nameStack.pop ();

    ASSERT (t.first == name);
    ProfilingTimes diff = val - lastTime;
    std::string lname = t.second;
    
    uint32_t nr = names[lname];
    times[nr] += diff;
    
    //lastTime = ProfilingTimes::get (); // This will exclude profiling time
    lastTime = val; // This will include profiling time
  }

  inline std::string ProfilingData::toString () {
    return doToString ();
  }

  inline ProfileHandle::ProfileHandle (ProfilingData& data, std::string name) : data (data) {
    if (!data.enabled)
      return;

    this->name = name;
    data.push (name);
  }
  
  inline ProfileHandle::ProfileHandle (ProfilingDataPtr data, std::string name) : data (*data) {
    if (!data->enabled)
      return;

    this->name = name;
    data->push (name);
  }

  inline ProfileHandle::~ProfileHandle () {
    if (!data.enabled)
      return;

    data.pop (name);
  }

#else

  // Do-nothing versions of the class when profiling is disabled at compile time.

  inline ProfilingData::ProfilingData (UNUSED bool enabled) {
  }
  inline void ProfilingData::push (UNUSED std::string name) {
  }
  inline void ProfilingData::pop (UNUSED std::string name) {
  }
  inline std::string ProfilingData::toString () {
    return doToStringNop ();
  }

  inline ProfileHandle::ProfileHandle (UNUSED ProfilingData& data, UNUSED std::string name) {}
  inline ProfileHandle::ProfileHandle (UNUSED ProfilingDataPtr data, UNUSED std::string name) {}
  inline ProfileHandle::~ProfileHandle () {}
#endif
}

#endif // !CORE_PROFILING_HPP_INCLUDED
