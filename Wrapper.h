#ifndef WRAPPER_H
#define WRAPPER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>

int Socket(int family, int type, int protocol)
{
int n;
if ( (n = socket(family, type, protocol)) < 0) {
perror("socket");
exit(1);
}
return(n);
}

int Connect(int socket,struct sockaddr *address, socklen_t address_len)
{
	int n;
  if ((n = connect(socket, address, address_len)) < 0) {
    perror("connect");
    exit(1);
  }
  return n;
}

int Bind(int socket, const struct sockaddr *address, socklen_t addrlen)
{
	int n;
	if ((n= bind(socket, address, addrlen)) < 0)
		{perror("bind");
			exit(1);
		}
		return n;
}

int Listen(int socket, int coda)
{
	int n;
	 if ( listen(socket, coda) < 0 ) {
    perror("listen");
    exit(1);
  }
  return n;
}

int Accept(int socket, struct sockaddr *clientaddr, socklen_t *addr_dim)
{
	int n;
	if ( ( n = accept(socket, clientaddr, addr_dim) ) < 0 ) {
      perror("accept");
      exit(1);
    }
    return n;
}

ssize_t FullWrite(int fd, const void *buf, size_t count)
  {
  size_t nleft;
  ssize_t nwritten;
  nleft = count;
  while (nleft > 0) // La scrittura è ripetuta nel ciclo fino all'esaurimento del numero di byte richiesti
  {
    if ((nwritten = write(fd, buf, nleft)) < 0)
      {
        if (errno == EINTR) // In caso di errore si controlla se è dovuto a un'interruzione della chiamata di sistema dovuta ad un segnale
          continue; // Ripeto l'accesso
        else
          return(nwritten); // Altrimenti l'errore viene ritornato al programma chiamante
      }
    nleft -= nwritten; // Setta la variabile left per la funzione write
    buf +=nwritten; // Setta il puntatore
  }
  return (nleft);
  }

ssize_t FullRead (int fd, void *buf, size_t count)
  {
  size_t nleft;
  ssize_t nread;
  nleft = count;
  while (nleft > 0) // La lettura è ripetuta nel ciclo fino all'esaurimento del numero di byte richiesti
    {

      if ((nread = read (fd, buf, nleft)) < 0)
        {
          if (errno == EINTR) // In caso di errore si controlla se è dovuto a un'interruzione della chiamata di sistema dovuta ad un segnale
            continue; // Ripeto l'accesso
      else
        return (nread); // Altrimenti l'errore viene ritornato al programma chiamante
        }
      else if (nread == 0) // Fine del file o socket chiusa
        return 0; // Si esce dal ciclo

      nleft -= nread; // Setta la variabile left per la funzione read
      buf += nread; // Setta il puntatore
    }
  return(nread);
  }

pid_t Fork()
  {
    pid_t pid;
    if ((pid = fork()) < 0)
    {
      perror("Fork Error");
      exit(1);
    }
    return pid;
}
int Select(int nfds,fd_set * readfds, fd_set * writefds,fd_set * errorfds,struct timeval * timeout)
{
  int readset;
  while((readset=select(nfds+1,readfds,(fd_set *)0,(fd_set *)0,NULL))<0)
   { 
   
     if(errno==EINTR)
       {
        continue;
       }
      else
       {
         perror("select");
         exit(1);
       }
    }
    return readset;
}

#endif