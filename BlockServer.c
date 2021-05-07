#include "function.c"

int main(int argc, char *argv[])
{

	int list_fd, conn_fd, nodon_fd, blocchiInit, controllo; 
	int i, nread, enable=1, max_fd, choice = 1, recbuff, sendbuff,segnale=1;
	struct sockaddr_in serv_add, client, nodon_add, buff_add;
	socklen_t len;
	char buffer[MAXLINE]; 
	fd_set f_set;
	pid_t pid; 
	Nodo *head;
	Blocco temp, temp2;
	//Creo un descrittore per la Listen
	list_fd = Socket(AF_INET,SOCK_STREAM,0);
	//Impostazioni dell'indirizzo del BlockServer
	//Imposto la famiglia dell'indirizzo
	serv_add.sin_family=AF_INET; 
	//Imposto la porta
	serv_add.sin_port=htons(1026);
	//Imposto l'indirizzo IP
	inet_aton("127.0.0.1", (struct in_addr*)&serv_add.sin_addr.s_addr); 
	//Setto l'opzione per il riutilizzo dello stesso indirizzo
	setsockopt(list_fd,SOL_SOCKET,SO_REUSEADDR,&enable,sizeof(enable));  
	//Associo l'indirizzo con la socket
	Bind(list_fd,(struct sockaddr*)&serv_add,sizeof(serv_add)); 
	//definisco la lunghezza della coda di attesa
	Listen(list_fd,1026); 
	//Creo un descrittore per il NodoN a cui connettermi
	nodon_fd = Socket(AF_INET,SOCK_STREAM,0); 
	//Imposto la famiglia
	nodon_add.sin_family=AF_INET; 
	//Imposto la porta presa in input
	nodon_add.sin_port=htons(atoi(argv[2])); 
	//Imposto l'indirizzo preso in input
	if(inet_pton(AF_INET,argv[1],(struct sockaddr*)&nodon_add.sin_addr)  <0)
	{
		fprintf(stderr,"Errore inet_pton %s\n", argv[1]);
		exit(1); 
	}
	//Prendo in input l'identificativo dell'ultimo blocco registrato dal Blockserver
	blocchiInit = atoi(argv[3]);
	if (blocchiInit <0)
		{
			printf("Id blocco non valido\n");
			exit(-1);
		}
	//Connessione al NodoN 
	Connect(nodon_fd,(struct sockaddr*)&nodon_add, sizeof(nodon_add));
	//Dico al NodoN che sono connesso 
	FullRead(nodon_fd,&segnale,sizeof(int));
	//Invio il numero di blocchi iniziali
	FullWrite(nodon_fd, &blocchiInit, sizeof(blocchiInit)); 
	//Aspetto il numero di blocchi da dover leggere			
	FullRead(nodon_fd, &nread, sizeof(nread));
	//Ho ricevuto il numero di blocchi che dovrò leggere 
	for(i=0; i<nread; i++)
	{
		//Leggo un blocco 
		FullRead(nodon_fd, &temp, sizeof(temp)); 
		 //Lo inserisco in un nodo e poi lo inserisco in testa alla lista
		if(i==0)
		{
			//Se è la prima iterazione fa l'assegnamento della testa.
			head=CreaNodo(temp); 
		}
		else
			//Inserisce in testa
			InserimentoNodo(CreaNodo(temp),&head); 
	}
	if (nread >0)
	printf("Mi sono connesso ed ho ricevuto %d blocchi\n",nread);
	else 
		printf("Non ho ricevuto nuovi blocchi\n");
	//Ho ricevuto la Blockchain ma inversa (1-2-3) quindi la capovolgo (3-2-1)
	Reverse(&head); 	


	while(1)
	{	
		//Impostazioni del set di descrittori da controllare
		FD_ZERO(&f_set); 		
		FD_SET(nodon_fd,&f_set); 
		FD_SET(list_fd, &f_set); 
		//
		if (nodon_fd > list_fd)
			max_fd=nodon_fd;
		else
			max_fd = list_fd;
			
		Select(max_fd,&f_set,NULL,NULL,NULL); 

		if(FD_ISSET(nodon_fd,&f_set))
		{
			//Se ho ricevuto una nuova connessione con nodoN
				if (FullRead(nodon_fd,&temp,sizeof(temp)) == 0) //Se ho ricevuto un segnale di chiusura
				{
					//Esco, il BlockServer non può vivere se il NodoN è chiuso
					printf("Persa la connessione con NodoN, chiudere il BlockServer.\n");
					close(nodon_fd);
					exit (-1);
				}
				//Lo inserisco in testa alla blockchain. 	
				InserimentoNodo(CreaNodo(temp),&head); 
				printf("Inserito un nuovo blocco con ID: %d\n",head->block.id_b); 
		}

		if(FD_ISSET(list_fd,&f_set))
		{
		//Nuova connessione con un Client
		len=sizeof(client); 
		conn_fd=Accept(list_fd,(struct sockaddr*)&client,&len); 
		if(Fork()==0) //Sono il figlio
		{
			close(list_fd); 
			close(nodon_fd);
		
	
			//Dico al Client che sono connesso 
			FullWrite(conn_fd,&segnale,sizeof(segnale)); 
			inet_ntop(AF_INET,&client.sin_addr,buffer,sizeof(buffer));
			printf("Nuovo client connesso: IP %s, port %d\n",buffer,ntohs(client.sin_port));
			//Choice è inizializzata ad 1, quindi entrerà nel while la prima volta
			while (choice > 0 && choice < 5) 
			{
				//Considero la scelta del client
				if (FullRead(conn_fd,&choice,sizeof(choice)) == 0) //Se la connessione è caduta
				{
					printf("Connessione chiusa: IP %s, port %d\n",buffer,ntohs(client.sin_port));
					close(conn_fd);
					return 0;
				} 
				switch(choice)
				{
					case 1: 
						FullRead(conn_fd,&recbuff,sizeof(recbuff)); //Il numero di transazioni da voler leggere
						// controllo se posso inviarli 
						controllo=head->block.id_b - recbuff; //differenza fra la richiesta e quello che ho
						printf("Numero transazioni da scrivere a IP %s, porta %d valore: %d\n",buffer, ntohs(client.sin_port),recbuff); 
						FullWrite(conn_fd,&controllo,sizeof(controllo)); //Invio il controllo che mi dice se posso soddisfare la richiesta
		
						if(controllo>=0)
						{									//Se la differenza è maggiore o uguale di zero significa che ho sufficienti numero di blocchi 
							ScritturaNtransazioni(recbuff,conn_fd,head);
						}
						break; 
					case 2: 
						//Richiesta dell'ammontare totale scambiato all'interno della blockchain
						//Calcolo la somma totale
						sendbuff=ValMax(head); 
						printf("Invio a IP %s, porta %d valore: %d\n",buffer,ntohs(client.sin_port),sendbuff); 
						//Invio la somma totale
						FullWrite(conn_fd,&sendbuff,sizeof(sendbuff));
						break; 
					case 3: 
						//Ricevo l'id della transazione da visualizzare
						FullRead(conn_fd,&recbuff, sizeof(recbuff)); 
						//Invio la transazione richiesta
						ScritturaDettagli(recbuff, head, conn_fd); 
						break; 
					case 4: 
						//Ricevo l'indirizzo di cui vuole avere informazioni
						FullRead(conn_fd,&buff_add,sizeof(buff_add)); 
						//La function invierà prima il numero delle transazioni che rispettano la richiesta e poi le suddette transazioni
						ScritturaTransazioniIp(buff_add,head,conn_fd); 
						break; 
			} //fine switch
		}//fine while
			//Finite le richiesta
			printf("Connessione chiusa: IP %s, porta %d\n",buffer,ntohs(client.sin_port));
			close(conn_fd);
			return 0;

		}
		else
		{
			/* padre */
			close(conn_fd); 
		}
	}
	}

	return 0; 
}