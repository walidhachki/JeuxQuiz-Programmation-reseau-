#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

/** La structure utilisateur **/
typedef struct Utilisateur{
    char nom_utilisateur[30];
    int score;
}Utilisateur;

/********* MAIN **********/
int main()
{
    //Creation du socket avec "socket()"
    int socketClient = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrClient;
    addrClient.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrClient.sin_family = AF_INET;
    addrClient.sin_port = htons(50000);
    
    // Connecter le client avec le serveur avec "connect()"
    if(connect(socketClient, (const struct sockaddr *)&addrClient, sizeof(addrClient)) < 0)
          printf("Erreur dans connect \n");
    printf("connect\n");

    int j = 0;
    int mode;
    Utilisateur utilisateur;
    char *msg;
    char rep[2];
    msg = malloc(sizeof(char)*1000);
    int i=0;
    recv(socketClient, msg, 1000, 0);
    printf("\t\t%s\n", msg);
    scanf("%d", &mode);
    
    send(socketClient, &mode, sizeof(mode), 0);
    recv(socketClient, msg, 1000, 0);
    printf("\t\t%s\n", msg);
    scanf("%s", utilisateur.nom_utilisateur);
    utilisateur.score = 0;
    send(socketClient, &utilisateur, sizeof(utilisateur), 0);
    while(1)
    {
        int r = recv(socketClient, msg, 1000, 0);
        if (!strncmp(msg, "Votre", 5))
        {
            printf("Finish\n");
            printf("%s\n", msg);
            break;
        }
        printf("%s\n", msg);
        scanf("%s", rep);
        send(socketClient, rep, sizeof(rep), 0);
    }
    close(socketClient);
    printf("Fin du Quiz\n");
    return 0;
}
