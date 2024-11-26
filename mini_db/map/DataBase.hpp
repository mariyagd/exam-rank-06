#ifndef DATABASE_HPP
# define DATABASE_HPP

# include <iostream>
# include <fstream>
# include <string>
# include <map>
# include <sstream>
# include <istream>
# include <sys/socket.h>
# include <unistd.h>

class DataBase {
private:
	std::string							_file_path;
	std::string 						_response;
	std::fstream 						_file;
	std::map<std::string, std::string>	_data;

public:
	DataBase(const std::string & file_path);
	~DataBase();

	void	load();
	void	save();
	void	treatRequest(int fd, const std::string & request);
	void	post(const std::string & key, const std::string & value);
	void	get(const std::string & key);
	void	del(const std::string & key);
};

#endif