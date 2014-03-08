// -*- C++ -*-
#ifndef _CONFIGPARSER_HPP_
#define _CONFIGPARSER_HPP_

///
/// @brief Simple Configuration File Parser
///
/// $Id$
///
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <map>
#include <regex.h>

//
// Simple Configuration File Parser
//
class ConfigParser
{
private:
  std::map<std::string, std::string> m_pair;

  // read config file and store (key, val) to pair to m_pair
  void read_file(const char* filename)
  {
    static const int max_num_match = 5;
    static const char pattern[] =
      "[[:space:]]*([A-Za-z0-9_\\.\\+\\-]+)[[:space:]]*="
      "[[:space:]]*([A-Za-z0-9_\\.\\+\\-]+)[[:space:]]*";

    int status;
    std::string line;

    // open file
    std::ifstream ifs(filename);

    // prepare for regex match
    regex_t buf;
    regmatch_t match[max_num_match];

    status = regcomp(&buf, pattern, REG_EXTENDED);
    if(status != 0) {
      std::cerr << "Error: regcomp failed" << std::endl;
      exit(-1);
    }

    // parse for each line
    while( std::getline(ifs, line) ) {
      line = discard_comment(line);
      parse_line(line, &buf, match, max_num_match);
    }
  }

  // discard comment (after "#" for each line)
  std::string discard_comment(std::string &str)
  {
    std::string::size_type last = str.find_first_of("#");
    return str.substr(0, last);
  }

  // perform regex match and store (key, val) pair
  void parse_line(std::string &str, regex_t *buf, regmatch_t *match, size_t n)
  {
    int status = regexec(buf, str.c_str(), n, match, 0);

    // ignore unrecognized line
    if(status != 0 || n < 3) {
      return;
    }

    // insert into m_pair
    std::string key = trim(str.substr(match[1].rm_so, match[1].rm_eo));
    std::string val = trim(str.substr(match[2].rm_so, match[2].rm_eo));
    m_pair.insert(std::make_pair(key, val));
  }

  // trim white space
  std::string trim(std::string str)
  {
    std::string::size_type first = str.find_first_not_of(" ");
    std::string::size_type last  = str.find_last_not_of (" ");

    return str.substr(first, last+1);
  }

  // template declaration
  template <class T> T type_cast(std::string &str);

public:
  // constructor: read given file
  ConfigParser(const char* filename)
  {
    read_file(filename);
  }

  // get value for given key
  template <class T>
  T getAs(const char* key)
  {
    return type_cast<T>(m_pair[key]);
  }

  // print all (key, val) pairs
  void print()
  {
    std::map<std::string, std::string>::iterator it = m_pair.begin();

    std::cout << std::setw(20) << std::left << "key"
              << std::setw( 2) << ":"
              << std::setw(20) << std::left << "val" << std::endl;
    std::cout << std::setw(42) << std::setfill('-') << "" << std::endl;
    std::cout << std::setfill(' ');
    while( it != m_pair.end() ) {
      std::cout << std::setw(20) << std::left << (*it).first
                << std::setw( 2) << ":"
                << std::setw(20) << std::left << (*it).second << std::endl;
      ++it;
    }
  }
};

// for string
template <>
std::string ConfigParser::type_cast<std::string>(std::string &str)
{
  return str;
}

// for int
template <>
int ConfigParser::type_cast<int>(std::string &str)
{
  return std::atoi(str.c_str());
}

// for float
template <>
float ConfigParser::type_cast<float>(std::string &str)
{
  return std::atof(str.c_str());
}

// for double
template <>
double ConfigParser::type_cast<double>(std::string &str)
{
  return std::atof(str.c_str());
}

// Local Variables:
// c-file-style   : "gnu"
// c-file-offsets : ((innamespace . 0) (inline-open . 0))
// End:
#endif
