//inspirat din exemplele de la cursul de RC al doamnei profesoare Alina Alboaie

#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <unistd.h>
#include <cstdio>
#include <cstring>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "EndlessLoop"

// portul folosit
#define PORT 2908

// codul de eroare returnat de anumite apeluri
extern int errno;

struct DateThread {
    int idThread; //id-ul thread-ului tinut in evidenta de acest program
    int client1, client2; //descriptorii intorsi de accept pentru cei doi jucatori
};

void camerajoc(DateThread *); // functia executata de fiecare thread ce realizeaza comunicarea cu clientii

int incepejocul(DateThread *, int partida);

void cameraasteptare(DateThread *);

void trimit(DateThread *, char *, int);

char citeste(DateThread *, int);


int main() {
    struct sockaddr_in server{};    // structura folosita de server
    struct sockaddr_in from{};
    int sd;        //descriptorul de socket
    pthread_t threaduri[100];    //Identificatorii thread-urilor care se vor crea
    int i = 0, nrclienti = 0;

    // crearea unui socket
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("[server]Eroare la socket().\n");
        return errno;
    }

    // utilizarea optiunii SO_REUSEADDR
    int on = 1;
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // pregatirea structurilor de date
    bzero(&server, sizeof(server));
    bzero(&from, sizeof(from));

    // umplem structura folosita de server
    // stabilirea familiei de socket-uri
    server.sin_family = AF_INET;
    // acceptam orice adresa
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    // utilizam un port utilizator
    server.sin_port = htons(PORT);

    // atasam socketul
    if (bind(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("[server]Eroare la bind().\n");
        return errno;
    }

    // punem serverul sa asculte daca vin clienti sa se conecteze
    if (listen(sd, 2) == -1) {
        perror("[server]Eroare la listen().\n");
        return errno;
    }

    DateThread *dt; //parametru functia executata de thread

    // servim in mod concurent perechi de cate 2 clienti pe care apoi ii servim in mod iterativ
    while (true) {
        int client;
        int length = sizeof(from);

        printf("[server]Asteptam la portul %d...\n", PORT);
        fflush(stdout);

        // acceptam un client (stare blocanta pina la realizarea conexiunii)
        if ((client = accept(sd, (struct sockaddr *) &from, reinterpret_cast<socklen_t *>(&length))) < 0) {
            perror("[server]Eroare la accept().\n");
            continue;
        }
        nrclienti++; //contorizam nr de clienti

        // int idThread; //id-ul threadului
        // int client1,client2; //descriptorii intorsi de accept

        if (nrclienti % 2 == 1) //daca nu are pereche
        {
            dt = new DateThread;
            dt->idThread = i++;
            dt->client1 = client;
            dt->client2 = -1;
            cameraasteptare(dt);
        } else //daca am gasit pereche
        {
            dt->client2 = client;
            pthread_create(&threaduri[i], nullptr, reinterpret_cast<void *(*)(void *)>(&camerajoc), dt);
            //camerajoc(dt);
        }

    }//while
}

void trimit(DateThread *a, char mesaj[150], int destinatar) // la destinatar 0 inseamna amandoi , 1 - primul, 2 - al doilea
{
    int lg = strlen(mesaj);
    // returnam mesaj  destinatarului
    if (destinatar == 1) {
        if (write(a->client1, &lg, sizeof(int)) <= 0) {
            printf("[Thread %d] ", a->idThread);
            perror("[Thread]Eroare la write() catre jucator 1.\n");
            return;
        } else
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", a->idThread);

        if (write(a->client1, mesaj, lg) <= 0) {
            printf("[Thread %d] ", a->idThread);
            perror("[Thread]Eroare la write() catre jucator 1.\n");
            return;
        } else
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", a->idThread);

    } else if (destinatar == 2) {
        if (write(a->client2, &lg, sizeof(int)) <= 0) {
            printf("[Thread %d] ", a->idThread);
            perror("[Thread]Eroare la write() catre jucator 2.\n");
            return;
        } else
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", a->idThread);

        if (write(a->client2, mesaj, lg) <= 0) {
            printf("[Thread %d] ", a->idThread);
            perror("[Thread]Eroare la write() catre jucator 2.\n");
            return;
        } else
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", a->idThread);
    } else {
        if (write(a->client1, &lg, sizeof(int)) <= 0) {
            printf("[Thread %d] ", a->idThread);
            perror("[Thread]Eroare la write() catre jucator 1.\n");
            return;
        } else
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", a->idThread);

        if (write(a->client1, mesaj, lg) <= 0) {
            printf("[Thread %d] ", a->idThread);
            perror("[Thread]Eroare la write() catre jucator 1.\n");
            return;
        } else
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", a->idThread);

        if (write(a->client2, &lg, sizeof(int)) <= 0) {
            printf("[Thread %d] ", a->idThread);
            perror("[Thread]Eroare la write() catre jucator 2.\n");
            return;
        } else
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", a->idThread);

        if (write(a->client2, mesaj, lg) <= 0) {
            printf("[Thread %d] ", a->idThread);
            perror("[Thread]Eroare la write() catre jucator 2.\n");
            return;
        } else
            printf("[Thread %d]Mesajul a fost trasmis cu succes.\n", a->idThread);
    }
    fflush(stdout);
}

void cameraasteptare(DateThread *x) {
    char mesaj[150];
    memset(mesaj, '\0', 150);
    printf("[server]S-a conectat primul jucator.\n");
    strcpy(mesaj, "1");
    trimit(x, mesaj, 1); // trimit nr jucator
    strcpy(mesaj, "Esti primul jucator in aceasta camera, trebuie sa astepti ca un oponent sa se conecteze.");
    trimit(x, mesaj, 1);
    printf("[server]Mesajul de asteptare a fost trimis cu succes.\n");
    fflush(stdout);
    //pthread_detach(pthread_self());
}

void camerajoc(DateThread *y) {
    pthread_detach(pthread_self());
    char mesaj[150];
    memset(mesaj, '\0', 150);
    printf("[server]S-a conectat al doilea jucator.\n");
    fflush(stdout);
    strcpy(mesaj, "2");
    trimit(y, mesaj, 2);//trimit nr jucator
    // returnam mesaje de notificare celor doi clienti
    strcpy(mesaj, "S-au conectat doi jucatori in aceasta camera, partida incepe acum.");
    trimit(y, mesaj, 0);
    strcpy(mesaj, "Esti jucatorul rosu (R).");
    trimit(y, mesaj, 1);
    strcpy(mesaj, "Esti jucatorul albastru (A).");
    trimit(y, mesaj, 2);
    int partida = 0, victoriiclient1 = 0, victoriiclient2 = 0, remize = 0;
    while (true) {
        partida++;
        sprintf(mesaj, "Scorul este: %d victorii pentru R, %d victorii pentru A si %d remize.", victoriiclient1,
                victoriiclient2, remize);
        trimit(y, mesaj, 0);
        int x = incepejocul(y, partida);
        if (x == 1)
            victoriiclient1++;
        else if (x == 2)
            victoriiclient2++;
        else remize++;
        // posibilitatea unei noi reprize
        strcpy(mesaj, "Inca o partida?");
        trimit(y, mesaj, 0);
        char raspuns1, raspuns2;

        while (true) {
            raspuns1 = citeste(y, 1);
            if (raspuns1 != 'D' && raspuns1 != 'd' && raspuns1 != 'N' && raspuns1 != 'n') {
                strcpy(mesaj, "0");
                trimit(y, mesaj, 1);
                strcpy(mesaj, "Raspuns invalid, mai incearca.");
                trimit(y, mesaj, 1);
            } else {
                strcpy(mesaj, "1");
                trimit(y, mesaj, 1);
                break;
            }

        }
        while (true) {
            raspuns2 = citeste(y, 2);
            if (raspuns2 != 'D' && raspuns2 != 'd' && raspuns2 != 'N' && raspuns2 != 'n') {
                strcpy(mesaj, "0");
                trimit(y, mesaj, 2);
                strcpy(mesaj, "Raspuns invalid, mai incearca.");
                trimit(y, mesaj, 2);
            } else {
                strcpy(mesaj, "1");
                trimit(y, mesaj, 2);
                break;
            }
        }

        if (raspuns1 == 'D' || raspuns1 == 'd') {
            if (raspuns2 == 'D' || raspuns2 == 'd') {
                strcpy(mesaj, "Jocul continua.");
                trimit(y, mesaj, 0);
                continue;
            } else {
                strcpy(mesaj, "Deoarece nu sunt ambii jucatori de acord, jocul se incheie cu scorul:");
                trimit(y, mesaj, 0);
                sprintf(mesaj, "%d victorii pentru R, %d victorii pentru A si %d remize.", victoriiclient1,
                        victoriiclient2, remize);
                trimit(y, mesaj, 0);
                break;
            }
        } else {
            strcpy(mesaj, "Deoarece nu au cazut de acord ambii jucatori, jocul se incheie cu scorul:");
            trimit(y, mesaj, 0);
            sprintf(mesaj, "%d victorii pentru R, %d victorii pentru A si %d remize.", victoriiclient1, victoriiclient2,
                    remize);
            trimit(y, mesaj, 0);
            break;
        }
    }
    // am terminat cu acesta camera, inchidem conexiunile
    close(y->client1);
    close(y->client2);
}

void castigator(DateThread *b, int jucator) {

    char mesaj[150];
    memset(mesaj, '\0', 150);
    strcpy(mesaj, "gata");
    trimit(b, mesaj, 0);
    if (jucator == 1) {
        strcpy(mesaj, "Jucatorul albastru (A) a castigat!");
        trimit(b, mesaj, 0);
    } else {
        strcpy(mesaj, "Jucatorul rosu (R) a castigat!");
        trimit(b, mesaj, 0);
    }
}

void trimitetabla(DateThread *c, char tabla[10][10]) {

    char mesaj[150];
    memset(mesaj, '\0', 100);
    int k = 0;
    for (int i = 1; i <= 6; i++)
        for (int j = 1; j <= 7; j++)
            mesaj[k++] = tabla[i][j];
    trimit(c, mesaj, 0);
}

char citeste(DateThread *d, int sursa) {
    char raspuns[5];
    memset(raspuns, '\0', 5);
    int lg;
    if (sursa == 1) {
        if (read(d->client1, &lg, sizeof(int)) <= 0) {
            printf("[Thread %d]\n", d->idThread);
            perror("Eroare la read() de la client.\n");
        } else
            printf("Mesajul a fost citit cu succes.\n");
        if (read(d->client1, &raspuns, lg) <= 0) {
            printf("[Thread %d]\n", d->idThread);
            perror("Eroare la read() de la client.\n");
        } else {
            printf("Mesajul a fost citit cu succes.\n");
            return raspuns[0];
        }
    } else {
        if (read(d->client2, &lg, sizeof(int)) <= 0) {
            printf("[Thread %d]\n", d->idThread);
            perror("Eroare la read() de la client.\n");
        } else
            printf("Mesajul a fost citit cu succes.\n");
        if (read(d->client2, &raspuns, lg) <= 0) {
            printf("[Thread %d]\n", d->idThread);
            perror("Eroare la read() de la client.\n");
        } else {
            printf("Mesajul a fost citit cu succes.\n");
            return raspuns[0];
        }
    }
    return 0;
}

int incepejocul(DateThread *z, int partida) {
    char tabla[10][10], copie[10][10], x, raspuns, mesaj[150];
    memset(mesaj, '\0', 150);
    memset(tabla, '\0', 100);
    memset(copie, '\0', 100);
    int i, j, runda = 0, nr, k, l, mutarealui;
    if (partida % 2 == 1) //stabilim cine incepe primul
        mutarealui = 2;
    else mutarealui = 1;
    // initializez tabla de joc
    for (i = 1; i <= 6; i++)
        for (j = 1; j <= 7; j++)
            tabla[i][j] = '0';
    while (true) {
        mutarealui = 3 - mutarealui;
        // verific daca e gata

        if (runda > 3) //verificare
        {
            memcpy(copie, tabla, sizeof(tabla));
            for (i = 1; i <= 6; i++) {
                for (j = 1; j <= 7; j++) {
                    if (copie[i][j] != '0') {
                        x = copie[i][j];
                        // verific pe verticala
                        nr = 1;
                        k = 1;
                        while (k < 4 && copie[i + k][j] == x) {
                            nr++;
                            k++;
                        }
                        k = 1;
                        while (k < 4 && copie[i - k][j] == x) {
                            nr++;
                            k++;
                        }
                        if (nr > 3) // avem un castigator
                        {
                            castigator(z, mutarealui);
                            trimitetabla(z, tabla);
                            return 3-mutarealui;
                        }

                        // verific pe orizontala

                        nr = 1;
                        k = 1;
                        while (k < 4 && copie[i][j + k] == x) {
                            nr++;
                            k++;
                        }
                        k = 1;
                        while (k < 4 && copie[i][j - k] == x) {
                            nr++;
                            k++;
                        }
                        if (nr > 3) // avem un castigator
                        {
                            castigator(z, mutarealui);
                            trimitetabla(z, tabla);
                            return 3-mutarealui;
                        }

                        // verific pe diagonala principala

                        nr = 1;
                        k = 1;
                        l = 1;
                        while (k < 4 && copie[i - k][j - l] == x) {
                            nr++;
                            k++;
                            l++;
                        }
                        k = 1;
                        l = 1;
                        while (k < 4 && copie[i + k][j + l] == x) {
                            nr++;
                            k++;
                            l++;
                        }
                        if (nr > 3) // avem un castigator
                        {
                            castigator(z, mutarealui);
                            trimitetabla(z, tabla);
                            return 3-mutarealui;
                        }

                        // verific pe diagonala secundara

                        nr = 1;
                        k = 1;
                        l = 1;
                        while (k < 4 && copie[i - k][j + l] == x) {
                            nr++;
                            k++;
                            l++;
                        }
                        k = 1;
                        l = 1;
                        while (k < 4 && copie[i + k][j - l] == x) {
                            nr++;
                            k++;
                            l++;
                        }
                        if (nr > 3) // avem un castigator
                        {
                            castigator(z, mutarealui);
                            trimitetabla(z, tabla);
                            return 3-mutarealui;
                        }

                    }
                }
            }
        }
        if (runda == 43) // avem egalitate
        {
            strcpy(mesaj, "gata");
            trimit(z, mesaj, 0);
            strcpy(mesaj, "Tabla de joc a fost completata->Egalitate!");
            trimit(z, mesaj, 0);
            trimitetabla(z, tabla);
            return 3;
        }
        strcpy(mesaj, "continua");
        trimit(z, mesaj, 0);
        sprintf(mesaj, "%d", mutarealui); // al cui e randul
        trimit(z, mesaj, 0);
        runda++;
        sprintf(mesaj, "Runda %d", (runda / 2 + runda % 2)); //trimit runda
        trimit(z, mesaj, 0);
        trimitetabla(z, tabla);
        if (mutarealui == 1) {
            strcpy(mesaj, "Este randul tau, scrie indicele coloanei pe care vrei sa pui o piesa->");
            trimit(z, mesaj, 1);
            strcpy(mesaj, "Este randul jucatorului rosu (R), asteapta mutarea lui.");
            trimit(z, mesaj, 2);
            while (true) {
                raspuns = citeste(z, 1);
                if (tabla[1][raspuns - '0'] != '0') {
                    strcpy(mesaj, "Coloana selectata este deja plina , alege alta coloana->");
                    trimit(z, mesaj, 1);
                    strcpy(mesaj, "0");
                    trimit(z, mesaj, 1);
                } else {
                    strcpy(mesaj, "S-a realizat mutarea");
                    trimit(z, mesaj, 1);
                    i = 1;
                    while (tabla[i][raspuns - '0'] == '0')
                        i++;
                    tabla[i - 1][raspuns - '0'] = 'R';
                    strcpy(mesaj, "1");
                    trimit(z, mesaj, 1);
                    break;
                }
            }
        } else {
            strcpy(mesaj, "Este randul tau, scrie indicele coloanei pe care vrei sa pui o piesa->");
            trimit(z, mesaj, 2);
            strcpy(mesaj, "Este randul jucatorului albastru (A), asteapta mutarea lui.");
            trimit(z, mesaj, 1);
            while (true) {
                raspuns = citeste(z, 2);
                if (tabla[1][raspuns - '0'] != '0') {
                    strcpy(mesaj, "Coloana selectata este deja plina , alege alta coloana->");
                    trimit(z, mesaj, 2);
                    strcpy(mesaj, "0");
                    trimit(z, mesaj, 2);
                } else {
                    strcpy(mesaj, "S-a realizat mutarea");
                    trimit(z, mesaj, 2);
                    i = 1;
                    while (tabla[i][raspuns - '0'] == '0')
                        i++;
                    tabla[i - 1][raspuns - '0'] = 'A';
                    strcpy(mesaj, "1");
                    trimit(z, mesaj, 2);
                    break;
                }
            }
        }
    }
}
