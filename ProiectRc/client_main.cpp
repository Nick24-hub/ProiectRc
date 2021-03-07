//inspirat din exemplele de la cursul de RC al doamnei profesoare Alina Alboaie

#include <sys/socket.h>
#include <netinet/in.h>
#include <cerrno>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <arpa/inet.h>
#include <cstring>

// codul de eroare returnat de anumite apeluri
extern int errno;

// portul de conectare la server
int port;
char mesaj[150];

void afisare() {
    printf("%s\n", mesaj);
    fflush(stdout);
}

void citeste(int descriptor) {
    memset(mesaj, '\0', 150);
    int lg;
    if (read(descriptor, &lg, sizeof(int)) < 0)
        perror("[client]Eroare la read() lungime de la server.\n");

    if (read(descriptor, mesaj, lg) < 0)
        perror("[client]Eroare la read() mesaj de la server.\n");
    fflush(stdout);
}

void citestetabla(int descriptor) {
    int lg;
    memset(mesaj, '\0', 150);
    if (read(descriptor, &lg, sizeof(int)) < 0)
        perror("[client]Eroare la read() lungime de la server.\n");

    if (read(descriptor, mesaj, lg) < 0)
        perror("[client]Eroare la read() mesaj de la server.\n");
    char tabla[10][10];
    memset(tabla, '\0', 100);
    int k = 0, i, j;
    for (i = 1; i <= 6; i++) {
        for (j = 1; j <= 7; j++) {
            tabla[i][j] = mesaj[k++];
        }
    }
    for (i = 1; i <= 6; i++) {
        for (j = 1; j <= 7; j++) {
            printf("%c ", tabla[i][j]);
        }
        printf("\n");
    }
    fflush(stdout);
}


int main(int argc, char *argv[]) {
    int jucator;
    int sd;            // descriptorul de socket
    struct sockaddr_in server{};    // structura folosita pentru conectare

    // exista toate argumentele in linia de comanda?
    if (argc != 3) {
        printf("Sintaxa: %s <adresa_server> <port>\n", argv[0]);
        return -1;
    }

    // stabilim portul
    port = atoi(argv[2]);

    // cream socketul
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Eroare la socket().\n");
        return errno;
    }

    // umplem structura folosita pentru realizarea conexiunii cu serverul
    // familia socket-ului
    server.sin_family = AF_INET;
    // adresa IP a serverului
    server.sin_addr.s_addr = inet_addr(argv[1]);
    // portul de conectare
    server.sin_port = htons(port);

    // ne conectam la server
    if (connect(sd, (struct sockaddr *) &server, sizeof(struct sockaddr)) == -1) {
        perror("[client]Eroare la connect().\n");
        return errno;
    }

    // citirea raspunsului dat de server
    // (apel blocant pina cind serverul raspunde)

    citeste(sd); //jucator 1 sau 2
    jucator = mesaj[0] - '0';

    if (jucator == 1) {
        citeste(sd); // prezenta unui jucator
        afisare();
    }
    citeste(sd); // prezenta a doi jucatori
    afisare();
    citeste(sd);// primesti culoarea si incepe jocul
    afisare();

    char coloana[5];
    int randulcui, lg, continua = 1;
    while (continua) {
        citeste(sd);// scorul
        afisare();
        while (true) {
            memset(coloana, '\0', 5);
            citeste(sd); // e gata sau nu?
            if (strcmp(mesaj, "gata") == 0) // s a terminat
            {
                citeste(sd); // concluzie
                afisare();
                citestetabla(sd);
                break;
            }
            citeste(sd);// al cui e randul
            randulcui = mesaj[0] - '0';
            citeste(sd); // runda
            afisare();
            citestetabla(sd);
            citeste(sd); //notificare mutare sau asteptare
            afisare();
            if (jucator == randulcui) //trebuie sa aleaga o coloana
            {
                int ok = 0;
                while (!ok) {
                    scanf("%s", coloana);
                    lg = strlen(coloana);
                    if (write(sd, &lg, sizeof(int)) <= 0) {
                        perror("[client]Eroare la write() spre server.\n");
                        return errno;
                    }
                    if (write(sd, &coloana, lg) <= 0) {
                        perror("[client]Eroare la write() spre server.\n");
                        return errno;
                    }
                    citeste(sd);// notificare daca mutarea e permisa
                    afisare();
                    citeste(sd);// raspuns pentru ok
                    ok = mesaj[0] - '0';
                }
            }
        }
        citeste(sd); //play again?
        afisare();
        printf("Scrie DA sau NU in terminal.\n");
        int ok = 0;
        char r[10];
        memset(r, '\0', 10);
        while (!ok) {
            scanf("%s", r);
            lg = strlen(r);
            if (write(sd, &lg, sizeof(int)) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            if (write(sd, &r, lg) <= 0) {
                perror("[client]Eroare la write() spre server.\n");
                return errno;
            }
            citeste(sd);// raspuns pentru ok
            ok = mesaj[0] - '0';
            if (ok == 0) {
                citeste(sd);
                afisare();
                continue;
            }
        }
        citeste(sd);// notificare daca jocul continua
        afisare();
        if (strcmp(mesaj, "Jocul continua.") != 0) {
            continua = 0;
            citeste(sd); //scor final
            afisare();
        }
    }
    //inchidem conexiunea, am terminat
    close(sd);
    return 0;
}