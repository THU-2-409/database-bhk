#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

class Database {
public:
	Database();
	~Database();
	bool createDatabase(string DBname);
	bool dropDatabase(string DBname);
	void useDatabase(string DBname);
private:
	string currentDBname;
};

bool Database::createDatabase(string DBname) {
	//判断文件夹是否存在
	if(access(DBname.c_str()) == 0)
		return false;

	//创建文件夹
	if(mkdir(DBname.c_str()) == 0)
		return true;
	else
		return false;
}

bool Database::dropDatabase(string DBname) {
	//判断文件夹是否存在
	if(access(DBname.c_str()) == 0)
		return false;

	//删除文件夹
	if(rmdir(DBname.c_str()) == 0)
		return true;
	else
		return false;
}

void Database::useDatabase(string DBname) {
	currentDBname = DBname;
}

#endif