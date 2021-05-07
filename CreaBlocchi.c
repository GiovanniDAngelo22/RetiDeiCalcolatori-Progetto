#include "function.c"

int main(int argc, char *argv[])
{

	int fd, conn, enable=1; 
	char buffer [MAXLINE]; 
	struct sockaddr_in nodon_add; 
	Blocco temp; 
	int tempo, nleft;
	int segnale=1; 
	//Creazione socket per il collegamento con Nodon
	fd=Socket(AF_INET, SOCK_STREAM,0); 
	//Imposto l'indirizzo 
	nodon_add.sin_family=AF_INET; 
	inet_aton(argv[1],(struct in_addr*)&nodon_add.sin_addr.s_addr); 
	nodon_add.sin_port=htons(atoi(argv[2])); 
	//Imposto che l'indirizzo è riutilizzabile
	setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(enable)); 
	//Mi connetto al NodoN
	Connect(fd,(struct sockaddr*)&nodon_add,sizeof(nodon_add)); 
	//Aspetta una risposta dal NodoN
	FullRead(fd,&segnale,sizeof(segnale)); 
	
	while(1)
	{
		//Creiamo il blocco e lo mettiamo in testa 
			temp=CreaBlocco();  
			if(temp.id_b!=0) //Non è un blocco error
			{

				FullWrite(fd,&segnale,sizeof(segnale)); //Comunico al NodoN che sto per inviargli un blocco
				//aspetto la conferma della ricezione del segnale
				if (FullRead(fd,&segnale,sizeof(segnale)) == 0)
					{
						printf("NodoN è Disconnesso, chiusura del CreaBlocchi.\n");
						exit(-1);
					}
				//Invio il nuovo blocco
				FullWrite(fd,&temp,sizeof(temp));
				printf("Inviato al NodoN\n");
			}
			else 
			{
				printf("Errore inserimento blocco\n");; 
			}
	}	
	return 0; 
}