#include "../include/Parser.h"
#include <dirent.h>


Parser::Parser(string s) : _configfilepath(s)
{
    
}

bool Parser::_isFileOpend ()
{
    
    ifstream File (_configfilepath.c_str());

    if (!File.is_open())
    {
        std::cerr << "Failed to open file : " 
            << _configfilepath << "\n";
        return (false);
    }

    return (true);
}

string Parser::_trim(const std::string& str)
{
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, last - first + 1);
}


bool Parser::_ReadData()
{

    fstream file (_configfilepath.c_str());
        
    if (!file.is_open())
    {
        std::cerr << "Failed to open file : " 
            << _configfilepath << "\n";
        return (false);
    }

    string line;

    while (getline(file, line))
    {
        _conf_line.push_back(_trim(line));
    }

    return (true);
}

string Parser::_ReadData(string filetoread)
{
    if (filetoread.empty())
        return ("empty filename" + filetoread);

    fstream file (filetoread.c_str());
        
    if (!file.is_open())
    {
        std::cerr << "Failed to open file : " 
            << filetoread << "\n";
        return ("");
    }

    string line;
    string result = "";

    while (getline(file, line))
    {
        result += line;
    }
    return (result);
}


void Parser::v_clear()
{
    while (_conf_line.size( ) > 0)
    {
        _conf_line.pop_back();
    }
}


int _find_word( char * s, string word)
{
    if (*s == '\0' || s == NULL || word.empty())
        return (0);

    if (strncpy(s, word.c_str(), word.length() - 1))
    {
        return (word.length() - 1);
    }
    return (0);
}

std::vector<std::string> Parser::_split(const std::string& str, char delimiter)
 {
    std::vector<std::string> result;
    std::string temp;
    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == delimiter){
            if (!temp.empty())
                result.push_back(temp);
            temp.clear();
        } else {
            temp += str[i];
        }
    }
    if (!temp.empty())
        result.push_back(temp);
    return result;
}

string Parser::_string_ToUpper(string s)
{
    for (size_t i = 0 ; i < s.length(); i++)
    {
        s[i] = toupper(s[i]);
    }
    return (s);
}

bool Parser::IsLocationExtracted(string Line, Server *srv)
{
    string tmp;
  
    vector <string> v_location = _split(Line, ';');
    vector <string> v_tmp;


    for (size_t  i= 0; i < v_location.size(); i++)
    {
        if (v_location[i].find ("root") != string::npos)
        {
            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() != 2)
                return (false);
            srv->location.root = v_tmp[1];
        }
        else if (v_location[i].find ("autoindex") != string::npos)
        {
            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() != 2)
                return (false);

            v_tmp[1] = _string_ToUpper(v_tmp[1]);
            if (v_tmp[1] == "ON")
                srv->location.autoindex = true;
            else if (v_tmp[1] == "OFF")
            {
                srv->location.autoindex = false;
            }
            else 
            {
                cout << "error parsing auto index\n";
                return (false);
            }
        }
        else if (v_location[i].find ("index") != string::npos)
        {

            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() != 2)
                return (false);
            srv->location.index =  v_tmp[1];
        }
        else if (v_location[i].find ("methods") != string::npos)
        {

            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() <= 1)
                return (false);
            
            for (size_t i = 0; i < v_tmp.size(); i++)
            {
                srv->location.methods.push_back(v_tmp[i]);
            }
        }
        else
            return (false);
    }
    return (true);
}

bool Parser::IsLocationUploadExtracted(string Line, Server *srv)
{
    string tmp;
    vector <string> v_location = _split(Line, ';');
    vector <string> v_tmp;



    for (size_t  i= 0; i < v_location.size(); i++)
    {
        if (v_location[i].find ("root") != string::npos)
        {

            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() != 2)
                return (false);
            srv->location_upload.root = v_tmp[1];
        }
        else if (v_location[i].find ("autoindex") != string::npos)
        {
            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() != 2)
                return (false);

            v_tmp[1] = _string_ToUpper(v_tmp[1]);
            if (v_tmp[1] == "ON")
                srv->location_upload.autoindex = true;
            else if (v_tmp[1] == "OFF")
            {
                srv->location_upload.autoindex = false;
            }
            else 
            {
                cout << "error parsing auto index\n";
                return (false);
            }
        }
        else if (v_location[i].find ("methods") != string::npos)
        {

            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() <= 1)
                return (false);
            
            for (size_t i = 0; i < v_tmp.size(); i++)
            {
                srv->location_upload.methods.push_back(v_tmp[i]);
            }


        }
        else if (v_location[i].find ("upload_store") != string::npos)
        {
            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() != 2)
                return (false);
            srv->location_upload.upload_store =  v_tmp[1];
        }
        else
            return (false);
    }
    return (true);
}

bool Parser::IsLocationCGIExtracted(string Line, Server *srv)
{
    string tmp;
    vector <string> v_location = _split(Line, ';');
    vector <string> v_tmp;


    for (size_t  i= 0; i < v_location.size(); i++)
    {
        if (v_location[i].find ("root") != string::npos)
        {
            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() != 2)
                return (false);
            srv->cgi_bin.root = v_tmp[1];
        }
        else if (v_location[i].find ("methods") != string::npos)
        {
            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() <= 1)
                return (false);
            
            for (size_t i = 1 ; i < v_tmp.size(); i++)
            {
               
                srv->cgi_bin.methods.push_back(v_tmp[i]);
            }
        }
        else if (v_location[i].find ("cgi_pass") != string::npos)
        {
            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() != 2)
                return (false);
            srv->cgi_bin.cgi_pass =  v_tmp[1];
        }
        else
            return (false);
    }
    return (true);
}

bool Parser::IsLocationImgExtracted(string Line, Server *srv)
{
    string tmp;
    vector <string> v_location = _split(Line, ';');
    vector <string> v_tmp;


    for (size_t  i= 0; i < v_location.size(); i++)
    {
        if (v_location[i].find ("root") != string::npos)
        {
            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() != 2)
                return (false);
            srv->location_images.root = v_tmp[1];
        }
        else if (v_location[i].find ("methods") != string::npos)
        {
            v_tmp = _split(v_location[i], ' ');

            if (v_tmp.size() <= 1)
                return (false);
            
            for (size_t i = 1 ; i < v_tmp.size(); i++)
            {
               
                srv->location_images.methods.push_back(v_tmp[i]);
            }
        }
        else
            return (false);
    }
    return (true);
}



int _find_char_position(std::string s, char c)
{
    for (size_t i = 0; i < s.length(); i++)
    {
        if (s[i] == c)
            return (i);
    }
    return (-1);
}


bool isvalidnumber(string s)
{
    for (size_t i = 0; i < s.length() ; i++)
    {
        if (!isdigit(s[i]))
        {
            return (false);
        }
            
    }
    return ( true);
}

bool _isLinesSeparated(string line)
{
    short countword = 0;
    short countcoma = 0;

    bool iswordfound=false;
    bool iscomafound=false;
    
    for (size_t i = 0; i < line.length(); i++)
    {
        if (isalnum(line[i]) || isspace(line[i]) ) 
        {
            if (!iswordfound)
            {
                countword++;
              iswordfound = true;
              iscomafound = false;
            }
        }
          

        else if (line[i] == ';' && !iscomafound)
        {
            countcoma++;
            iswordfound = false;
            iscomafound = true;
        }
    }
    if (countword != countcoma)
        return (false);
    return (true);
}

bool _check_max_body_size(string line)
{   
    short count_M = 0;

    for (size_t i = 0; i < line.length(); i++)
    {
        if (!isdigit(line[i]) && line[i] != 'M' && line[i] != '\0')
            return (false);
        
        if (line[i] == 'M')
            count_M++;
        if (count_M > 1)
            return (false);
    }
    return (true);
}

bool Parser::_isserver_closed(size_t startindex, string &tmp)
{
    int i = -1;
    int j = 0;
    int tmp_j = -1;
    bool isleftfound = false;
    int tmp_index = 0;


    for (size_t index = startindex; index < _conf_line.size();index++)
    {
        if (!isleftfound)
            i = _find_char_position(_conf_line[index], '{');
        if (i != -1  && !isleftfound)
        {   
            string temp = _conf_line[index].substr(i+ 1, _conf_line[index].length());

            tmp = temp;

            _conf_line[index].substr(0, i + 1);
            _conf_line[index] = temp;
            isleftfound = true;
        }

       
        j = _find_char_position(_conf_line[index], '}');
        if (j != -1)
        {
            tmp_index = index;
            tmp_j = j;
        }
            
        if (index == _conf_line.size() - 1 && tmp_j != -1)
        {   
            string tmp = _conf_line[tmp_index].substr(0, tmp_j);
            string tmp_ = _conf_line[tmp_index].substr(tmp_j + 1, _conf_line[tmp_index].length());

            _conf_line[tmp_index] = tmp + tmp_;
        }
    }
    return (true);
}

bool Parser::_ExtractData(Server *srv)
{

    bool isserverfound = false;

    string line = "";
    string tmp =  "";
    // cout << "i extract data \n";

    for (size_t i = 0; i < _conf_line.size(); i++)
    {
        line = tmp +  _conf_line[i];

        if (!isserverfound)
        {
           if  (  _conf_line[i].find("server") != std::string::npos)
           {
                isserverfound = true;

                string temp = "";
                temp = line.substr(0, line.find("server"));
                line = line.substr(6, line.length());
                line = temp + line;

                if (!_isserver_closed(i, tmp))
                    return (false);
                i = 0;
           }
        }
        
        else if (line.find("listen") != string::npos)
        {
            
            while ((_find_char_position(line, ';') == -1))
            {

                if ( (i + 1) < _conf_line.size())
                {
                    line += _conf_line[i + 1];
                    i++;
                }
                else
                    return (false);
            }
            int tmpnb = _find_char_position(line, ';');

            tmp = &line[tmpnb + 1];
            line = line .substr(0, line.find_last_of(';'));

            line  = line.substr(_find_word((char *)line.c_str(), "listen") + 1, line.length());
            vector <string> listen = _split(line, ':');

            if (listen.size() != 2)
                return (false);

            listen[0] = _trim(listen[0]);
            listen[1] = _trim(listen[1]);

            srv->listening.ip_addr = listen[0]; // to be deleted;
            srv->listening.Port = atoi (listen[1].c_str()); // to be deleted;
        
            if (!_is_Valide_ipaddress(listen[0]))
                return (false);

            if (!_Validate_Ports(listen[1]) )
                return (false);

            Server::Listening l;

            l.ip_addr = listen[0];

            l.Port  = atoi (listen[1].c_str());
            srv->v_listening.push_back (l);

            // to be deleted;

        }
        
        
        else if (line.find("error_page") != string::npos)
        {
            while ((_find_char_position(line, ';') == -1))
            {
                if ( (i + 1) < _conf_line.size())
                {
                    line += _conf_line[i + 1];
                    i++;
                }
                else
                    return (false);
            }
            int tmpnb = _find_char_position(line, ';');
            tmp = &line[tmpnb + 1];
            line = line .substr(0, line.find_last_of(';'));

            line  = line.substr(_find_word((char *)line.c_str(), "error_page") + 1, line.length());
            vector <string> error_page = _split(line, ' ');

            Server::sterror error;

            if (error_page.size() != 1)
                return (false);

            srv->error.error.html_path = error_page[0];
        }

        else if (line.find("client_max_body_size") != string::npos)
        {
            while ((_find_char_position(line, ';') == -1))
            {
                if ( (i + 1) < _conf_line.size())
                {
                    line += _conf_line[i + 1];
                    i++;
                }
                else
                    return (false);
            }
            int tmpnb = _find_char_position(line, ';');

            tmp = &line[tmpnb + 1];
            line = line .substr(0, line.find_last_of(';'));

            line  = line.substr(_find_word((char *)line.c_str(), "client_max_body_size") + 1, line.length());

            vector <string> body_size = _split(line, ' ');
            
            if (body_size.size() != 1)
                return (false);

            _trim(body_size[0]);
            if (!_check_max_body_size(body_size[0]))
                return (false);
            srv->max_body_size = atoi(body_size[0].c_str());
        }

        else if (line.find("location /upload") != string::npos)
        {
            line  = line.substr(_find_word((char *)line.c_str(), "location /upload") + 1, line.length());

            int pos  = 0;
            while ((pos = _find_char_position(line, '{')) == -1)
            {
                if ( (i + 1) < _conf_line.size())
                {
                    line += _conf_line[i + 1];
                    i++;
                }
                else
                    return (false);
            }
            line  = line.substr(pos  + 1, line.length());
            while ((pos = _find_char_position(line, '}')) == -1)
            {
                if ( (i + 1) < _conf_line.size())
                {
                    line += _conf_line[i + 1];
                    i++;
                }
                else
                    return (false);
            }
            tmp = line.substr(pos, line.length());
            line  = line.substr(0, pos);
            _trim(line);

            if (!_isLinesSeparated(line))
                return (false);
            if (!IsLocationUploadExtracted(line, srv))
            {
                return (false);
            }
        }

         else if (line.find("location /cgi-bin") != string::npos)
        {
            line  = line.substr(_find_word((char *)line.c_str(), "location /cgi-bin") + 1, line.length());

            int pos  = 0;
            while ((pos = _find_char_position(line, '{')) == -1)
            {
                if ( (i + 1) < _conf_line.size())
                {
                    line += _conf_line[i + 1];
                    i++;
                }
                else
                    return (false);
            }
            line  = line.substr(pos  + 1, line.length());

            while ((pos = _find_char_position(line, '}')) == -1)
            {
                if ( (i + 1) < _conf_line.size())
                {
                    line += _conf_line[i + 1];
                    i++;
                }
                else
                    return (false);
            }
            tmp = line.substr(pos, line.length());
            line  = line.substr(0, pos);
            _trim(line);

            if (!_isLinesSeparated(line))
                return (false);
            if (!IsLocationCGIExtracted(line, srv))
            {
                return (false);
            }
        }

        else if (line.find("location /images") != string::npos)
        {


            line  = line.substr(_find_word((char *)line.c_str(), "location /images") + 1, line.length());

            int pos  = 0;
            while ((pos = _find_char_position(line, '{')) == -1)
            {
                if ( (i + 1) < _conf_line.size())
                {
                    line += _conf_line[i + 1];
                    i++;
                }
                else
                    return (false);
            }
            line  = line.substr(pos  + 1, line.length());

            while ((pos = _find_char_position(line, '}')) == -1)
            {
                if ( (i + 1) < _conf_line.size())
                {
                    line += _conf_line[i + 1];
                    i++;
                }
                else
                    return (false);
            }
            tmp = line.substr(pos, line.length());
            line  = line.substr(0, pos);
            _trim(line);

            if (!_isLinesSeparated(line))
                return (false);

            if (!IsLocationImgExtracted(line, srv))
                return (false);
        }

        else if (line.find("location /") != string::npos)
        {
            line  =  line.substr(_find_word((char *)line.c_str(), "location /") + 1, line.length());

            int pos  = 0;
            while ((pos = _find_char_position(line, '{')) == -1)
            {
                if ( (i + 1) < _conf_line.size())
                {
                    line += _conf_line[i + 1];
                    i++;
                }
                else
                    return (false);
            }
            line  = line.substr(pos  + 1, line.length());

            while ((pos = _find_char_position(line, '}')) == -1)
            {
                if ( (i + 1) < _conf_line.size())
                {
                    line += _conf_line[i + 1];
                    i++;
                }
                else
                    return (false);
            }
            tmp = line.substr(pos, line.length());
            line  = line.substr(0, pos);
            _trim(line);

            if (!_isLinesSeparated(line))
                return (false);

            if (!IsLocationExtracted(line, srv))
            {
                return (false);
            }
        }

        else 
        {

            for (size_t j = 0; j < line.length(); j++)
            {
                if (!isspace(_conf_line[i][j]) && _conf_line[i][j] != '{' && _conf_line[i][j] != '}'
                                    &&   _conf_line[i][j] != '\0')
                {
                    cout << "unkown config\n";
                    // cout << "[line: **" << line << "**]\n";
                    return (false);
                }
                    
            }
        }
    }
    return (true);
}

bool Parser::_is_Valide_ipaddress(string ip)
{
    if (ip.empty())
    {
        return (false);
    }



    vector <string > v_ipaddress = _split(ip, '.');

    if (v_ipaddress.size() != 4)
        return (false);

    
    for (size_t i = 0 ; i < v_ipaddress.size(); i++)
    {
        for (size_t j = 0; j < v_ipaddress[i].length(); j++ )
        {
            if (!isdigit(v_ipaddress[i][j]))
                return (false);
            
            int num = atoi (v_ipaddress[i].c_str());

            if (num > 255 || num < 0)
                return (false);
        }
    }
    return (true);
}

bool   Parser::_Validate_Ports(string s)
{
    if (!isvalidnumber(s))
        return (false);


    int num  = atoi (s.c_str());

    if (num > 65535 || num < 0)
        return (false);
    return (true);
}


bool isdirectoryopened(string path)
{
    DIR *dir = opendir (path.c_str());

    if (dir)
    {
        closedir (dir);
        return (true);
    }
    return (false);
}


bool Parser::_ValidateData(Server *srv)
{
    cout << "\n%%%%%%%%%%%%%%%%%%%%%%%%\n\n";
    cout << "validate data:\n";



    if (srv->listening.ip_addr.empty() || srv->listening.Port==-1)
    {
        cerr << "cannot read ip adress and port \n";
        return (false);
    }



     srv->error.error.html_content = _ReadData(srv->error.error.html_path);

    if (srv->error.error.html_content.empty())
    {
        cerr << "cannot read " <<srv->error.error.html_path << "\n";
        return (false);
    }

    if (!isdirectoryopened( srv->location.root))
    {
        cerr << "cannot open  default location folder\n";
        return (false);
    }

    if (!isdirectoryopened( srv->location_upload.root))
    {
        cerr << "cannot open  upload folder\n";
        return (false);
    }
        
    if (!isdirectoryopened( srv->location_images.root))
    {
        cerr << "cannot open  images folder\n";
        return (false);
    }

    srv->location.index_content = _ReadData(srv->location.root + "/" + 
           srv->location.index );

    if (srv->location.index_content.empty())
    {
        cerr << "cannot read " << srv->location.index << "\n";
        return (false);
    }

    string htmlpath =  srv->location.root + "/cgi_answer.html";
    srv->cgi_bin.htmlcontent = _ReadData  (htmlpath);


    if (srv->cgi_bin.htmlcontent.empty())
    {
        cerr << "cannot read " << srv->location.root + "/cgi_answer.html" << "\n";
        return (false);
    }


    cout << "still need validating cgi in parsing\n";
    cout << "still need validation for python || cgi_pass existence\n";
    cout << "\n%%%%%%%%%%%%%%%%%%%%%%%%\n\n";
    return (true);
}

Server *Parser::Parse()
{
    if (!_isFileOpend())
        return (NULL);

    if (!_ReadData())
        return (NULL);

    Server *srv  = new Server();

    srv->listening.ip_addr = "";
    srv->listening.Port=-1;
    srv->cgi_bin.htmlcontent = "";
    
    if (!_ExtractData(srv))
    {
        if (srv)
            delete srv;
        return (NULL); 
    }
       
    if (!_ValidateData(srv))
    {
         if (srv)
            delete srv;
        return (NULL); 
    }
    return ( srv );
}

Parser::~Parser() {}