#ifndef DATABASE_HPP
 #define DATABASE_HPP

#include<string>
#include<fstream>
#include<sstream>
#include<vector>
#include <string.h>
#include <errno.h>
#include<exception>
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>

class DataBase
{
    private:
    const char                                              *_filePath;
    std::fstream                                            _file;
    std::string                                             _response;
    std::vector< std::pair< std::string, std::string > >    _data;


    public:
    DataBase(const char *filePath);
    ~DataBase();

    std::vector< std::pair< std::string, std::string > >::iterator findKey(const std::string & key);
    void    load();
    void    treatRequest(int fd, const std::string & buffer);
    void    post(const std::string & key, const std::string & value);
    void    get(const std::string & key);
    void    del(const std::string & key);
};

#endif