#include "function.c"


int main(int argc, char *argv[])
{
	int sock, enable=1, segnale=1;
	struct sockaddr_in serv_add;
	//Creo il descrittore per la connessione con il BlockServer
	sock = Socket(AF_INET,SOCK_STREAM,0); 
	//Imposto la famiglia
	serv_add.sin_family=AF_INET; 
	//Imposto la porta da input
	serv_add.sin_port=htons(atoi(argv[2])); 
	//Imposto l'IP da input
	if(inet_pton(AF_INET,argv[1],(struct sockaddr*)&serv_add.sin_addr)  <0)
	{
		fprintf(stderr,"Errore inet_pton %s\n", argv[1]);
		exit(1); 
	}
	//Imposto la riusabilità dell'indirizzo associato
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(enable)); 
	//Mi connetto al BlockServer
	Connect(sock,(struct sockaddr*)&serv_add, sizeof(serv_add));
	//Ricevo un segnale
	FullRead(sock,&segnale, sizeof(segnale)); 
	//Avvio la funzione che gestisce la connessione con il BlockServer
	ClientEcho(stdin,sock);
}
