#include <iostream>
#include "include/Parser.h"


using namespace std;


int main (int argc, char **argv)
{
    if (argc != 2)
    {
        cout << "Wrong Arguments : ./server <conf_file>\n";
        return (1);
    }


    Parser parser (argv[1]);
    
    Server *srv = parser.Parse();


    if (srv == NULL)
    {
        std::cerr << "error parsing Config File\n";
        return (1);
    }

    if (srv)
    {
        delete srv;
    }


    cout << "starting server setup" << endl;
    
    (void)argc;
    (void)argv;
    return (0);
}