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

// Tool working on HDF5 files

#include <Core/OStream.hpp>
#include <Core/BoostFilesystem.hpp>

#include <EMSim/JonesToMueller.hpp>
#include <EMSim/Hdf5ToText.hpp>
#include <EMSim/ResaveHdf5.hpp>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

using namespace EMSim;

int main (int argc, char** argv) {
  typedef boost::program_options::positional_options_description PositionalDescription;
  typedef boost::program_options::options_description Description;
  typedef boost::program_options::variables_map Map;
  typedef boost::program_options::command_line_style::style_t Style;

  const Style style = static_cast<Style> (boost::program_options::command_line_style::unix_style & ~boost::program_options::command_line_style::allow_guessing);

  PositionalDescription positional;
  positional.add ("positional-argument", -1);

  Description description;
  description.add_options ()
    ("help", "Show help")
    ("verbose", "Be verbose")

    ("output,o", boost::program_options::value<std::string> (), "Set output filename")

    ("theta-max-180", "Only output entries with theta between 0 and 180 degrees")
    ("no-phi", "Do not output phi values (for text output)")
    ("type", boost::program_options::value<std::string> (), "Overwrite 'Type' from .hdf5 file")
    ;

  Description descriptionAll = description;
  descriptionAll.add_options ()
    ("positional-argument", boost::program_options::value<std::vector<std::string> > ()->default_value (std::vector<std::string> (), ""))
    ;

  boost::program_options::variables_map map;
  try {
    boost::program_options::store (boost::program_options::command_line_parser (argc, const_cast<char**> (argv)).style (style).options (descriptionAll).positional (positional).run (), map);
  } catch (boost::program_options::error& e) {
    Core::OStream::getStderr ()
      << "Error parsing command line: " << e.what () << std::endl
      << "See 'Hdf5Util --help' for more information" << std::endl;
    return 1;
  }

  if (map.count ("help")) {
    Core::OStream::getStdout ()
      << "Usage: Hdf5Util [options] <command> <input-file.hdf5> [...]" << std::endl
      << "Valid commands:" << std::endl
      << "  hdf5-to-text      Convert .hdf5 files to .txt files" << std::endl
      << "  cat               Write content of .hdf5 files to stdout" << std::endl
      << "  resave-hdf5       Load and resave a .hdf5 file (mostly useful for debugging)" << std::endl
      << "  jones-to-mueller  Convert a .hdf5 jones matrix file to a mueller matrix file" << std::endl
      << "Valid options:" << std::endl
      << description;
    return 0;
  }

  std::vector<std::string> args = map["positional-argument"].as<std::vector<std::string> > ();
  if (args.size () == 0) {
    Core::OStream::getStderr ()
      << "Error: No command given" << std::endl
      << "See 'Hdf5Util --help' for more information" << std::endl;
    return 1;
  }
  std::string command = args[0];
  args.erase (args.begin ());

  if (args.size () == 0) {
    Core::OStream::getStderr ()
      << "Error: No input files given" << std::endl
      << "See 'Hdf5Util --help' for more information" << std::endl;
  }

  if (args.size () > 1 && map.count ("output")) {
    Core::OStream::getStderr ()
      << "Error: Got multiple input files and a --output option" << std::endl;
    return 1;
  }

  bool verbose = map.count ("verbose");

  boost::optional<std::string> typeOverwrite;
  if (map.count ("type"))
    typeOverwrite = map["type"].as<std::string> ();

  if (command == "hdf5-to-text") {
    BOOST_FOREACH (const std::string& inputStr, args) {
      boost::filesystem::path input = inputStr;
      std::string outputName;
      StreamOrPathOutput output;
      if (map.count ("output")) {
        outputName = map["output"].as<std::string> ();
        if (outputName == "-" && 0) {
          output = Core::OStream::getStdout ();
          outputName = "<stdout>";
        } else {
          output = outputName;
        }
      } else {
        boost::filesystem::path out = input;
        out.replace_extension (".txt");
        output = out;
        outputName = out.BOOST_FILE_STRING;
      }
      if (verbose)
        Core::OStream::getStderr ().fprintf ("Converting '%s' to '%s' ...", input, outputName);
      hdf5ToText (input, output, typeOverwrite, map.count ("theta-max-180"), map.count ("no-phi"));
      if (verbose)
        Core::OStream::getStderr ().fprintf (" done\n");
    }
  } else if (command == "cat") {
    BOOST_FOREACH (const std::string& inputStr, args) {
      boost::filesystem::path input = inputStr;
      if (verbose)
        Core::OStream::getStderr ().fprintf ("Showing '%s':\n", input);
      hdf5ToText (input, Core::OStream::getStdout (), typeOverwrite, map.count ("theta-max-180"), map.count ("no-phi"));
    }
  } else if (command == "resave-hdf5") {
    BOOST_FOREACH (const std::string& inputStr, args) {
      boost::filesystem::path input = inputStr;
      boost::filesystem::path output;
      if (map.count ("output")) {
        output = map["output"].as<std::string> ();
      } else {
        output = input;
        output.replace_extension (".new.hdf5");
      }
      if (verbose)
        Core::OStream::getStderr ().fprintf ("Resaving '%s' as '%s' ...", input, output);
      resaveHdf5 (input, output, typeOverwrite, map.count ("theta-max-180"));
      if (verbose)
        Core::OStream::getStderr ().fprintf (" done\n");
    }
  } else if (command == "jones-to-mueller") {
    BOOST_FOREACH (const std::string& inputStr, args) {
      boost::filesystem::path input = inputStr;
      boost::filesystem::path output;
      if (map.count ("output")) {
        output = map["output"].as<std::string> ();
      } else {
        output = input;
        output.replace_extension (".mueller.hdf5");
      }
      if (verbose)
        Core::OStream::getStderr ().fprintf ("Converting jones matrix in '%s' to mueller matrix in '%s' ...", input, output);
      jonesToMueller (input, output, map.count ("theta-max-180"));
      if (verbose)
        Core::OStream::getStderr ().fprintf (" done\n");
    }
  } else {
    Core::OStream::getStderr ()
      << "Error: Unknown command: " << command << std::endl
      << "See 'Hdf5Util --help' for more information" << std::endl;
    return 1;
  }
}
