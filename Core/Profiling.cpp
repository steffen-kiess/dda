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

#include "Profiling.hpp"

#include <sstream>

namespace Core {
  std::string ProfilingTimes::toString () const {
    std::stringstream result;

    result.setf (std::ios_base::internal, std::ios_base::adjustfield);
    result.setf (std::ios_base::fixed, std::ios_base::floatfield);
    result.setf (std::ios_base::showpoint);

    result.width (10);
    result.precision (3);
    result << real.getMilliseconds () << " ms, ";

    result.width (10);
    result.precision (3);
    result << user.getMilliseconds () + system.getMilliseconds () << " ms";

    return result.str ();
  }

  namespace {
    void wrEntry (std::stringstream& result, const std::string& name, size_t maxlen, ProfilingTimes values, ProfilingTimes overalls) {

      result.width (maxlen);
      result.setf (std::ios_base::left, std::ios_base::adjustfield);
      result << name << " ";

      result.setf (std::ios_base::internal, std::ios_base::adjustfield);
      result.setf (std::ios_base::fixed, std::ios_base::floatfield);
      result.setf (std::ios_base::showpoint);

      double value;
      double overall;

      value = values.real.getMilliseconds ();
      overall = overalls.real.getMilliseconds ();
      result.width (10);
      result.precision (3);
      result << value << " ms = ";
      result.width (5);
      result.precision (1);
      result << 100.0f * value / overall << "%";

      result << " ";

      value = values.user.getMilliseconds () + values.system.getMilliseconds ();
      overall = overalls.user.getMilliseconds () + overalls.system.getMilliseconds ();
      result.width (10);
      result.precision (3);
      result << value << " ms = ";
      result.width (5);
      result.precision (1);
      result << 100.0f * value / overall << "%";

      result << " ";

      /*
      value = values.user.getMilliseconds ();
      overall = overalls.user.getMilliseconds ();
      result.width (10);
      result.precision (3);
      result << value << " ms = ";
      result.width (5);
      result.precision (1);
      result << 100.0f * value / overall << "%";

      result << " ";
      
      value = values.system.getMilliseconds ();
      overall = overalls.system.getMilliseconds ();
      result.width (10);
      result.precision (3);
      result << value << " ms = ";
      result.width (5);
      result.precision (1);
      result << 100.0f * value / overall << "%";
      */
      result << std::endl;
    }
  }

#if USE_PROFILING
  std::string ProfilingData::doToString () {
    if (!enabled) 
      return "Profiling disabled";

    std::stringstream result;
    
    std::string o = "overall";
    
    size_t maxlen = o.length ();
    ProfilingTimes sum = ProfilingTimes ();

    std::map<std::string, ProfilingTimes> s;
    
    for (uint32_t i = 0; i < lastnr; i++)
      s[namesR[i]] = times[i];

    for (uint32_t i = 0; i < lastnr; i++) {
      maxlen = std::max<size_t> (maxlen, namesR[i].length ());
      sum += times[i];
      for (uint32_t j = 0; j < lastnr; j++)
        if (namesR[i].substr (0, namesR[j].length () + 1) == namesR[j] + ".")
          s[namesR[j]] += times[i];
    }
    
    maxlen++;

    for (uint32_t i = 0; i < lastnr; i++) {
      std::string name = namesR[i];
      ProfilingTimes value = times[i];
      wrEntry (result, name, maxlen, value, sum);
    }

    result << "====" << std::endl;

    for (uint32_t i = 0; i < lastnr; i++) {
      std::string name = namesR[i];
      ProfilingTimes value = s[name];
      wrEntry (result, "=" + name, maxlen, value, sum);
    }
    
    wrEntry (result, o, maxlen, sum, sum);
    
    return result.str ();
  }
#endif

  ProfilingData ProfilingDataPtr::unused (false);

  ProfilingTimes ProfilingData::getOverallTimes () {
#if USE_PROFILING
    ProfilingTimes sum = ProfilingTimes ();

    for (uint32_t i = 0; i < lastnr; i++)
      sum += times[i];

    return sum;
#else
    return ProfilingTimes (TimeSpan (-1), TimeSpan (-1), TimeSpan (-1));
#endif
  }
}
