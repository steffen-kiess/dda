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

#include "Context.hpp"

#include <Core/HelpResultException.hpp>

#include <sstream>

#include <boost/lexical_cast.hpp>

namespace OpenCL {
  std::string PlatformEntry::toString () const {
    std::stringstream str;
    str << id () << " `" << name () << "'";
    return str.str ();
  }

  DeviceEntry::DeviceEntry (const PlatformEntry& platform, size_t id, const cl::Device& device) : platform_ (&platform), id_ (id), device_ (device) {
    type_ = device_.getInfo<CL_DEVICE_TYPE> ();
    name_ = device_.getInfo<CL_DEVICE_NAME> ();
  }

  std::string DeviceEntry::deviceTypeToString (cl_device_type ty) {
    ASSERT (ty != 0);
    ASSERT ((ty | CL_DEVICE_TYPE_ALL) == CL_DEVICE_TYPE_ALL);

    std::stringstream str;
#define D(x) do { if ((ty & CL_DEVICE_TYPE_##x) != 0) { if (str.str () != "") str << "|"; str << #x; } ty &= ~static_cast<cl_device_type> (CL_DEVICE_TYPE_##x); } while (0)
    D (DEFAULT);
    D (CPU);
    D (GPU);
    D (ACCELERATOR);
#undef D
    if (ty != 0) {
      if (str.str () != "")
        str << "|";
      str.setf (std::ios::hex, std::ios::basefield);
      str << "0x" << ty;
    }
    return str.str ();
  }

  std::string DeviceEntry::typeString () const {
    return deviceTypeToString (type ());
  }

  std::string DeviceEntry::toString () const {
    std::stringstream str;
    str << platform ().id () << "." << id () << " " << typeString () << " `" << name () << "'";
    return str.str ();
  }

  PlatformEntry::PlatformEntry (size_t id, const cl::Platform& platform) : id_ (id), platform_ (platform) {
    name_ = platform_.getInfo<CL_PLATFORM_NAME> ();

    std::vector<cl::Device> devs;
    platform_.getDevices (CL_DEVICE_TYPE_ALL, &devs);

    /*
      static std::vector<cl::Device> devs2;
      devs2.insert (devs2.begin (), devs.begin (), devs.end ());
      devs = devs2;
    */

    for (size_t i = 0; i < devs.size (); i++)
      devices_.push_back (DeviceEntry (*this, i, devs[i]));
  }

  std::string PlatformEntry::list () const {
    std::stringstream str;
    str << toString () << std::endl;
    for (size_t i = 0; i < devices ().size (); i++)
      str << " " << devices ()[i].toString () << std::endl;
    return str.str ();
  }

  std::vector<DeviceEntry> PlatformEntry::getDevices (cl_device_type type) const {
    std::vector<DeviceEntry> result;
    for (size_t i = 0; i < devices ().size (); i++)
      if ((devices ()[i].type () & type) != 0)
        result.push_back (devices ()[i]);
    return result;
  }

  PlatformList::PlatformList (const std::vector<cl::Platform>& platforms) {
    for (size_t i = 0; i < platforms.size (); i++) {
      platforms_.push_back (boost::shared_ptr<PlatformEntry> (new PlatformEntry (i, platforms[i])));
    }
  }

  PlatformList PlatformList::getPlatformList () {
    std::vector<cl::Platform> platforms;
    cl::Platform::get (&platforms);
    return PlatformList (platforms);
  }

  std::string PlatformList::list () const {
    std::stringstream str;
    for (size_t i = 0; i < platforms ().size (); i++)
      str << platforms ()[i]->list ();
    return str.str ();
  }

  std::vector<DeviceEntry> PlatformList::getDevices (cl_device_type type) const {
    std::vector<DeviceEntry> result;
    for (size_t i = 0; i < platforms ().size (); i++) {
      std::vector<DeviceEntry> d = platforms ()[i]->getDevices (type);
      result.insert (result.end (), d.begin (), d.end ());
    }
    return result;
  }

  bool DevicePattern::endsWith (const std::string& s, const std::string& w) {
    return s.length () >= w.length () && s.substr (s.length () - w.length ()) == w;
  }

  DevicePattern DevicePattern::parse (const std::string& str) {
    DevicePattern r;
    std::string s = str;

    r.list_ = false;
    if (endsWith (s, ":list")) {
      s = s.substr (0, s.length () - strlen (":list"));
      r.list_ = true;
    } else if (s == "list") {
      s = "all";
      r.list_ = true;
    }

    size_t dotpos = s.find (".");
    if (dotpos == std::string::npos) {
      if (s.length () > 0 && s[0] >= '0' && s[0] <= '9') {
        r.hasPlatform_ = true;
        r.platform_ = boost::lexical_cast<size_t> (s);
        r.type_ = CL_DEVICE_TYPE_ALL;
        return r;
      } else {
        r.hasPlatform_ = false;
      }
    } else {
      r.hasPlatform_ = true;
      r.platform_ = boost::lexical_cast<size_t> (s.substr (0, dotpos));
      s = s.substr (dotpos + 1);
    }

    if (s.length () > 0 && s[0] >= '0' && s[0] <= '9') {
      r.type_ = 0;

      size_t pos = 0;
      size_t pos_old = 0;
      s += ",";
      while ((pos = s.find (",", pos)) != std::string::npos) {
        r.devices_.push_back (boost::lexical_cast<size_t> (s.substr (pos_old, pos - pos_old)));
        pos = pos + 1;
        pos_old = pos;
      }
    } else {
      if (s == "all") {
        r.type_ = CL_DEVICE_TYPE_ALL;
        return r;
      }

      if (s == "auto") {
        r.type_ = 0;
        return r;
      }

      r.type_ = 0;
      size_t pos = 0;
      size_t pos_old = 0;
      s += ",";
      while ((pos = s.find (",", pos)) != std::string::npos) {
        std::string t = s.substr (pos_old, pos - pos_old);
        if (t == "cpu") {
          r.type_ |= CL_DEVICE_TYPE_CPU;
        } else if (t == "gpu") {
          r.type_ |= CL_DEVICE_TYPE_GPU;
        } else if (t == "accelerator") {
          r.type_ |= CL_DEVICE_TYPE_ACCELERATOR;
        } else {
          ABORT_MSG ("Unknown device type `" + t + "'");
        }
        pos = pos + 1;
        pos_old = pos;
      }
    }

    return r;
  }

  std::vector<DeviceEntry> getAuto (const std::vector<DeviceEntry>& devices) {
    std::vector<DeviceEntry> result;
  
    for (size_t j = 0; j < devices.size (); j++) {
      const DeviceEntry& dev = devices[j];
      if ((dev.type () & CL_DEVICE_TYPE_DEFAULT) != 0) {
        result.push_back (dev);
        return result;
      }
    }
    for (size_t j = 0; j < devices.size (); j++) {
      const DeviceEntry& dev = devices[j];
      if ((dev.type () & CL_DEVICE_TYPE_GPU) != 0) {
        result.push_back (dev);
        return result;
      }
    }
    for (size_t j = 0; j < devices.size (); j++) {
      const DeviceEntry& dev = devices[j];
      if ((dev.type () & CL_DEVICE_TYPE_ACCELERATOR) != 0) {
        result.push_back (dev);
        return result;
      }
    }
    for (size_t j = 0; j < devices.size (); j++) {
      const DeviceEntry& dev = devices[j];
      if ((dev.type () & CL_DEVICE_TYPE_CPU) != 0) {
        result.push_back (dev);
        return result;
      }
    }
    if (devices.size () > 0) {
      result.push_back (devices[0]);
      return result;
    }
    return result;
  }

  cl::Context createContext (std::string arg, const Core::OStream& out) {
    DevicePattern pattern = DevicePattern::parse (arg);
    PlatformList list = PlatformList::getPlatformList ();

    std::vector<DeviceEntry> devices;
    if (pattern.hasPlatform ()) {
      if (pattern.platform () >= list.platforms ().size ())
        ABORT_MSG ("Invalid platform id " + boost::lexical_cast<std::string> (pattern.platform ()));
      devices = list.platforms ()[pattern.platform ()]->getDevices (pattern.typeOrAll ());
    } else {
      devices = list.getDevices (pattern.typeOrAll ());
    }

    if (pattern.devices ().size () > 0) {
      std::vector<DeviceEntry> devices2;
      size_t platform = pattern.platform ();
      for (size_t i = 0; i < pattern.devices ().size (); i++) {
        size_t devid = pattern.devices ()[i];
        bool found = false;
        for (size_t j = 0; j < devices.size () && !found; j++) {
          ASSERT (devices[j].platform ().id () == platform);
          if (devices[j].id () == devid) {
            found = true;
            devices2.push_back (devices[j]);
          }
        }
        if (!found)
          ABORT_MSG ("Invalid device id " + boost::lexical_cast<std::string> (devid) + " for platform id " + boost::lexical_cast<std::string> (platform));
      }
      devices = devices2;
    } else if (pattern.type () == 0) {
      devices = getAuto (devices);
    } else {
    }

    if (pattern.list ()) {
      std::stringstream str;
      size_t lastPl = (size_t) (ssize_t) -1;
      for (size_t i = 0; i < devices.size (); i++) {
        const DeviceEntry& dev = devices[i];
        if (dev.platform ().id () != lastPl) {
          lastPl = dev.platform ().id ();
          str << dev.platform ().toString () << std::endl;
        }
        str << " " << dev.toString () << std::endl;
      }
      throw Core::HelpResultException (str.str ());
    }

    if (pattern.devices ().size () == 0 && devices.size () > 0) {
      devices.erase (devices.begin () + 1, devices.end ());
    }

    if (devices.size () == 0)
      ABORT_MSG ("No matching device found");

    if (devices.size () == 1) {
      out << "Using device " << devices[0].toString () << std::endl;
    } else {
      out << "Using devices:" << std::endl;
      for (size_t i = 0; i < devices.size (); i++)
        out << " " << devices[i].toString () << std::endl;
    }

    std::vector<cl::Device> clDevs;
    for (size_t i = 0; i < devices.size (); i++)
      clDevs.push_back (devices[i].device ());

    cl::Context context (clDevs);
    return context;
  }
}
