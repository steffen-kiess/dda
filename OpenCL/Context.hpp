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

#ifndef OPENCL_CONTEXT_HPP_INCLUDED
#define OPENCL_CONTEXT_HPP_INCLUDED

// Code for creating a opencl context

#include <Core/Assert.hpp>
#include <Core/Util.hpp>
#include <Core/OStream.hpp>

#include <OpenCL/Bindings.hpp>

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>

namespace OpenCL {
  class PlatformEntry;
  class DeviceEntry;

  class PlatformEntry {
    NO_COPY_CLASS(PlatformEntry);

    friend class DeviceEntry;

    size_t id_;
    cl::Platform platform_;
    std::string name_;
    std::vector<DeviceEntry> devices_;

  public:
    PlatformEntry (size_t id, const cl::Platform& platform);

    size_t id () const {
      return id_;
    }

    const cl::Platform& platform () {
      return platform_;
    }

    const std::string& name () const {
      return name_;
    }

    const std::vector<DeviceEntry>& devices () const {
      return devices_;
    }

    std::string list () const;

    std::string toString () const;

    std::vector<DeviceEntry> getDevices (cl_device_type type) const;
  };

  class DeviceEntry {
    friend class PlatformEntry;

    const PlatformEntry* platform_;
    size_t id_;
    cl::Device device_;
    cl_device_type type_;
    std::string name_;

    DeviceEntry (const PlatformEntry& platform, size_t id, const cl::Device& device);

  public:
    static std::string deviceTypeToString (cl_device_type ty);

    const PlatformEntry& platform () const {
      return *platform_;
    }

    size_t id () const {
      return id_;
    }

    const cl::Device& device () const {
      return device_;
    }

    cl_device_type type () const {
      return type_;
    }
  
    const std::string& name () const {
      return name_;
    }

    std::string typeString () const;

    std::string toString () const;
  };

  class PlatformList {
    std::vector<boost::shared_ptr<PlatformEntry> > platforms_;

  public:
    PlatformList (const std::vector<cl::Platform>& platforms);

    static PlatformList getPlatformList ();

    const std::vector<boost::shared_ptr<PlatformEntry> > platforms () const {
      return platforms_;
    }

    std::string list () const;

    std::vector<DeviceEntry> getDevices (cl_device_type type) const;
  };

  class DevicePattern {
    bool hasPlatform_;
    size_t platform_;

    std::vector<size_t> devices_;

    cl_device_type type_;

    bool list_;

    static bool endsWith (const std::string& s, const std::string& w);

    DevicePattern () {}

  public:
    bool hasPlatform () const {
      return hasPlatform_;
    }
    size_t platform () const {
      ASSERT (hasPlatform ());
      return platform_;
    }
    const std::vector<size_t>& devices () const {
      return devices_;
    }
    cl_device_type type () const {
      return type_;
    }
    bool list () const {
      return list_;
    }
    cl_device_type typeOrAll () const {
      return type () ? type () : CL_DEVICE_TYPE_ALL;
    }
  
    static DevicePattern parse (const std::string& str);
  };

  std::vector<DeviceEntry> getAuto (const std::vector<DeviceEntry>& devices);

  cl::Context createContext (std::string arg = "auto", const Core::OStream& out = Core::OStream::getStderr ());
}

#endif // !OPENCL_CONTEXT_HPP_INCLUDED
