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

// Show the filters for all datasets in a HDF5 file

#include <Core/OStream.hpp>

#include <HDF5/File.hpp>
#include <HDF5/DataSet.hpp>
#include <HDF5/Group.hpp>
#include <HDF5/PropLists.hpp>

#include <boost/foreach.hpp>

static void indent (int indentLevel = 0) {
  for (int i = 0; i < indentLevel; i++)
    Core::OStream::getStdout () << "  ";
}

static void show (const HDF5::Object& obj, int indentLevel = 0) {
  if (obj.getType () == H5I_DATASET) {
    indent (indentLevel);
    Core::OStream::getStdout () << "DATASET" << std::endl;
    HDF5::DataSet ds = (HDF5::DataSet) obj;
    HDF5::DataSetCreatePropList cp = ds.createPropList ();
    HDF5::DataSetAccessPropList ap = ds.accessPropList ();
    int nfilter = HDF5::Exception::check ("H5Pget_nfilters", H5Pget_nfilters (cp.handle ()));
    indent (indentLevel);
    Core::OStream::getStdout ().fprintf ("filters: %s\n", nfilter);
    for (int j = 0; j < nfilter; j++) {
      unsigned int flags;
      unsigned int filter_config;
      size_t cd_nelemts = 0;
      char name[1024];
      H5Z_filter_t filter = HDF5::Exception::check ("H5Pget_filter2", H5Pget_filter2 (cp.handle (), j, &flags, &cd_nelemts, NULL, sizeof (name) / sizeof (*name), name, &filter_config));
      std::vector<unsigned int> cd_values (cd_nelemts);
      filter = HDF5::Exception::check ("H5Pget_filter2", H5Pget_filter2 (cp.handle (), j, &flags, &cd_nelemts, cd_values.data (), sizeof (name) / sizeof (*name), name, &filter_config));
      indent (indentLevel + 1);
      Core::OStream::getStdout ().fprintf ("filter '%s' filter=%s flags=%s config=%s\n", name, filter, flags, filter_config);
      BOOST_FOREACH (unsigned int value, cd_values) {
        indent (indentLevel + 2);
        Core::OStream::getStdout ().fprintf ("parameter %s\n", value);
      }
    }
  } else if (obj.getType () == H5I_GROUP) {
    indent (indentLevel);
    Core::OStream::getStdout () << "GROUP" << std::endl;
    HDF5::Group group = (HDF5::Group) obj;
    BOOST_FOREACH (const std::string& s, group.list ()) {
      indent (indentLevel);
      Core::OStream::getStdout () << s << std::endl;
      show (group.open (s), indentLevel + 1);
    }
  }
}

int main (int argc, char** argv) {
  if (argc < 2) {
    Core::OStream::getStderr () << "Usage: ShowFilters file.hdf5 [...]" << std::endl;
    return 1;
  }

  for (int i = 1; i < argc; i++) {
    Core::OStream::getStdout () << argv[i] << std::endl;
    HDF5::File file = HDF5::File::open (argv[i], H5F_ACC_RDONLY);
    show (file.rootGroup (), 1);
  }
  
  return 0;
}
