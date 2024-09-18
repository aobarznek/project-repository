#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <sqlite3.h>
#include <iostream>
#include <winsock2.h>
#include <string>
#include <fstream>
#include "ConfReader.h"
#pragma comment(lib, "ws2_32.lib")

using namespace std;

void rejestracja(SOCKET kgniazdo) 
{
    char buffer[1024];
    int recvSize = recv(kgniazdo, buffer, sizeof(buffer), 0);
    if (recvSize == SOCKET_ERROR) {
        int error = WSAGetLastError();
        cout << "Blad odbierania wiadomosci: " << error << endl;
        send(kgniazdo, "Blad odbierania wiadomosci", strlen("Blad odbierania wiadomosci"), 0);
        return;
    }

    buffer[recvSize] = '\0';

 
    sqlite3* db;
    int rc = sqlite3_open("bd.db", &db);
    if (rc != SQLITE_OK) 
    {
        cout << "Nie mozna otworzyc bazy: " << sqlite3_errmsg(db) << endl;
        send(kgniazdo, "Blad bazy danych", strlen("Blad bazy danych"), 0);
        return;
    }

    const char* sql = "INSERT INTO users (nazwa) VALUES (?)";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        cout << "Blad przygotowania zapytania: " << sqlite3_errmsg(db) << endl;
        send(kgniazdo, "Blad przygotowania zapytania", strlen("Blad przygotowania zapytania"), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, buffer, -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) 
    {
        cout << "Blad wykonywania zapytania: " << sqlite3_errmsg(db) << endl;
        send(kgniazdo, "Blad wykonywania zapytania", strlen("Blad wykonywania zapytania"), 0);
    }
    else 
    {
        cout << "Nazwa uzytkownika zostala zapisana." << endl;
        send(kgniazdo, "Rejestracja sie powiodla", strlen("Rejestracja sie powiodla"), 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void listauzytkownikow(SOCKET kgniazdo) 
{
    sqlite3* db;
    sqlite3_stmt* stmt;
    string lista = "";

    int rc = sqlite3_open("bd.db", &db);
    if (rc != SQLITE_OK) 
    {
        cout << "Nie mozna otworzyc bazy danych: " << sqlite3_errmsg(db) << endl;
        send(kgniazdo, "Blad bazy danych", strlen("Blad bazy danych"), 0);
        return;
    }

    const char* sql = "SELECT nazwa FROM users";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) 
    {
        cout << "Blad przygotowania zapytania: " << sqlite3_errmsg(db) << endl;
        send(kgniazdo, "Blad przygotowania zapytania", strlen("Blad przygotowania zapytania"), 0);
        sqlite3_close(db);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) 
    {
        const char* nazwa = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        lista += nazwa;
        lista += "\n";
    }

    send(kgniazdo, lista.c_str(), lista.length(), 0);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}
//void logowanie(SOCKET kgniazdo) //(niedokonczone)
//{
   // char buffer[1024];
  //  int recvSize = recv(kgniazdo, buffer, sizeof(buffer), 0);
    //if (recvSize == SOCKET_ERROR)
   // {
  //      int error = WSAGetLastError();
//        cout << "Blad odbierania wiadomosci: " << error << endl;
    //    send(kgniazdo, "Blad odbierania wiadomosci", strlen("Blad odbierania wiadomosci"), 0);
//    }

   // buffer[recvSize] = '\0';

  //  sqlite3* db;
   // int rc = sqlite3_open("bd.db", &db);
   // if (rc != SQLITE_OK)
  //  {
  //      cout << "Nie mozna otworzyc bazy: " << sqlite3_errmsg(db) << endl;
 //       send(kgniazdo, "Blad bazy danych", strlen("Blad bazy danych"), 0);
  //      return;
//    }

//    const char* sql = "SELECT COUNT(*) FROM users WHERE nazwa = ?";
  //  sqlite3_stmt* stmt;
   // rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

int main() 
{
    WSADATA wsadata;
    SOCKET sgniazdo, kgniazdo;
    struct sockaddr_in serwer, klient;
    int rozmiar, ile = 0;
    char buffer[1024];

    ConfReader dbreader;
    dbreader.load();
    string ipS(dbreader.addrIp);
    int port = dbreader.port;

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) 
    {
        cout << "Blad inicjalizacji Winsock" << endl;
        return 1;
    }

    sgniazdo = socket(AF_INET, SOCK_STREAM, 0);
    if (sgniazdo == INVALID_SOCKET) 
    {
        cout << "Blad tworzenia gniazda" << endl;
        WSACleanup();
        return 1;
    }

    serwer.sin_family = AF_INET;
    serwer.sin_addr.s_addr = inet_addr(ipS.c_str());
    serwer.sin_port = htons(port);

    if (bind(sgniazdo, (struct sockaddr*)&serwer, sizeof(serwer)) == SOCKET_ERROR) 
    {
        cout << "Blad bindowania" << endl;
        closesocket(sgniazdo);
        WSACleanup();
        return 1;
    }

    if (listen(sgniazdo, 3) == SOCKET_ERROR) 
    {
        cout << "Blad nasluchiwania" << endl;
        closesocket(sgniazdo);
        WSACleanup();
        return 1;
    }

    cout << "Czekanie na polaczenie" << endl;
    while (ile <= 999) 
    {
        rozmiar = sizeof(struct sockaddr_in);
        kgniazdo = accept(sgniazdo, (struct sockaddr*)&klient, &rozmiar);
        if (kgniazdo == INVALID_SOCKET) 
        {
            cout << "Blad akceptacji polaczenia" << endl;
            closesocket(sgniazdo);
            WSACleanup();
            return 1;
        }

        cout << "Polaczono z klientem" << endl;
        rejestracja(kgniazdo);
        listauzytkownikow(kgniazdo);
        closesocket(kgniazdo);
        ile++;
    }

    closesocket(sgniazdo);
    WSACleanup();
    return 0;
}
