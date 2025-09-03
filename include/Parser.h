#ifndef PARSER_H
#define PARSER_H

#include <iostream>
#include "Server.h"
#include <fstream>
#include <vector>
#include <cstring>
#include <cstdlib>

using namespace std;

class Parser
{
    private : 

      const     string _configfilepath;
      bool      _isFileOpend();
      // ifstream  *_file;
      bool _ReadData();
      string  _ReadData(string s);
      bool _ValidateData(Server *srv);
      bool _ExtractData(Server *srv);
      vector <string> _conf_line;
      string _trim(const std::string& str);
      bool IsLocationExtracted(string line, Server *srv);
      bool IsLocationUploadExtracted(string Line, Server *srv);
      bool IsLocationCGIExtracted(string Line, Server *srv);
      string _string_ToUpper(string s);
     bool  _is_Valide_ipaddress(Server *srv);
     bool   _Validate_Ports(Server *srv, string s);
     bool _isserver_closed(size_t startindex, string &tmp);
     

    public :
      std::vector<std::string> _split(const std::string& str, char delimiter);
      Parser (string s);
      Server *Parse ();
      void v_clear();
      ~Parser ();
};


#endif