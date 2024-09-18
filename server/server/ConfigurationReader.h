#include <sqlite3.h>
#include <iostream>
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#include <string>

using namespace std;

class Server {
private:
    string ip;
    int port;
    SOCKET sgniazdo, kgniazdo;
    struct sockaddr_in serwer, klient;
    char buffer[1024];

public:
    // Konstruktor otwieraj¹cy bazê danych i inicjalizuj¹cy IP oraz port
    Server() {
        sqlite3* db;
        sqlite3_stmt* stmt;
        int rc = sqlite3_open("bd.db", &db);

        if (rc) {
            cout << "Nie mo¿na otworzyæ bazy danych: " << sqlite3_errmsg(db) << endl;
            return;
        }

        // Zak³adamy, ¿e dane IP i port znajduj¹ siê w tabeli 'server_config'
        const char* sql = "SELECT ip, port FROM configuration";
        rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);

        if (rc != SQLITE_OK) {
            cout << "Nie uda³o siê przygotowaæ zapytania SQL" << endl;
            sqlite3_close(db);
            return;
        }

        // Odczytanie danych z bazy
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            ip = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            port = sqlite3_column_int(stmt, 1);
        }
        else {
            cout << "Nie znaleziono danych" << endl;
        }

        sqlite3_finalize(stmt);
        sqlite3_close(db);
    }

    // Inicjalizacja Winsock
    bool initializeWinsock() {
        WSADATA wsadata;
        if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
            cout << "B³¹d inicjalizacji Winsock" << endl;
            return false;
        }
        return true;
    }

    // Tworzenie gniazda
    bool createSocket() {
        sgniazdo = socket(AF_INET, SOCK_STREAM, 0);
        if (sgniazdo == INVALID_SOCKET) {
            cout << "B³¹d tworzenia gniazda" << endl;
            WSACleanup();
            return false;
        }
        return true;
    }

    // Konfiguracja serwera i bindowanie
    bool configureServer() {
        serwer.sin_family = AF_INET;
        serwer.sin_addr.s_addr = inet_addr(ip.c_str());
        serwer.sin_port = htons(port);

        if (bind(sgniazdo, (struct sockaddr*)&serwer, sizeof(serwer)) == SOCKET_ERROR) {
            cout << "B³¹d bindowania gniazda" << endl;
            closesocket(sgniazdo);
            WSACleanup();
            return false;
        }
        return true;
    }

    // Nas³uchiwanie i obs³uga po³¹czeñ
    void listenForConnections() {
        int rozmiar, recvSize;
        cout << "Serwer nas³uchuje na IP: " << ip << ", Port: " << port << endl;
        listen(sgniazdo, 3);
        rozmiar = sizeof(struct sockaddr_in);

        // Akceptowanie po³¹czenia
        kgniazdo = accept(sgniazdo, (struct sockaddr*)&klient, &rozmiar);
        if (kgniazdo == INVALID_SOCKET) {
            cout << "B³¹d akceptacji po³¹czenia" << endl;
            closesocket(sgniazdo);
            WSACleanup();
            return;
        }

        cout << "Po³¹czono z klientem" << endl;

        // Odbieranie danych od klienta
        recvSize = recv(kgniazdo, buffer, sizeof(buffer), 0);
        if (recvSize == SOCKET_ERROR) {
            cout << "B³¹d odbierania danych" << endl;
        }
        else {
            buffer[recvSize] = '\0';
            cout << "Odebrano: " << buffer << endl;
        }

        // Zamkniêcie po³¹czenia
        closesocket(kgniazdo);
    }

    // Zamkniêcie gniazd i Winsock
    void cleanup() {
        closesocket(sgniazdo);
        WSACleanup();
    }
};

int main() {
    Server server;

    if (!server.initializeWinsock()) return 1;
    if (!server.createSocket()) return 1;
    if (!server.configureServer()) return 1;

    server.listenForConnections();
    server.cleanup();

    return 0;
}


