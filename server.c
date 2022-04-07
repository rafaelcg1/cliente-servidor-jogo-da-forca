#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/types.h>

#define BUFSZ 1024
#define version "v4" //versao optada a ser utilizada, também cabe o uso da IPV6, basta alterar para "v6"

char word[] = "redes"; //palavra a ser descoberta no jogo

void usage(int argc, char **argv) {
    printf("usage: %s <server port>\n", argv[0]);
    printf("example: %s 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {
    if (argc < 2) {
        usage(argc, argv);
    }
	//função que realiza o parse do socket
    struct sockaddr_storage storage;
    if (0 != server_sockaddr_init(version, argv[1], &storage)) {
        usage(argc, argv);
    }

	//cria um novo socket
    int s;
    s = socket(storage.ss_family, SOCK_STREAM, 0);
    if (s == -1) {
        logexit("socket");
    }

   // habilita o uso da mesma porta sucessivamente pelo servidor
    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) {
        logexit("setsockopt");
    }

	//bind
    struct sockaddr *addr = (struct sockaddr *)(&storage);
    if (0 != bind(s, addr, sizeof(storage))) {
        logexit("bind");
    }

    //listen
    if (0 != listen(s, 10)) {
        logexit("listen");
    }

    char addrstr[BUFSZ];
    addrtostr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    while (1) {
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        //cria o socket
        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1) {
            logexit("accept");
        }

        char caddrstr[BUFSZ];
        addrtostr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

	    int tamanho = strlen(word); //recebe o tamanho da palavra escolhida

	    char completed_word[tamanho+1];
	    memset(completed_word, '-', tamanho); //preenche a palavra inicialmente com - para mostrar ao cliente
	    completed_word[tamanho] = '\0';

        
	    char buf[BUFSZ];
   	    memset(buf, 0, BUFSZ);
    	buf[0] = WELCOME_CODE;
    	buf[1] = (char) tamanho;
    	size_t count = send(csock, buf, 2, 0); //inicio do jogo

        while(1) {
            memset(buf, 0, BUFSZ);
            count = recv(csock, buf, BUFSZ, 0); //recebe um palpite

            int message_type = buf[0];
            if(count == 0 || message_type != GUESS_CODE){
                logexit("ERROR: Erro ao receber guess");
            }
            char guess = buf[1];

            memset(buf, 0, BUFSZ);
            int n_occurrences = 0;
            for (int i = 0; i < tamanho; i++){ //verifica o número de ocorrencias do palpite
                if(word[i] == guess){
                    buf[n_occurrences+2] = i;
                    completed_word[i] = guess;
                    n_occurrences++;
                }
            }
            
            buf[0] = ANSWER_CODE;
            buf[1] = (char) n_occurrences;
            //envia a resposta ao palpite recebido pelo cliente
            if(!strcmp(word, completed_word)){
                memset(buf, 0, BUFSZ);
                buf[0] = FINAL_CODE;
                count = send(csock, buf, 1, 0); 
                if (count != 1) {
                    logexit("send");
                }
                break;
            } else {
                count = send(csock, buf, n_occurrences+2, 0);
                if (count != n_occurrences+2) {
                    logexit("send");
                }
            }
        }
        close(csock);
    }
    exit(EXIT_SUCCESS);
}
