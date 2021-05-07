#ifndef HEADER_H
#define HEADER_H
#include <stdlib.h>
#include <sys/types.h> /* predefined types */
#include <unistd.h> /* include unix standard library */
#include <arpa/inet.h> /* IP addresses conversion utililites */
#include <sys/socket.h> /* socket library */
#include <stdio.h> /* include standard I/O library */
#include <string.h>
#include <errno.h>
#include <time.h>
#include <syslog.h>
#include <stdbool.h>
#include "Wrapper.h"

typedef struct trax  //struttura che descrive la transazione 
{
	int id_trax; //numero progressivo
	int sum; //ammontare
	struct sockaddr_in mittente, destinatario; //ip e porta del mittente e del destinatario
}Trax; 

typedef struct blocco
{
	int id_b; //numero progressivo
	int tempo; //tempo di attesa
	Trax t; //transazione
}Blocco;

typedef struct nodo
{
	Blocco block; 
	struct nodo *next;
}Nodo; 

//Funzioni per la gestione della lista

//Crea un nuovo nodo da inserire in testa
void InserimentoNodo(Nodo *x, Nodo **Head);

//Crea un nuovo nodo da un blocco ricevuto 
Nodo* CreaNodo(Blocco b); 

//Funzione utile al BlockServer in quanto alla prima connessione con il NodoN riceve la lista al "contrario", questa
//funzione ci permette di renderla uguale a quella del NodoN
void Reverse(Nodo** head); 

//Funzione booleana che ritorna true se due indirizzi ip e porta sono uguali oppure no 
bool Check(struct sockaddr_in key, struct sockaddr_in current); 

//Dato un indirizzo ip-porta conta il numero di blocchi in cui è presente lo stesso
int RicercaIp(struct sockaddr_in key, Nodo* head); 

//Permette di inviare il numero di blocchi in cui è coinvolto l'indirizzo ip:porta trovato al Client
void ScritturaTransazioniIp(struct sockaddr_in key, Nodo* head, int fd);

//Permette di inviare le N ultime transazioni al Client
void ScritturaNtransazioni(int num, int fd, Nodo* head); 

//Funzione che ritorna il valore complessivamente scambiato con le transazioni memorizzate
int ValMax(Nodo *head); 

//Funzione che ritorna true se l'id transazione esiste nella blockchain
int RicercaId(int id, Nodo* head); 

//Funzione per inviare i dettagli di una specifica transazione ad un Client
void ScritturaDettagli(int id, Nodo* head, int fd);

//menù a video per il Client
void ShowMenu(); 

//Funzione attua a creare il solo Nodo Genesi utile per il NodoN
Nodo* NodoGenesi();

//Funzione che crea un blocco da input di tastiera
Blocco CreaBlocco(); 

//Funzione CLientEcho
void ClientEcho(FILE * filein, int socket);


//Funzione che dato un blocco stampa a video le informazioni 
void LetturaBlocco(Blocco b); 

//Ritorna un blocco errore identificato semplicemente dall'id del blocco 0 
Blocco BloccoError(); 



//Controlla che in un array di caratteri ci siano solo numeri
bool IsInt(char buffer []); 
#endif 
