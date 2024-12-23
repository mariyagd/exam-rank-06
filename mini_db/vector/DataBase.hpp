#ifndef DATABASE_HPP
 # define DATABASE_HPP

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <sstream>
#include <stdio.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <cstdlib>

class DataBase
{
    private:
    std::vector< std::pair< std::string, std::string > >    _data;
    const char *                                            _filePath;
    std::fstream                                            _file;
    std::string                                             _response;

    public:
    DataBase(const char * filePath);
    ~DataBase();

    std::vector< std::pair< std::string, std::string > >::iterator     findKey(const std::string & key);
    void    load();
    void    treatRequest(const int & fd, const std::string & buffer);
    void    post(const std::string & key, const std::string & value);
    void    get(const std::string & key);
    void    del(const std::string & key);
};

#endif