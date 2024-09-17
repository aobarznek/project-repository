#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <sqlite3.h>
#include <iostream>
#include <winsock2.h>
#include <string>
#include <fstream>
#include "ConfReader.h"
#pragma comment(lib, "ws2_32.lib")

using namespace std;

void rejestracja(SOCKET kgniazdo) {
    char buffer[1024];
    int recvSize = recv(kgniazdo, buffer, sizeof(buffer), 0);
    if (recvSize == SOCKET_ERROR) {
        int error = WSAGetLastError();
        cout << "Blad odbierania wiadomosci: " << error << endl;
        return;
    }
    else {
        buffer[recvSize] = '\0';

        // Otwieranie bazy danych
        sqlite3* db;
        int rc = sqlite3_open("bd.db", &db);
        if (rc != SQLITE_OK) 
        {
            cerr << "Nie mozna otworzyc bazy danych: " << sqlite3_errmsg(db) << endl;
            return;
        }

        // Tworzenie zapytania 
        const char* sql = "INSERT INTO users (nazwa) VALUES (?)";
        sqlite3_stmt* stmt;

        // Przygotowanie zapytania
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        if (rc != SQLITE_OK) 
        {
            cerr << "Blad przygotowania zapytania: " << sqlite3_errmsg(db) << endl;
            sqlite3_close(db);
            return;
        }

        sqlite3_bind_text(stmt, 1, buffer, -1, SQLITE_STATIC);

        // Wykonanie zapytania
        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE) {
            cerr << "Blad wykonywania zapytania: " << sqlite3_errmsg(db) << endl;
        }
        else {
            cout << "Nazwa uzytkownika zostala zapisana." << endl;
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }
}

int main() {
    sqlite3* baza;
    int wynik;
    wynik = sqlite3_open("bd.db", &baza);
    sqlite3_close(baza);

    WSADATA wsadata;
    SOCKET sgniazdo, kgniazdo;
    struct sockaddr_in serwer, klient;
    int rozmiar, ile;
    char buffer[1024];
    string ip;
    ile = 0;

    ConfReader dbreader;
    dbreader.load();
    string ipS(dbreader.addrIp);
    int port = dbreader.port;

    // Inicjalizacja Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        cout << "Blad inicjalizacji Winsock" << endl;
        return 1;
    }

    // Tworzenie gniazda serwera
    sgniazdo = socket(AF_INET, SOCK_STREAM, 0);
    if (sgniazdo == INVALID_SOCKET) {
        cout << "Blad tworzenia gniazda" << endl;
        WSACleanup();
        return 1;
    }

    // Ustawienia serwera
    serwer.sin_family = AF_INET;
    serwer.sin_addr.s_addr = inet_addr(ipS.c_str()); // Użyj adresu IP z pliku konfiguracyjnego
    serwer.sin_port = htons(port);

    // Bindowanie gniazda
    if (bind(sgniazdo, (struct sockaddr*)&serwer, sizeof(serwer)) == SOCKET_ERROR) {
        cout << "Blad bindowania" << endl;
        closesocket(sgniazdo);
        WSACleanup();
        return 1;
    }

    // Nasłuchiwanie na połączenia
    if (listen(sgniazdo, 3) == SOCKET_ERROR) {
        cout << "Blad nasluchiwania" << endl;
        closesocket(sgniazdo);
        WSACleanup();
        return 1;
    }

    cout << "Czekanie na polaczenie" << endl;

    while (ile <= 999) 
    {
        rozmiar = sizeof(struct sockaddr_in);

        // Akceptacja połączenia od klienta
        kgniazdo = accept(sgniazdo, (struct sockaddr*)&klient, &rozmiar);
        if (kgniazdo == INVALID_SOCKET) {
            cout << "Blad akceptacji polaczenia" << endl;
            closesocket(sgniazdo);
            WSACleanup();
            return 1;
        }

        cout << "Polaczono z klientem" << endl;

        // Obsługa klienta
        rejestracja(kgniazdo);
        //wiadomosci
        int recvSize = recv(kgniazdo, buffer, sizeof(buffer), 0);
        buffer[recvSize] = '\0';
        cout << "wiadomosc: "<<buffer<<endl;
        

        
        // Zamknięcie połączenia z klientem
        closesocket(kgniazdo);
        ile++;
    }

    // Zamknięcie gniazda serwera
    closesocket(sgniazdo);
    WSACleanup();
    return 0;
}
