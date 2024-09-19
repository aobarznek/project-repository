#include<iostream>
#include<winsock2.h>
#include<ws2tcpip.h>
#include <windows.h>
#include<regex> // Do walidacji adresu IP i portu
#pragma comment(lib,"ws2_32.lib")//winsock
using namespace std;

void setCursorPosition(int x, int y)
{
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    COORD position;
    position.X = x;
    position.Y = y;
    SetConsoleCursorPosition(hConsole, position);
}

bool iptest(const string& ip)
{
    const regex ipRegex("^((25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4][0-9]|[0-1]?[0-9][0-9]?)$");
    return regex_match(ip, ipRegex);
}


bool porttest(const string& port1)
{
    const regex portRegex("^[0-9]+$");
    if (regex_match(port1, portRegex))
    {
        int port = stoi(port1);
        return (port);
    }
    else
    {
        return false;
    }
}

int main()
{
  
        WSADATA wsaData;
        SOCKET kgniazdo;
        struct sockaddr_in server;
        string ip, port1, user, opcje;
        int port2;
        string setting, block, color, back;
        string nazwa, nazwa1, wiadomosc, odbiorca;
    // winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        cout << "Blad" << endl;
        return 1;
    }

    // gniazdo
    kgniazdo = socket(AF_INET, SOCK_STREAM, 0);
    if (kgniazdo == INVALID_SOCKET)
    {
        cout << "Blad" << endl;
        WSACleanup();
        return 1;
    }

    // wprowadzenie adresu IP
    do
    {
        cout << "Podaj IP serwera: ";
        getline(cin, ip);
        if (!iptest(ip))
        {
            cout << "Podales bledne IP" << endl;
        }
    } while (!iptest(ip));

    // wprowadzenie portu
    do
    {
        cout << "Podaj port: ";
        getline(cin, port1);
        if (!porttest(port1))
        {
            cout << "Podales bledny port" << endl;
        }
    } while (!porttest(port1));
    port2 = stoi(port1);

    // ustawienia serwera
    server.sin_family = AF_INET;
    server.sin_port = htons(port2);
    if (inet_pton(AF_INET, ip.c_str(), &server.sin_addr) <= 0)
    {
        cout << "Nieprawidlowy adres IP" << endl;
        closesocket(kgniazdo);
        WSACleanup();
        return 1;
    }

    // laczenie z serwerem
    if (connect(kgniazdo, (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        cout << "Blad polaczenia z serwerem, nastepuje wyjscie z programu" << endl;
        closesocket(kgniazdo);
        WSACleanup();
        return 1;
    }
    cout << "Polaczono z serwerem" << endl;


    do
    {
        cout << "REJESTRACJA/LOGOWANIE (logowanie niegotowe)";
        cout << "wpisz register lub login: ";
        getline(cin, opcje);
    } while (opcje != "register" and opcje != "login");
    if (opcje == "register")
    {
        cout << "podaj nazwe uzytkownika" << endl;

        getline(cin, nazwa);
        send(kgniazdo, nazwa.c_str(), nazwa.length(), 0);
        char response[1024];
        int recvSize = recv(kgniazdo, response, sizeof(response), 0);
        response[recvSize] = '\0';
        cout << response << endl;
        
    }
    //(NIE DZIALA)
    else if (opcje == "login") {
        cout << "Podaj nazwe uzytkownika do logowania: ";
        getline(cin, nazwa1);
        string request = "login " + nazwa1;
        send(kgniazdo, request.c_str(), request.length(), 0);
        char response[1024];
       int recvSize = recv(kgniazdo, response, sizeof(response), 0);
        response[recvSize] = '\0';
        cout << response << endl;
        if (strcmp(response, "Nie znaleziono uzytkownika") == 0) 
        {
           cout << "Uzytkownik nie istnieje. Przejdz do rejestracji." << endl;
       }
    }
    do
    {
        do
        {
            cout << "UZYTKOWNICY" << endl;

            char listauzytkownikow[1024];
            int recvSize = recv(kgniazdo, listauzytkownikow, sizeof(listauzytkownikow), 0);
            if (recvSize > 0) {
                listauzytkownikow[recvSize] = '\0';
                cout << listauzytkownikow << endl;
            }
            else {
                cout << "Blad podczas odbierania listy użytkownikow";
            }


            cout << "Aby wybrac uzytkownika napisz jego nazwe uzytkownika" << endl;
            cout << "exit - wyjscie z programu" << endl;
            cout << "settings - ustawienia" << endl;


            getline(cin, user);
            if (user != "exit" and user != "settings" and user != "winiary")
            {
                cout << "wprowadziles nazwe opcji ktora nie istnieje w programie" << endl;
            }
        } while (user != "exit" and user != "settings" and user != "winiary");
        if (user == "user")
        {
            
        cout<< "Napisz zdanie ktore chcesz wyslac do uzytkownika" << endl;
            getline(cin, wiadomosc);
            cout << "wybierz odbiorce: " << endl;
            getline(cin, odbiorca);
           
            string message = "message#" + odbiorca + "#" + wiadomosc;
            send(kgniazdo, message.c_str(), message.length(), 0);
            
        }
        if (user == "exit")
        {
            exit(0);
        }
        if (user == "settings")
        {
            do
            {
                cout << "mute - wyciszenie uzytkownika" << endl;
                cout << "block - blokowanie uzytkownikow" << endl;
                cout << "color - zmiana koloru tekstu " << endl;
                getline(cin, setting);
                if (setting != "mute" and setting != "block" and setting != "color")
                {
                    cout << "podales opcje ktora nie istnieje, wpisz ponownie" << endl;
                }
            } while (setting != "mute" and setting != "block" and setting != "color");

        }
        if (setting == "block")
        {
            cout << "Czy jestes pewny, ze chcesz zablokowac usera(tak/nie)" << endl;
            getline(cin, block);
            if (block == "tak")
            {
                cout << "zablokowales uzytkownika" << endl;
            }
            else
            {
                system("pause");
            }
        }
        if (setting == "color")
        {
            do
            {
                cout << "\nNapisz wybrany kolor (zielony/niebieski/fioletowy" << endl;
                getline(cin, color);
            } while (color != "zielony" and color != "niebieski" and color != "fioletowy");
            setCursorPosition(90, 0);
            if (color == "zielony")
            {
                system("color A");

                setCursorPosition(52, 50);
                cout << "kolor zmieniony na zielony\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
            }
            if (color == "niebieski")
            {
                system("color  9");
                cout << "kolor zmieniony na niebieski\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
            }
            if (color == "fioletowy")
            {
                system("color  5");
                cout << "kolor zmieniony na fioletowy\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n" << endl;
            }
        }
        setCursorPosition(0, 100);
        cout << "\n\n\nChcesz wrocic do menu glownego?(tak/nie)" << endl;
        getline(cin, back);
        if (back == "nie")
        {
            exit(0);
        }


    } while (back == "tak");



    Sleep(500);
    closesocket(kgniazdo);
    WSACleanup();
    return 0;
}