#include "header.h"
#define MAXLINE 1024

void InserimentoNodo(Nodo *x, Nodo **head)
{
	//Il nuovo nodo punterà alla testa corrente
	x->next=*head;
	//Il nuovo nodo diventa la testa
	*head=x; 
}

Nodo* CreaNodo(Blocco b)
{ 
	//Alloco la memoria per un nuovo nodo
	Nodo* newNodo = (Nodo*)malloc(sizeof(Nodo)); 
	//Assegno il blocco al nuovo nodo
	newNodo->block=b; 
	return newNodo; 
}

void Reverse(Nodo** head)
{
	//Function in place per invertire la lista in place.
	Nodo* prev = NULL;
	Nodo* current = *head;
	Nodo* next= NULL; 
	
	while(current!=NULL)
	{
		//Salvo il puntatore al prossimo elemento in una variabile
		next = current->next; 
		//Imposto il puntatore al prossimo elemento all'elemento precedente (inizialmente NULL)
		current->next = prev; 
		//Salvo l'elemento corrente in una variabile per usarlo nella prossima iterazione
		prev=current; 
		//Vado avanti nella lista
		current=next; 
	}
	*head = prev; 

}

int ValMax(Nodo *head)
{
	int somma=0; 
	Nodo* temp=head; 
	//Scrollo la lista e somme di tutte gli ammontare delle transazioni effettuate
	while(temp!=NULL)
	{
		somma += temp->block.t.sum; 
		temp=temp->next; 
	}

	return somma; 
}


bool Check(struct sockaddr_in key, struct sockaddr_in current)
{
	//Controllo se l'indirizzo e la porta combaciano
	if (key.sin_addr.s_addr == current.sin_addr.s_addr && key.sin_port==current.sin_port)
		return true;
	else
		return false;
}

int RicercaIp(struct sockaddr_in key, Nodo* head)
{
	int num=0; 
	Nodo* temp=head; 
	while(temp!=NULL)
	{
		//Scrollo la lista e confronto la struttura sockaddr_in ricevuta in input sia col destinatario che col mittente.
		if(Check(key,temp->block.t.mittente) == true || Check(key,temp->block.t.destinatario) == true)
			num++; 
		temp=temp->next; 
	}
	return num; 
}


int RicercaId(int id, Nodo* head)
{
	Nodo* temp=head; 
	//Scrollo la lista e cerco l'ID richiesto. Ritorno 1 se esiste.
	while(temp!=NULL) 
	{
		if(temp->block.t.id_trax!=id)
			temp=temp->next;
		else
			return 1;
	}
	return 0; 
}


void ScritturaTransazioniIp(struct sockaddr_in key, Nodo* head, int fd)
{
	int num=RicercaIp(key, head);  
	Blocco x; 
	Nodo* temp;
	//Invio al descrittore preso in input il numero di transazioni in cui è presente l'ip/porta richiesto
	FullWrite(fd,&num,sizeof(int)); 	
	if(num>0)
	{
		temp=head; 
		while(temp!=NULL && num>0)
		{
			//Scrollo la lista, se IP e Porta del mittente o del destinatario sono uguali a quelli presi in input li invio
			if(Check(key,temp->block.t.mittente) || Check(key,temp->block.t.destinatario))
			{
				x=temp->block; 
				FullWrite(fd, &x, sizeof(Blocco)); 
				num--; 
				temp=temp->next; 
			}
			else //Il Nodo considerato non ha IP/PORTA del mittente/destinatario uguale a quello in input.
			{
				temp=temp->next; 
			}
		}
	}
}


void ScritturaNtransazioni(int num, int fd, Nodo* head)
{
	Nodo* temp=head; 
	Blocco x; 
	while(temp!=NULL && num>0) //finché ci sono blocchi da inviare
	{
		x=temp->block; 
		//Invio i blocchi 
		FullWrite(fd, &x, sizeof(Blocco));
		num--; 
		temp=temp->next;
	}
}

void ScritturaDettagli(int id, Nodo* head, int fd)
{	
	//Cerco se esiste una transazione con l'ID richiesto
	int esito=RicercaId(id, head);
	Nodo* temp=head;
	Blocco x;
	//Invio al Client l'esito della ricerca
	FullWrite(fd, &esito, sizeof(esito)); 
	//Se non è 0, allora esiste quella transazione e procedo ad inviare i dettagli
	if(esito!=0) 						     
	{
		//Scrollo la lista cercando la transazione con l'id richiesto
		while(temp!=NULL)
			{	
				if(temp->block.t.id_trax!=id)
					temp=temp->next; 
				else
				{
					x=temp->block; 
					FullWrite(fd, &x, sizeof(Blocco)); 
					break; 			//L'id è univoco quindi dopo averlo trovato possiamo interrompere il loop
				}
			}
	}
}

void ShowMenu()
{
	printf("\n1)Visualizzare le ultime n transazioni\n"); 
	printf("2)Visualizzare il valore complessivamente scambiato con le transazioni\n"); 
	printf("3)Visualizzare i dettagli di una transazione da richiere\n"); 
	printf("4)Visualizzare le transazioni in cui è coinvolto l'indirizzo ip da richiere\n"); 
	printf("Inserire qualsiasi altro carattere per chiudere la connessione\n"); 
}


Nodo* NodoGenesi()
{
	//Creo un nodo fittizio con cui iniziare la blockchain
	Nodo *head=(Nodo*)malloc(sizeof(Nodo)); 
	head->block.t.id_trax=0; 
	head->block.t.sum=0;
	//Unico parametro impostato ad 1, utile per poterlo identificare rispetto ad un blocco Error
	head->block.id_b=1; 
	head->block.tempo=0;  
	head->block.t.mittente.sin_port=htons(0); 
	head->block.t.destinatario.sin_port=htons(0);
	head->block.t.mittente.sin_family=AF_INET; 
	head->block.t.destinatario.sin_family=AF_INET; 
	inet_aton("0.0.0.1", (struct in_addr*)&head->block.t.mittente.sin_addr.s_addr);
	inet_aton("0.0.0.1",(struct in_addr*)&head->block.t.destinatario.sin_addr.s_addr); 
	head->next=NULL;
	return head; 
}

Blocco BloccoError()
{
	//Blocco creato nel caso ci sia un errore nell'inserimento dei dati da parte di CreaBlocchi
	Blocco error; 
	error.id_b=0; 
	return error; 
}


Blocco CreaBlocco()
{
	char buffer[MAXLINE];
	Blocco b;
	b.id_b=1; 
	b.t.id_trax=1;
	/*Questi id sono temporanei, verranno corretti quando saranno mandati a NodoN */
	//Prendo in input tutti i parametri necessari per registrare una nuova transazione e controllo siano validi
	printf("\nInserire ammontare:\n"); 
	fgets(buffer, MAXLINE, stdin);
	if(IsInt(buffer)!= true) //Controllo se è effettivamente un numero
	{
		printf("Impossibile inserire questo ammontare.\n"); 
		b=BloccoError(); 
		return b;
	}  
	b.t.sum=atoi(buffer); 
	printf("\nHai inserito: %d\n",atoi(buffer)); 
	printf("\nInserire ip mittente:\n"); 
	fgets(buffer,MAXLINE,stdin); 
	if(inet_aton(buffer,(struct in_addr*)&b.t.mittente.sin_addr.s_addr)==0)
	{
		printf("Impossibile inserire questo indirizzo ip.\n"); 
		b=BloccoError(); 
		return b;
	}
	printf("\nHai inserito ip mittente: %s\n",buffer); 
	printf("\nInserire porta mittente:\n"); 
	fgets(buffer,MAXLINE,stdin);
	if(IsInt(buffer)!= true)
	{
		printf("Impossibile inserire questa porta.\n"); 
		b=BloccoError(); 
		return b;
	} 

	b.t.mittente.sin_port=htons(atoi(buffer)); 
	printf("\nHai inserito: %d\n",atoi(buffer));
	printf("\nInserire ip destinatario:\n"); 
	fgets(buffer,MAXLINE,stdin); 
	if(inet_aton(buffer,(struct in_addr*)&b.t.destinatario.sin_addr.s_addr)==0)
	{
		printf("Impossibile inserire questo indirizzo ip.\n"); 
		b=BloccoError(); 
		return b; 
	} 
	printf("\nHai inserito ip destinatario: %s\n",buffer); 
	printf("\nInserire porta destinatario:\n"); 
	fgets(buffer,MAXLINE,stdin);
	if(!IsInt(buffer))
	{
		printf("Impossibile inserire questa porta.\n"); 
		b=BloccoError(); 
		return b;
	} 
	b.t.destinatario.sin_port=htons(atoi(buffer)); 
	return b; 
}



void LetturaBlocco(Blocco b)
{
	printf("Id blocco: %d\n",b.id_b);
	printf("\nNumero transazione: %d\n", b.t.id_trax); 
	printf("Ammontare: %d\n", b.t.sum); 
	printf("Ip e porta mittente %s | %d\n", inet_ntoa(b.t.mittente.sin_addr),ntohs(b.t.mittente.sin_port));
	printf("Ip e porta destinatario %s | %d\n", inet_ntoa(b.t.destinatario.sin_addr),ntohs(b.t.destinatario.sin_port));
}

void ClientEcho(FILE * filein, int socket)
{
	int choice=0, intbuff, recbuff, controllo; 
	struct sockaddr_in buff_add; 
	char buffer [MAXLINE]; 
	Blocco temp; 
	while(1)
	{
		ShowMenu();
		//Prendo in input la scelta del client
		fgets(buffer,MAXLINE,filein);  
		choice=atoi(buffer); 
		printf("Hai scelto: %d\n", choice);
		//Invio la richiesta del client al Server
		FullWrite(socket,&choice,sizeof(choice)); 
		switch(choice)
		{
			case 1: //Leggere un numero da tastiera, inviare la richiesta al BlockServer e stampare a video i dati
				fputs("\nScrivere il numero di transazioni da leggere: \n", stdout); 
				fgets(buffer,MAXLINE,filein);   
				intbuff=atoi(buffer); 
				FullWrite(socket,&intbuff,sizeof(intbuff)); 
				//È possibile che il numero di transazioni richiesto eccede quelli effettivamente disponibili.
			    //riceveremo un valore -1
				if (FullRead(socket,&controllo, sizeof(controllo))==0)
				{
					printf("Connessione chiusa\n");
					exit(-1);
				} 
				if(controllo<0) 
					printf("Non ci sono %d transazioni da visualizzare.\n",intbuff); 
				else
				{
					while(intbuff>0) //Se la mia richiesta è soddisfatta so quanti Blocchi devo leggere
					{
						//Leggo i blocchi richiesti
						FullRead(socket,&temp,sizeof(Blocco));
						//Stampo le transazioni
						LetturaBlocco(temp);  
						intbuff--; 
					}
				}
				break;
			case 2: //Visualizzare il valore complessivamente scambiato con le transazioni
				//Attendo il valore richiesto
				if (FullRead(socket,&recbuff,sizeof(recbuff)) == 0)
				{					
					printf("Connessione chiusa\n");
					exit(-1);
				}
				printf("Valore complessivamente scambiato: %d\n",recbuff); 
				break;
			case 3: //Visualizzare i dettagli di una transazione da richiere
				printf("Scrivere il numero della transazione di cui si vuole leggere le informazioni: \n");
				//Prendo in input l'ID della transazione richiesta
				fgets(buffer, MAXLINE, filein); 
				//Converto la stringa in un intero
				intbuff=atoi(buffer); 
				//Invio l'id transazione 
				FullWrite(socket,&intbuff,sizeof(intbuff));
				//Potremmo ricevere -1 se non esiste la transazione richiesta
				if (FullRead(socket,&recbuff, sizeof(recbuff))==0)
									{
					printf("Connessione chiusa\n");
					exit(-1);
				} 
				if(recbuff==0)
					printf("L'id richiesto non esiste.\n"); 
				else
					{
						//Ricevo la transazione
						FullRead(socket,&temp,sizeof(Blocco)); 
						//La mostro a schermo
						LetturaBlocco(temp); 
					}
				break;
			case 4: //Visualizzare le transazioni in cui è coinvolto l'indirizzo ip da richiere
				printf("Scrivere l'indirizzo ip di cui si vogliono conoscere le transazioni:\n");
				//Prendo l'indirizzo IP 
				fgets(buffer, MAXLINE, filein); 
				//Lo metto in una struttura in_addr temporanea 
				inet_aton(buffer, (struct in_addr*)&buff_add.sin_addr.s_addr);
				fputs("Scrivere la porta associata all'indirizzo ip di cui si vogliono conoscere le transazioni: \n", stdout); 
				fgets(buffer, MAXLINE, filein); 
				//Inserisco la porta ricevuta in Input nella stessa struct temporanea
				buff_add.sin_port=htons(atoi(buffer)); 
				buff_add.sin_family=AF_INET; 
				//Invio l'indirizzo al BlockServer
				FullWrite(socket, &buff_add,sizeof(buff_add)); 
				//Ricevo il quantitativo di blocchi che dovrò leggere
				if (FullRead(socket,&recbuff,sizeof(recbuff))==0)
				{
					printf("Connessione chiusa\n");
					exit(-1);
				} 
				if(recbuff==0) //Se non ci sono blocchi con quell'IP
				{
					printf("Non ci sono transazioni in cui è coinvolto l'indirizzo richiesto.\n"); 
				}
				else
				{
					while(recbuff>0)
					{   //Leggo i blocchi richiesti
						FullRead(socket,&temp,sizeof(Blocco)); 
						//Li mostro a video
						LetturaBlocco(temp); 
						recbuff--; 
					}
				}
				break; 
			default: 
				//Se è stato ricevuto un input <1 o >4 la connessione si chiude
				printf("\nConnessione chiusa.\n"); 
				close(socket); 
				exit(1); 
				break; 
		}
	}
}

bool IsInt(char buffer [])
{
	int i;  
	for(i=0; i<strlen(buffer)-1; i++)
	{
		//Controllo carattere per carattere se è un numero da 0 a 9
		if(buffer[i]<48 || buffer[i]>57)
			return false;
	}
	return true; 
}