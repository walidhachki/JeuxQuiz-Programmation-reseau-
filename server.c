#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include<pthread.h>
#include <fcntl.h>

#define NOMBRE_JOUEUR 3

/** La structure utilisateur **/
typedef struct Utilisateur{
    char nom_utilisateur[30];
    int score;
}Utilisateur;

/** la structure SocketQst **/
typedef struct SocketQst
{
    char **qst;
    int socketClient;
    int indice_Utilisateur;
}SocketQst;

char **tabRep;
Utilisateur utilisateurs[NOMBRE_JOUEUR];

/** La fonction qui compte le nombre des lignes **/
int compte_nombre_ligne(char *fichier){
    int i = 0;
    char ligne[255];
    FILE *f = fopen(fichier, "r");
    while(fgets(ligne, sizeof(ligne), f)){
        i++;
    }
    fclose(f);
    return i;
}

/** la fonction de lecture à partir du fichier **/
char**   lecture_fichier(char *fichier, char **qst){
    int i = 0;
    char ligne[255];
    FILE *f = fopen(fichier, "r");
    while(fgets(ligne, sizeof(ligne), f)){
        qst[i] = strdup(ligne);
        i++;
    }
    fclose(f);
    qst[i] = NULL;
    return (qst);
}

/** La fonction qui vérifie la reponse **/
int Verifier_Rep(char *rep, int i)
{
    if (strncmp(rep,tabRep[i],1))
        return 0;
    return 1;
}

/** la fonction qui retourne le meilleur score **/
int meilleurScore(Utilisateur utilisateurs[NOMBRE_JOUEUR]){
    int max = 0;
    int indice = 0;
    for(int i = 0; i < NOMBRE_JOUEUR; i++){
        if(utilisateurs[i].score >= max){
            indice = i;
            max = utilisateurs[i].score;
        }
    }
    return indice;
}

/** Fonction du jeu **/
void *function_du_jeu(void *arg){
    SocketQst socket = *(SocketQst *)arg;
    char rep[128];
    int i = 0;
    int resultat = 0;
    int mode;
    char score[10];
    int j = socket.indice_Utilisateur;

    char *msg = strdup("/------------ choisir le mode (Unique/ Multi): -------------/\n \t 1: Mode Unique (un seul joueur)\n \t2: Mode Multi (multijoueur)");
    send(socket.socketClient, msg, strlen(msg) + 1, 0);
    recv(socket.socketClient, &mode, sizeof(mode), 0);
    free(msg);
    msg = strdup("/------ Entrer Votre Nom :  --------");
    send(socket.socketClient, msg, strlen(msg) + 1, 0);
    recv(socket.socketClient, &utilisateurs[j], sizeof(utilisateurs[j]), 0);
    printf("/----- Le joueur : %s -----/\n",utilisateurs[j].nom_utilisateur);
    free(msg);

    while(socket.qst[i]){
        send(socket.socketClient, socket.qst[i], strlen(socket.qst[i]) + 1, 0);
        int r = recv(socket.socketClient, &rep, sizeof(char) * 2, 0);
        //printf("Sa reponse %c\n", *rep);
        utilisateurs[j].score += Verifier_Rep(rep, i);
        //printf("%d\n", r);
        i++;
    }

    if(socket.qst[i] == NULL){
        sprintf(score, "%d", utilisateurs[j].score);
        msg = strcat(strdup("Votre resultat est: "), score);
        send(socket.socketClient, msg, strlen(msg) + 1, 0);
        printf("%s\n",msg);
    }
    free(msg);
    close(socket.socketClient);
    pthread_exit(NULL);
}

/********* MAIN *****************/
int main(int argc, char **argv){
    int f;
    SocketQst socketQst;
    
    //condition
    if(argc != 3){
        printf("Entrer les fichiers des questions et reponses\n");
        exit(EXIT_FAILURE);
    }

    //initialiser le score de tt les joueurs
    for(int i = 0; i < 3; i++){
        utilisateurs[i].score = 0;
    }

    //Lecture des questions
    int nombreDesLignes = compte_nombre_ligne(argv[1]);

    if ((f = open(argv[1], O_RDONLY)) < 0){
        exit(-1);
    }
    else{
        socketQst.qst = (char**)malloc(sizeof(char *) * (nombreDesLignes + 1));
		socketQst.qst = lecture_fichier(argv[1], socketQst.qst);
		close(f);
    }

    //Lecture des reponses
    nombreDesLignes = compte_nombre_ligne(argv[2]);

    if ((f = open(argv[2], O_RDONLY)) < 0){
        exit(-1);
    }
	else {
        tabRep = (char**)malloc(sizeof(char *) * (nombreDesLignes + 1));
		tabRep = lecture_fichier(argv[2], tabRep);
		close(f);
    }   

    // Creation du socket avec "Socket()"
    int socketServeur = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addrServeur;
    addrServeur.sin_addr.s_addr = inet_addr("127.0.0.1");
    addrServeur.sin_family = AF_INET;
    addrServeur.sin_port = htons(50000);

    int opt = 1;
    int r = setsockopt(socketServeur, SOL_SOCKET, SO_REUSEADDR, 
                        &opt, sizeof(opt));
    
    //Attachement du socket avec "Bind()"
    if (bind(socketServeur, (const struct sockaddr *)&addrServeur, sizeof(addrServeur)) < 0)
        printf("Erreur dans bind \n");

    // Listen avec listen()
    if(listen(socketServeur, 5) <0)
       printf("Erreur dans listen \n");
    printf("listen\n");

    // Creation du thread
    pthread_t clientThread[NOMBRE_JOUEUR];
    for(int i = 0; i < NOMBRE_JOUEUR; i++){
        struct sockaddr_in addrClient;
        socklen_t csize = sizeof(addrClient);
        int socketClient= accept(socketServeur, (struct sockaddr *)&addrClient, &csize);
        printf("\taccept\n");
        //printf("\t\tJoueur : %d\n", socketClient);

        socketQst.socketClient = socketClient;
        socketQst.indice_Utilisateur = i;
        pthread_create(&clientThread[i], NULL, function_du_jeu, &socketQst);
    }
    for(int i = 0; i < NOMBRE_JOUEUR; i++)
    {
        pthread_join(clientThread[i], NULL);
    }
    close(socketServeur);

    // Chercher le meilleur score pour l'afficher
    int indice = meilleurScore(utilisateurs);
    printf("/-------- Le meilleur joueur est %s\n Son score est: %d -------/\n",
            utilisateurs[indice].nom_utilisateur, utilisateurs[indice].score);
    printf("Fin du jeu\n");
    return 0;
}
