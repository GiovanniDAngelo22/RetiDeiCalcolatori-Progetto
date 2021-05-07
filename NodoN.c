#include "function.c"

int main(int argc, char *argv[])
{
	srand(time(NULL)); //Creazione seed per il tempo di attesa random

	//Creazione nodo Genesi
	Nodo *head = NodoGenesi(); 
	int list_fd, conn_fd=-1, creablocchi_fd, i, buffnum, numBlocco,enable=1;
	struct sockaddr_in serv_addr, c_addr, cb_addr; 
	socklen_t len;            //dimensione di una socket
	ssize_t nread;
	fd_set f_set;	
	int max_fd; 	 //numero massimo di descrittori che la select deve monitorare
	int segnale; //variabile 
	char buffer[MAXLINE]; 
	Blocco temp;

	//Creazione socket per la listen
	list_fd=Socket(AF_INET,SOCK_STREAM,0); 
	serv_addr.sin_family=AF_INET; 
	//Prendere da linea di comando porta e ip 
	serv_addr.sin_port=htons(1025);
	inet_aton("127.0.0.1", (struct in_addr*)&serv_addr.sin_addr.s_addr); 

	setsockopt(list_fd,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(enable)); //permette il riutilizzo di indirizzi locali

	//Assegniamo la listen all'indirizzo 
	Bind(list_fd,(struct sockaddr*)&serv_addr,sizeof(serv_addr)); 

	//Mettiamo in ascolto il NodoN 
	Listen(list_fd, 1025); 
	puts("Attendo la connessione del creatore di blocchi.\n");
	creablocchi_fd = Accept(list_fd, (struct sockaddr*)&cb_addr, &len);
	//Se la Accept è riuscita
	if (creablocchi_fd != -1)
	puts("Connesso!\n");
	//Rispondo al CreaBlocchi che ci siamo connessi
	FullWrite(creablocchi_fd,&segnale,sizeof(segnale)); 
	
		

	while(1)
	{			

		FD_ZERO(&f_set); 			    //pulisce f_set
		FD_SET(creablocchi_fd,&f_set);  //CreaBlocchi.c
		FD_SET(list_fd,&f_set);
		if (list_fd > creablocchi_fd)
			max_fd=list_fd;
		else
			max_fd = creablocchi_fd;
		if(conn_fd>0)					//se il BlockServer si è connesso 
		{
			FD_SET(conn_fd,&f_set); //lo aggiungo alla select nel nuovo ciclo
			if(max_fd<conn_fd)//aggiorno max_fd
				max_fd=conn_fd; 

		}
		

		Select(max_fd+1,&f_set,NULL,NULL,NULL); 
	
		if(FD_ISSET(creablocchi_fd,&f_set)) 		//Se ci sono nuovi blocchi da aggiungere alla blockchain
		{
			//ricevo il segnale
			if(FullRead(creablocchi_fd,&segnale, sizeof(segnale))==0)
			{
				printf("Connessione con Crea Blocchi caduta, chiudo NodoN\n");
				exit(-1);
			}
			//Invio la conferma della ricezione del segnale
			FullWrite(creablocchi_fd,&segnale, sizeof(segnale)); 
			//Aspetto il blocco
			FullRead(creablocchi_fd,&temp,sizeof(temp)); 
			temp.id_b = 1 + head->block.id_b;  //Assegno al blocco ricevuto un Identificativo
			temp.t.id_trax = 1 + head->block.t.id_trax; //Assegno alla transazione un identificativo 
			temp.tempo=(rand()%10)+5; //Assegno un tempo di attesa casuale 
			printf("Tempo di attesa: %d\n",temp.tempo); 
			sleep(temp.tempo);
			InserimentoNodo(CreaNodo(temp),&head);
			printf("Ho ricevuto un nuovo blocco\n");
			LetturaBlocco(head->block);   //Visualizzo il nuovo blocco ricevuto
			if (conn_fd > 0) //Se il BlockServer è attivo
				{FullWrite(conn_fd,&temp,sizeof(temp));} //invio il nuovo blocco al BlockServer
			
		}
		

		if (FD_ISSET(list_fd,&f_set))
		{

			len=sizeof(c_addr); 
			conn_fd=Accept(list_fd, (struct sockaddr*)&c_addr, &len); 
			//Inserisco il BlockServer tra i descrittori da controllare nella Select
			FD_SET(conn_fd, &f_set);
			if(max_fd<conn_fd)
				max_fd=conn_fd; 
			//Mi assicuro che BlockServer sia effettivamente connesso
			FullWrite(conn_fd,&segnale, sizeof(segnale)); 
			FullWrite(STDOUT_FILENO,"\nE' stata effettuata una nuova connessione con il Blockserver\n",sizeof("\nE' stata effettuata una nuova connessione con il Blockserver\n")); 	
		}

		//Controllo se il descrittore del BlockServer è attivo solo se è connesso.
		if(conn_fd>0)
		{
		if(FD_ISSET(conn_fd,&f_set))
		{
				if (FullRead(conn_fd,&numBlocco,sizeof(numBlocco)) == 0) //Se il BlockServer è caduto
					{ 
						if (max_fd == conn_fd) //Aggiorno il max_fd
						{
							if (list_fd > creablocchi_fd)
								max_fd=list_fd;
							else
								max_fd=creablocchi_fd; 
						}
						printf("Il Blockserver si è disconnesso.\n");
						//Chiudo la connessione e reimposto il descrittore a -1 in modo da poter accettare una nuova connessione
						close(conn_fd); 
						conn_fd=-1; 

					}
					else 
					{
					if (Fork()==0) //Sono il figlio
					{
						close(creablocchi_fd);
						close(list_fd);

						printf("Ultimo blocco del Blockserver connesso: %d\n",numBlocco); 
						buffnum=head->block.id_b - numBlocco;
						if (numBlocco > buffnum)
						printf("Non è stato inviato nessun nuovo nodo\n");

						FullWrite(conn_fd,&buffnum,sizeof(buffnum)); 
						//Mi accerto che ha letto quanti ne sto per inviare
						
						if(buffnum>0)
						{
							ScritturaNtransazioni(buffnum,conn_fd,head); //invio tutte le transazioni richieste dal BlockServer
						}
					close(conn_fd);
					return 0;
					}
			}
		}
		}
		

	}
	return 0; 
}

