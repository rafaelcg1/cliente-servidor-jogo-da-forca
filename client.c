#include "common.h" //biblioteca que reune as funções comuns a todos os codigos

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>


void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char **argv) {
	if (argc < 3) {
		usage(argc, argv);
	}
	//função que realiza o parse do socket
	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) {
		usage(argc, argv);
	}
	//cria um novo socket para o cliente
	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0);
	if (s == -1) {
		logexit("socket");
	}
	//conecta o socket ao servidor
	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}

	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("connected to %s\n", addrstr);
	//é mostrado o lugar de conexão do cliente
	char buf[BUFSZ];
	memset(buf, 0, BUFSZ);

	//inicia-se o jogo recebendo a mensagem com o tamanho da palavra
	size_t count = recv(s, buf, 2, 0);
	if (count == 0 || buf[0] != WELCOME_CODE) {
		logexit("ERROR: Erro ao receber tamanho da palavra.");
	}
	int word_size = buf[1];
	printf("%d\n", word_size);
	char word[BUFSZ];
	for (int i = 0; i < word_size; i++)
	{
		word[i] = '-';
	}
	word[word_size] = '\0';


	int done = 0;

	while(!done){
		memset(buf, 0, BUFSZ);
		buf[0] = GUESS_CODE;
		fgets(buf+1, BUFSZ-1, stdin);
		char letter = buf[1];
		size_t count = send(s, buf, 2, 0);//envia um palpite para o servidor
		if (count != 2) {
			logexit("send");
		}

		memset(buf, 0, BUFSZ);
		count = recv(s, buf, BUFSZ, 0); //recebe a resposta do servidor quanto ao palpite

		int message_type = buf[0];
		if (count == 0 || (message_type != ANSWER_CODE && message_type != FINAL_CODE)) {
			logexit("ERROR: Erro ao receber resposta.");
		}else{
			if(message_type == ANSWER_CODE){ //se o palpite for uma letra presente na palavra procura quantas ocorrências da letra existem
				int n_occurrences = buf[1];
				for(int i = 0; i < n_occurrences; i++){
					word[(int)buf[i+2]] = letter;
				}
			}
			if(message_type == FINAL_CODE){
				for(int i = 0; i < word_size; i++){
					if(word[i] == '-'){
						word[i] = letter;
					}
				}
				done = 1; //ao preencher completamente a palavra sai do while e fecha o cliente
			}
			printf("%s\n",word); //imprime a palavra atual com os palpites corretos ate então
		}
	}
	close(s);


	exit(EXIT_SUCCESS);
}
