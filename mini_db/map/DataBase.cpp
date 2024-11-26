#include "DataBase.hpp"

DataBase::DataBase(const std::string & file_path) : _file_path(file_path), _response(), _data() {
}

DataBase::~DataBase() {
	save();
}

void DataBase::load() {

	_file.open(_file_path, std::ios::in);
	if (!_file.is_open())
	{
		throw std::runtime_error("open: " + std::string(strerror(errno)));
	}

	std::string line;
	while (std::getline(_file, line))
	{
		printf("line: %s\n", line.c_str());
		std::istringstream iss;
		iss.str(line);
		std::string key;
		std::string value;
		iss >> key >> value;
		_data.insert(std::pair<std::string, std::string>(key, value));
	}
	printf("Data loaded\n");
	_file.close();
	printf("File closed\n");
}

void DataBase::save() {

	if (_data.empty())
	{
		printf("No data to save\n");
		return;
	}
	_file.open(_file_path, std::ios::out | std::ios::trunc);
	if (!_file.is_open())
	{
		throw std::runtime_error("Failed to open file");
	}
	std::map<std::string, std::string>::const_iterator it = _data.begin();
	for (; it != _data.end(); it++)
	{
		printf("Saving: %s %s\n", it->first.c_str(), it->second.c_str());
		_file << it->first << " " << it->second << std::endl;
	}
	_file.close();
	printf("Data saved\n");
	printf("File closed\n");
}

void	DataBase::treatRequest(int fd, const std::string & request)
{
		std::istringstream iss;
		iss.str(request);

		std::string line;
		while (std::getline(iss, line, '\n'))
		{
			std::istringstream iss2;
			iss2.str(line);
			while ( !iss2.eof() )
			{
				std::string command;
				std::string key;
				std::string value;
				iss2 >> command >> key;
				printf("command: %s\n", command.c_str());
				printf("key: %s\n", key.c_str());
				if (command == "POST")
				{
					iss2 >> value;
					printf("value: %s\n", value.c_str());
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
		}
		send(fd, _response.c_str(), _response.size(), 0);
		_response.clear();
}

void DataBase::post(const std::string & key, const std::string & value) {

	std::map<std::string, std::string>::iterator it = _data.find(key);
	if (it != _data.end())
	{
		if (it->second != value)
		{
			printf("Post existing key\n");
			it->second  = value;
			_response += "0\n";
			return;
		}
		printf("Existing key with same value\n");
	}
	else
	{
		printf("Post new key\n");
		_data.insert(std::pair<std::string, std::string>(key, value));
		_response += "0\n";
		return;
	}
}

void DataBase::get(const std::string & key) {

	std::map<std::string, std::string>::iterator it = _data.find(key);
	if (it != _data.end())
	{
		_response += "0 " + it->second + "\n";
	}
	else
	{
		_response += "1\n";
	}
}

void DataBase::del(const std::string & key) {

	std::map<std::string, std::string>::iterator it = _data.find(key);
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
