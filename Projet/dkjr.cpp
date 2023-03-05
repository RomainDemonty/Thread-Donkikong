#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <SDL/SDL.h>
#include "./presentation/presentation.h"

#define VIDE        		0
#define DKJR       		1
#define CROCO       		2
#define CORBEAU     		3
#define CLE 			4

#define AUCUN_EVENEMENT    	0

#define LIBRE_BAS		1
#define LIANE_BAS		2
#define DOUBLE_LIANE_BAS	3
#define LIBRE_HAUT		4
#define LIANE_HAUT		5

void* FctThreadEvenements(void *);
void* FctThreadCle(void *);
void* FctThreadDK(void *);
void* FctThreadDKJr(void *);
void* FctThreadScore(void *);
void* FctThreadEnnemis(void *);
void* FctThreadCorbeau(void *);
void* FctThreadCroco(void *);

void initGrilleJeu();
void setGrilleJeu(int l, int c, int type = VIDE, pthread_t tid = 0);
void afficherGrilleJeu();

void HandlerSIGUSR1(int);
void HandlerSIGUSR2(int);
void HandlerSIGALRM(int);
void HandlerSIGINT(int);
void HandlerSIGQUIT(int);
void HandlerSIGCHLD(int);
void HandlerSIGHUP(int);

void DestructeurVS(void *p);

pthread_t threadCle;
pthread_t threadDK;
pthread_t threadDKJr;
pthread_t threadEvenements;
pthread_t threadScore;
pthread_t threadEnnemis;

pthread_cond_t condDK;
pthread_cond_t condScore;

pthread_mutex_t mutexGrilleJeu;
pthread_mutex_t mutexDK;
pthread_mutex_t mutexEvenement;
pthread_mutex_t mutexScore;

pthread_key_t keySpec;

bool MAJDK = false;
int  score = 0;
bool MAJScore = true;
int  delaiEnnemis = 4000;
int  positionDKJr = 1;
int  evenement = AUCUN_EVENEMENT;
int etatDKJr;

typedef struct
{
  int type;
  pthread_t tid;
} S_CASE;

S_CASE grilleJeu[4][8];

typedef struct
{
  bool haut;
  int position;
} S_CROCO;

// ------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	//int evt;

	ouvrirFenetreGraphique();

	afficherCage(1);
	afficherCage(2);
	afficherCage(3);
	afficherCage(4);

	/*
	afficherCroco(11, 2);
	afficherCroco(17, 1);
	afficherCroco(0, 3);
	afficherCroco(12, 5);
	afficherCroco(18, 4);
	*/

	//afficherDKJr(11, 9, 1);
	
	//effacerCarres(9, 10, 2, 1);

	afficherEchec(0);//1
	afficherScore(0);//1999

	pthread_create(&threadCle,NULL,(void*(*)(void*))FctThreadCle,NULL);
	pthread_create(&threadEvenements,NULL,(void*(*)(void*))FctThreadEvenements,NULL);
	pthread_create(&threadDKJr,NULL,(void*(*)(void*))FctThreadDKJr,NULL);
	/*
	while (1)
	{
	    evt = lireEvenement();

	    switch (evt)
	    {
		case SDL_QUIT:
			exit(0);
		case SDLK_UP:
			printf("KEY_UP\n");
			break;
		case SDLK_DOWN:
			printf("KEY_DOWN\n");
			break;
		case SDLK_LEFT:
			printf("KEY_LEFT\n");
			break;
		case SDLK_RIGHT:
			printf("KEY_RIGHT\n");
	    }
	}
	*/
	while(1)
	{

	}
}

// -------------------------------------

void initGrilleJeu()
{
  int i, j;   
  
  pthread_mutex_lock(&mutexGrilleJeu);

  for(i = 0; i < 4; i++)
    for(j = 0; j < 7; j++)
      setGrilleJeu(i, j);

  pthread_mutex_unlock(&mutexGrilleJeu);
}

// -------------------------------------

void setGrilleJeu(int l, int c, int type, pthread_t tid)
{
  grilleJeu[l][c].type = type;
  grilleJeu[l][c].tid = tid;
}

// -------------------------------------

void afficherGrilleJeu()
{   
   for(int j = 0; j < 4; j++)
   {
       for(int k = 0; k < 8; k++)
          printf("%d  ", grilleJeu[j][k].type);
       printf("\n");
   }

   printf("\n");   
}

void* FctThreadCle(void *)
{
	printf("Lancement du thread de la cle\n");
	int sens = 1;//savoir dans quelle sens la clé se déplace(0 = gauche , 1, droite)
	int i =1; //positionnement de la clé (1,2,3,4)
	struct timespec temps;//Le temps entre chaque mouvement de clé
      temps.tv_sec = 0;
      temps.tv_nsec = 700000000;
	while(1)
	{
		pthread_mutex_lock(&mutexGrilleJeu);
		grilleJeu[0][1].type = 0;
		effacerCarres(3,12,2,3);
		pthread_mutex_unlock(&mutexGrilleJeu);

		pthread_mutex_lock(&mutexGrilleJeu);
		grilleJeu[0][1].type = 4;
		pthread_mutex_unlock(&mutexGrilleJeu);

		afficherCle(i);
		if(sens)
		{
			i++;
			if(i == 4)
			{
				sens = 0;
			}
		}
		else
		{
			i--;
			if(i == 1)
			{
				sens = 1;
			}
		}
		nanosleep(&temps,NULL);
	}

	pthread_mutex_lock(&mutexGrilleJeu);
	grilleJeu[0][1].type = 0;
	effacerCarres(12,3,3,3);
	pthread_mutex_unlock(&mutexGrilleJeu);

	printf("Fin du thread de la cle\n");
}

void* FctThreadEvenements(void *)
{	
	struct timespec temps;//Le temps entre attente e mouvement
	temps.tv_sec = 0;
	temps.tv_nsec = 100000000;
	while (1)
	{
		pthread_mutex_lock(&mutexEvenement);
		evenement = lireEvenement();
		switch (evenement)
		{
			case SDL_QUIT:
				printf("Exit\n");
				exit(0);
			case SDLK_UP:
				printf("KEY_UP\n");
				break;
			case SDLK_DOWN:
				printf("KEY_DOWN\n");
				break;
			case SDLK_LEFT:
				printf("KEY_LEFT\n");
				break;
			case SDLK_RIGHT:
				printf("KEY_RIGHT\n");
				break;
		}
		pthread_mutex_unlock(&mutexEvenement);

		//Pas sur : 
		//kill(getpid(),SIGQUIT);
		nanosleep(&temps,NULL);

		pthread_mutex_lock(&mutexEvenement);
		evenement = AUCUN_EVENEMENT;
		pthread_mutex_unlock(&mutexEvenement);
	}
}

void HandlerSIGQUIT(int)
{
	/*
	pthread_mutex_lock(&mutexEvenement);
	if()
	pthread_mutex_unlock(&mutexEvenement);
	*/
}

void* FctThreadDKJr(void *)
{	
	bool on = true;
	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(3, 1, DKJR);
	afficherDKJr(11, 9, 1);
	etatDKJr = LIBRE_BAS;
	positionDKJr = 1;
	pthread_mutex_unlock(&mutexGrilleJeu);
	while (on)
	{
		pause();
		pthread_mutex_lock(&mutexEvenement);
		pthread_mutex_lock(&mutexGrilleJeu);
		switch (etatDKJr)
		{
			case LIBRE_BAS:
			switch (evenement)
			{
				case SDLK_LEFT:
				if (positionDKJr > 1)
				{
					setGrilleJeu(3, positionDKJr);
					effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
					positionDKJr--;
					setGrilleJeu(3, positionDKJr, DKJR);
					afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
				}
				break;
				case SDLK_RIGHT:
				
				break;
				case SDLK_UP:
				
				break;
			}
			case LIANE_BAS:
			
			break;
			case DOUBLE_LIANE_BAS:

			break;
			case LIBRE_HAUT:
			
			break;
			case LIANE_HAUT:
			
			break;
		}
	}
	pthread_mutex_unlock(&mutexGrilleJeu);
	pthread_mutex_unlock(&mutexEvenement);
	pthread_exit(0);
}
	