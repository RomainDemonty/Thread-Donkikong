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
#include<time.h>

//Questions: 
//

pthread_mutex_t mutexDonnee,mutexCompteur;
pthread_cond_t condCompteur;
pthread_key_t cle;
int compteur ;

typedef struct{
      char nom[20];
      int nbSecondes;
}DONNEE;

DONNEE data[]={
      "MATAGNE",15,
      "WILVERS",10,
      "WAGNER",17,
      "QUETTIER",8,
      "",0
};

DONNEE Param;

void*Slave( DONNEE *str);
pthread_t trheadHandle[5];
void handlerSignal(int sig);
struct sigaction sigAct;

int main()
{
      int i;

      pthread_mutex_init(&mutexDonnee,NULL);
      pthread_mutex_init(&mutexCompteur,NULL);
      pthread_cond_init(&condCompteur,NULL);

      pthread_key_create(&cle,NULL);

      compteur =0;
	//------------------------------------------------------------Création des processus-------------------------------------------

      for(i=0 ; i < 5 ; i++)
      {
            pthread_mutex_lock(&mutexDonnee);
            pthread_mutex_lock(&mutexCompteur);

            compteur++;

            memcpy(&Param,&data[i],sizeof(DONNEE));
            printf("Lancement du processus fils numero %d:\n",i+1); 
            pthread_create(&trheadHandle[i],NULL,(void*(*)(void*))Slave,&Param);

            pthread_mutex_unlock(&mutexCompteur);
      }

      //bloquage de sigint
      sigset_t sigpro;
      sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGINT);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);

	//------------------------------------------------------------Attente des retours-------------------------------------------

      /*
      for(i=0 ; i < 5 ; i++)
      {	
	      pthread_join(trheadHandle[i],(void**)NULL);
	      printf("Thread %d : Fini apres join\n",i+1);
      }
      */

      pthread_mutex_lock(&mutexCompteur);
      while(compteur !=0)
      {
            pthread_cond_wait(&condCompteur,&mutexCompteur);
      }
      pthread_mutex_unlock(&mutexCompteur);

	printf("Fin du thread principal\n");
	return 0;
}


//donner les instructions à exécuter si SIGINT : 
void handlerSignal(int sig)
{
      char *mal  = (char*)pthread_getspecific(cle);
	printf("\nIdentite du thread ayant reçu un signal SIGINT = %u.%u s occupant de %s\n",getpid(),pthread_self(),(char*)pthread_getspecific(cle));
}

void*Slave(DONNEE *str)
{
	//Changement de SIGINT
	sigAct.sa_handler = handlerSignal;
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_flags = 0;
	sigaction(SIGINT,&sigAct,NULL);

      DONNEE temp;
      char *mal = (char*) malloc(20*sizeof(char));
      strcpy(mal , str->nom);
      pthread_setspecific(cle,(void*)mal);

      memcpy(&temp,str,sizeof(DONNEE));
      pthread_mutex_unlock(&mutexDonnee);

      struct timespec temps,tr;
      temps.tv_sec = temp.nbSecondes;
      temps.tv_nsec = 0;
      
      while( nanosleep(&temps,&tr)==-1)
      {
            printf("\nIl restait %d secondes (redemarage)\n",tr.tv_sec);
            temps.tv_sec = tr.tv_sec;
            temps.tv_nsec = tr.tv_nsec;
      }

	printf("Thread %u.%u\n",getpid(),pthread_self());
      printf("Nom : %s\n",temp.nom);
      
	printf("thread %u.%u se termine\n\n\n",getpid(),pthread_self());
      pthread_mutex_lock(&mutexCompteur);
      compteur--;
      pthread_mutex_unlock(&mutexCompteur);
      pthread_cond_signal(&condCompteur);

	pthread_exit(NULL);
}
