#include "DataBase.hpp"

DataBase::DataBase(const char *filePath) :_filePath(filePath), _response(), _data() {}


DataBase::~DataBase() 
{
    _file.open(_filePath, std::ios::out | std::ios::trunc);
    if (!_file.is_open())
    {
        std::cerr << "open: " << std::string(strerror(errno)) << std::endl;
        exit(1);
    }
    std::vector< std::pair< std::string, std::string > >::iterator it = _data.begin();
    for (; it != _data.end(); it++)
    {
        _file << it->first << " " << it->second << std::endl;
    }
    _file.close();
    printf("data saved\n");
}

void    DataBase::load()
{
    _file.open(_filePath, std::ios::in);
    if (!_file.is_open())
    {
        throw std::runtime_error("open: " + std::string(strerror(errno)));
    }
    std::string line;
    while(std::getline(_file, line))
    {
        std::string key, value;
        std::istringstream iss;
        iss.str(line);

        iss >> key >> value;
        _data.push_back(std::make_pair(key, value));
    }

    _file.close();
    printf("data loaded\n");
}

std::vector< std::pair< std::string, std::string > >::iterator DataBase::findKey(const std::string & key)
{
    std::vector< std::pair< std::string, std::string > >::iterator it = _data.begin();
    for(; it != _data.end(); it++)
    {
        if (it->first == key)
            break;
    }
    return it;
}

void    DataBase::treatRequest(int fd, const std::string & buffer)
{
    std::istringstream iss1;
    iss1.str(buffer);

    std::string line;

    while(getline(iss1, line))
    {
        std::istringstream iss;
        iss.str(line);

        std::string command, key, value;
        iss >> command >> key >> value;

        if (command == "POST")
        {
            post(key, value);
        }
        else if (command == "GET")
        {
            get(key);
        }
        else if (command == "DEL")
        {
            del(key);
        }
        else
        {
            _response += "2\n";
        }
    }
    send(fd, _response.c_str(), _response.size(), 0);
    printf("reponse sent\n");
    _response.clear();
}

void    DataBase::post(const std::string & key, const std::string & value)
{
    std::vector< std::pair< std::string, std::string > >::iterator it = findKey(key);

    if (it != _data.end())
    {
        if (it->second != value)
        {
            it->second = value;
            _response += "0\n";
        }
    }
    else
    {
        _data.push_back(std::make_pair(key, value));
        _response += "0\n";
    }
}
void    DataBase::get(const std::string & key)
{
    std::vector< std::pair< std::string, std::string > >::iterator it = findKey(key);

    if (it != _data.end())
    {
        _response += "0 " + it->second + "\n";
    }
    else
    {
        _response += "1\n";
    }
}

void    DataBase::del(const std::string & key)
{
    std::vector< std::pair< std::string, std::string > >::iterator it = findKey(key);

    if (it != _data.end())
    {
        _data.erase(it);
        _response += "0\n";
    }
    else
    {
        _response += "1\n";
    }    
}