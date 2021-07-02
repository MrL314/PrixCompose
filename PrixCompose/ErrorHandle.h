
#include <iostream>
#include <exception>
#include <string>
#include <cstring>

#include <sstream>

#ifndef ERRORHANDLE_H
#define ERRORHANDLE_H

class PrixException : virtual public std::runtime_error {
protected:
	std::string file;
	int line;
	std::string func;
	std::string info;
	std::string msg;
	std::string error_msg;

public:
	explicit PrixException(const std::string& msg_, int line_ = -1, const std::string& info_ = "", const std::string& file_ = "", const std::string& func_ = "") : std::runtime_error("")
	{
		line = line_;
		info = info_;
		file = file_;
		func = func_;
		msg = msg_;
	
		std::stringstream ss;
		if (line_ != -1)
		{
			if (strcmp(file_.c_str(), "") != 0)
			{
				ss << "Error in " << file_ << " at line " << line_ << ": " << std::endl;
			}
			else
			{
				ss << "Error at line " << line_ << ": " << std::endl;
			}
		}

		ss << "  " << msg_ << std::endl;

		if (strcmp(info_.c_str(), "") != 0)
		{
			ss << "    " << info_ << std::endl;
		}

		error_msg = ss.str();

	}





	std::string get_file() const { return file; }
	int get_line() const { return line; }
	std::string get_func() const { return func; }
	std::string get_info() const { return info; }
	std::string get_msg() const { return msg; }

	virtual const char* what() const noexcept
	{
		return error_msg.c_str();
	}

	virtual ~PrixException() noexcept {};
};

#endif