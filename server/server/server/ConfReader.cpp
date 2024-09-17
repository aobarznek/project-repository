#include "ConfReader.h"
#include "sqlite3.h"
#include <iostream>
#include <vector>
#include <string>
//#pragma comment(lib, "C:\Praktyki\klient serwer\server\server\x64\Debug\\sqlite3.lib")
using namespace std;


using Record = std::vector<std::string>;
using Records = std::vector<Record>;

sqlite3* db;

int  select_callback(void* p_data, int num_fields, char** p_fields, char** p_col_names)
{
	Records* records = static_cast<Records*>(p_data);
	try {
		records->emplace_back(p_fields, p_fields + num_fields);
	}
	catch (...) {
		// abort select on failure, don't let exception propogate thru sqlite3 call-stack
		return 1;
	}
	return 0;
}

Records  select_stmt(const char* stmt)	
{
	Records records;
	char* errmsg;
	int ret = sqlite3_exec(db, stmt, select_callback, &records, &errmsg);
	if (ret != SQLITE_OK) {
		std::cerr << "Blad w instrukcji Select" << stmt << "[" << errmsg << "]\n";
	}
	return records;
}

void sql_stmt(const char* stmt)
{
	char* errmsg;
	int ret = sqlite3_exec(db, stmt, 0, 0, &errmsg);
	if (ret != SQLITE_OK) {
		std::cerr << "Blad w instrukcji Select" << stmt << "[" << errmsg << "]\n";
	}
}
ConfReader::ConfReader()
{

}
void ConfReader::load()
{
	const int STATEMENTS = 1;


	int rc;

	rc = sqlite3_open("bd.db", &db);

	if (rc)
	{
		cout << "Nie mozna otworztc BD: " << sqlite3_errmsg(db) << "\n";
	}
	else
	{
		cout << "Otwarto baze danych\n\n";
	}

	Records records = select_stmt("select  ip , port  from configuration");
	std::string addrIpLocal = records[0][0];
	size_t length = addrIpLocal.size();
	strcpy_s(addrIp, addrIpLocal.c_str());

	port = stoi(records[0][1]);
	sqlite3_close(db);

}
