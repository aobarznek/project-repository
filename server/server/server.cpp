#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <sqlite3.h>
#include <iostream>
#include <winsock2.h>
#include <string>
#include <fstream>
#include <thread>
#include <unordered_map>
#include "ConfReader.h"

#pragma comment(lib, "ws2_32.lib")

using namespace std;

unordered_map<std::string, SOCKET> userSockets;
void logowanie(SOCKET kgniazdo) {
    char buffer[1024];
    int recvSize = recv(kgniazdo, buffer, sizeof(buffer), 0);
    if (recvSize == SOCKET_ERROR) {
        int error = WSAGetLastError();
        cout << "Blad odbierania wiadomosci: " << error << endl;
        send(kgniazdo, "Blad odbierania wiadomosci", strlen("Blad odbierania wiadomosci"), 0);
        return;
    }

    buffer[recvSize] = '\0';
    string username(buffer);

    sqlite3* db;
    int rc = sqlite3_open("bd.db", &db);
    if (rc != SQLITE_OK) {
        cout << "Nie mozna otworzyc bazy: " << sqlite3_errmsg(db) << endl;  
        send(kgniazdo, "Blad bazy danych", strlen("Blad bazy danych"), 0);
        return;
    }

    const char* sql = "SELECT COUNT(*) FROM users WHERE nazwa = ?";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Blad przygotowania zapytania: " << sqlite3_errmsg(db) << endl;
        send(kgniazdo, "Blad przygotowania zapytania", strlen("Blad przygotowania zapytania"), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    int userExists = sqlite3_column_int(stmt, 0);

    if (userExists > 0) {
        send(kgniazdo, "Zalogowano pomyslnie", strlen("Zalogowano pomyslnie"), 0);
        userSockets[username] = kgniazdo; 
    }
    else {
        send(kgniazdo, "Nie znaleziono uzytkownika", strlen("Nie znaleziono uzytkownika"), 0);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void rejestracja(SOCKET kgniazdo) {
    char buffer[1024];
    int recvSize = recv(kgniazdo, buffer, sizeof(buffer), 0);
    if (recvSize == SOCKET_ERROR) {
        int error = WSAGetLastError();
        cout << "Blad odbierania wiadomosci: " << error << endl;
        send(kgniazdo, "Blad odbierania wiadomosci", strlen("Blad odbierania wiadomosci"), 0);
        return;
    }

    buffer[recvSize] = '\0';
    string username(buffer); 
    sqlite3* db;
    int rc = sqlite3_open("bd.db", &db);
    if (rc != SQLITE_OK) {
        cout << "Nie mozna otworzyc bazy: " << sqlite3_errmsg(db) << endl;
        send(kgniazdo, "Blad bazy danych", strlen("Blad bazy danych"), 0);
        return;
    }

    const char* sql = "INSERT INTO users (nazwa) VALUES (?)";
    sqlite3_stmt* stmt;
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Blad przygotowania zapytania: " << sqlite3_errmsg(db) << endl;
        send(kgniazdo, "Blad przygotowania zapytania", strlen("Blad przygotowania zapytania"), 0);
        sqlite3_close(db);
        return;
    }

    sqlite3_bind_text(stmt, 1, username.c_str(), -1, SQLITE_STATIC);
    rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        cout << "Blad wykonywania zapytania: " << sqlite3_errmsg(db) << endl;
        send(kgniazdo, "Blad wykonywania zapytania", strlen("Blad wykonywania zapytania"), 0);
    }
    else {
        cout << "Nazwa uzytkownika zostala zapisana." << endl;
        send(kgniazdo, "Rejestracja sie powiodla", strlen("Rejestracja sie powiodla"), 0);
        userSockets[username] = kgniazdo;
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void listauzytkownikow(SOCKET kgniazdo) {
    sqlite3* db;
    sqlite3_stmt* stmt;
    string lista = "";

    int rc = sqlite3_open("bd.db", &db);
    if (rc != SQLITE_OK) {
        cout << "Nie mozna otworzyc bazy danych: " << sqlite3_errmsg(db) << endl;
        send(kgniazdo, "Blad bazy danych", strlen("Blad bazy danych"), 0);
        return;
    }

    const char* sql = "SELECT nazwa FROM users";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        cout << "Blad przygotowania zapytania: " << sqlite3_errmsg(db) << endl;
        send(kgniazdo, "Blad przygotowania zapytania", strlen("Blad przygotowania zapytania"), 0);
        sqlite3_close(db);
        return;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const char* nazwa = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        lista += nazwa;
        lista += "\n";
    }

    send(kgniazdo, lista.c_str(), lista.length(), 0);

    sqlite3_finalize(stmt);
    sqlite3_close(db);
}

void wyslijWiadomosc(SOCKET kgniazdo) {
    char buffer[1024];
    int recvSize = recv(kgniazdo, buffer, sizeof(buffer), 0);
    if (recvSize == SOCKET_ERROR) {
        int error = WSAGetLastError();
        cout << "Blad odbierania wiadomosci: " << error << endl;
        send(kgniazdo, "Blad odbierania wiadomosci", strlen("Blad odbierania wiadomosci"), 0);
        return;
    }

    buffer[recvSize] = '\0';


    string wiadomosc(buffer);
    size_t pos = wiadomosc.find(':');
    if (pos == string::npos) {
        send(kgniazdo, "Nieprawidlowy format wiadomosci", strlen("Nieprawidlowy format wiadomosci"), 0);
        return;
    }

    string odbiorca = wiadomosc.substr(0, pos);
    string tresc = wiadomosc.substr(pos + 1);


    if (userSockets.find(odbiorca) != userSockets.end()) {
        SOCKET odbiorcaSocket = userSockets[odbiorca];
        send(odbiorcaSocket, tresc.c_str(), tresc.length(), 0);
    }
    else {
        send(kgniazdo, "Odbiorca nie jest online", strlen("Odbiorca nie jest online"), 0);
    }
}

void odbierajWiadomosci(SOCKET kgniazdo) {
    char buffer[1024];
    while (true) {
        int recvSize = recv(kgniazdo, buffer, sizeof(buffer), 0);
        if (recvSize <= 0) break;
        buffer[recvSize] = '\0';
        cout << buffer << endl; 
    }
}

int main() {
    WSADATA wsadata;
    SOCKET sgniazdo, kgniazdo;
    struct sockaddr_in serwer, klient;
    int rozmiar;
    char buffer[1024];
    string username;
    ConfReader dbreader;
    dbreader.load();
    string ipS(dbreader.addrIp);
    int port = dbreader.port;

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        cout << "Blad inicjalizacji Winsock" << endl;
        return 1;
    }

    sgniazdo = socket(AF_INET, SOCK_STREAM, 0);
    if (sgniazdo == INVALID_SOCKET) {
        cout << "Blad tworzenia gniazda" << endl;
        WSACleanup();
        return 1;
    }

    serwer.sin_family = AF_INET;
    serwer.sin_addr.s_addr = inet_addr(ipS.c_str());
    serwer.sin_port = htons(port);

    if (bind(sgniazdo, (struct sockaddr*)&serwer, sizeof(serwer)) == SOCKET_ERROR) {
        cout << "Blad bindowania" << endl;
        closesocket(sgniazdo);
        WSACleanup();
        return 1;
    }

    if (listen(sgniazdo, 3) == SOCKET_ERROR) {
        cout << "Blad nasluchiwania" << endl;
        closesocket(sgniazdo);
        WSACleanup();
        return 1;
    }
    while (true) {
        rozmiar = sizeof(struct sockaddr_in);
        kgniazdo = accept(sgniazdo, (struct sockaddr*)&klient, &rozmiar);
        if (kgniazdo == INVALID_SOCKET) {
            cout << "Blad akceptacji polaczenia" << endl;
            continue;
        }

        cout << "Polaczono z klientem" << endl;

        char commandBuffer[1024];
        int recvSize = recv(kgniazdo, commandBuffer, sizeof(commandBuffer), 0);
        if (recvSize <= 0) {
            closesocket(kgniazdo);
            continue;
        }
        commandBuffer[recvSize] = '\0';
        string command(commandBuffer);

        if (command == "register") {
            rejestracja(kgniazdo);
        }
        else if (command == "login") {
            logowanie(kgniazdo);
            // Sprawdź, czy użytkownik został zalogowany
            if (userSockets.find(username) != userSockets.end()) {
            }
        }
        listauzytkownikow(kgniazdo);
        thread t(odbierajWiadomosci, kgniazdo);
        t.detach();
    }

    closesocket(sgniazdo);
    WSACleanup();
    return 0;
}
