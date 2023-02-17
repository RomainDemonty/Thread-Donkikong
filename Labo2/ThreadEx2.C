/*
Demonty Romain 2201
*/

#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<fcntl.h>
#include<cstring>
#include<signal.h>

//Questions: 
//

//valeurs des exits:
//0 = rien ne c'est passé
//1 = signal sigint

void handlerSignal(int sig);
void handlerMaster(int sig);
void*Slave(void *);
void*Master(void*);
void fctFinMaster(void *p);

struct sigaction sigAct;

pthread_t trheadHandle1;
pthread_t trheadHandle2;
pthread_t trheadHandle3;
pthread_t trheadHandle4;
pthread_t trheadHandle5;

int main()
{
	int ret;//Vérifie si l'opération c'est bien passée
	int *rt; //Nombre d'occurence renvoyée;

	//armement d'un sigation du signal SIGINT sur un handler et celui qui reçevra affichera son identité et un message bien reçu
	//après la reception du message pthread_exit()
	struct sigaction sigAct;
	sigset_t sigpro;

	//Changement en SIGINT
	/*
	sigAct.sa_handler = handlerSignal;
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_flags = 0;
	sigaction(SIGINT,&sigAct,NULL); //
	*/
	
	//On bloque le signal SIGINT
	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGINT);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);

	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGUSR1);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);
	
	//------------------------------------------------------------Création des processus-------------------------------------------
	printf("Lancement du processus 1:\n"); 
	pthread_create(&trheadHandle1,NULL,(void*(*)(void*))Slave,NULL);

	printf("Lancement du processus 2:\n"); 
	pthread_create(&trheadHandle2,NULL,(void*(*)(void*))Slave,NULL);

	printf("Lancement du processus 3:\n"); 
	pthread_create(&trheadHandle3,NULL,(void*(*)(void*))Slave,NULL);

	printf("Lancement du processus 4:\n"); 
	pthread_create(&trheadHandle4,NULL,(void*(*)(void*))Slave,NULL);

	printf("Lancement du processus Master 5:\n"); 
	pthread_create(&trheadHandle5,NULL,(void*(*)(void*))Master,NULL);
	pause();


	//------------------------------------------------------------Attente des retours-------------------------------------------
	rt = 0;
	pthread_join(trheadHandle1,(void**)&rt);
	printf("Thread 1 : Fini de manière %d \n",*rt);
	free(rt);
	
	rt = 0;
	pthread_join(trheadHandle2,(void**)&rt);
	printf("Thread 2 : Fini de manière %d \n",*rt);
	free(rt);

	rt = 0;
	pthread_join(trheadHandle3,(void**)&rt);
	printf("Thread 3 : Fini de manière %d \n",*rt);
	free(rt);

	rt = 0;
	pthread_join(trheadHandle4,(void**)&rt);
	printf("Thread 4 : Fini de manière %d \n",*rt);
	free(rt);

	pthread_cancel(trheadHandle5);
	pthread_join(trheadHandle5,(void**)&rt);

	printf("Fin du thread principal\n");
	return 0;
}

//donner les instructions à exécuter si SIGINT : 
void handlerSignal(int sig)
{
	//Pas d'autres moyen ?
	int *Rt = (int*)malloc(sizeof(int));
	*Rt = 1;
	printf("Identité du thread ayant reçu un signal de Master = %u.%u\n",getpid(),pthread_self());
	pthread_exit(Rt);
}


void handlerMaster(int sig)
{
	kill(getpid(),SIGUSR1);
}

void*Slave(void *)
{
	sigset_t sigpro;

	//struct sigaction sigAct;
	sigAct.sa_handler = handlerSignal;
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_flags = 0;
	sigaction(SIGUSR1,&sigAct,NULL); 
	//On bloque le signal SIGINT
	
	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGINT);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);
	

	int *Rt = (int*)malloc(sizeof(int)); //Si une erreur ou valeur de retour

	//afficher son identité
	//pthread_self()
	printf("Identité du thread slave = %u.%u\n",getpid(),pthread_self());

	//pause en attendant un signal
	//pause()
	pause();


	printf("fin du thread \n");
 	
 	//se terminer et quitter
 	*Rt = 0;
	pthread_exit(Rt);
}

void*Master(void *)
{
	int *Rt = (int*)malloc(sizeof(int));
	pthread_cleanup_push(fctFinMaster,0);

	sigset_t sigpro;
	//On bloque le signal SIGUSR1
	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGUSR1);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);

	//struct sigaction sigAct;
	sigAct.sa_handler = handlerMaster;
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_flags = 0;
	sigaction(SIGINT,&sigAct,NULL); 

	//mise en place d'accord de destruction
	int ancEtat;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&ancEtat);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&ancEtat);

	

	printf("Identité du thread Master = %u.%u\n",getpid(),pthread_self());

	//attendre dans une pause
	while(1)
	{
		pause();
	}

	pthread_cleanup_pop(1);
}

void fctFinMaster(void *p)
{
	printf("Fin de master = %u.%u\n",getpid(),pthread_self());
}