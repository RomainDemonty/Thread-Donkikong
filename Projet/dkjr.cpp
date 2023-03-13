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

//Nombre de vie
int vie = 0;

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

struct sigaction sigAct;
sigset_t sigpro;

// ------------------------------------------------------------------------

int main(int argc, char* argv[])
{
	sigAct.sa_handler = HandlerSIGQUIT;
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_flags = 0;
	sigaction(SIGQUIT,&sigAct,NULL);

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

	afficherEchec(vie);//1
	afficherScore(0);//1999

	pthread_create(&threadCle,NULL,(void*(*)(void*))FctThreadCle,NULL);
	pthread_create(&threadEvenements,NULL,(void*(*)(void*))FctThreadEvenements,NULL);

	while(vie < 3)
	{
		pthread_create(&threadDKJr,NULL,(void*(*)(void*))FctThreadDKJr,NULL);
		pthread_join(threadDKJr, (void **) NULL);
	}


	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGQUIT);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);

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
	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGQUIT);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);

	printf("Lancement du thread de la cle\n");
	int sens = 1;//savoir dans quelle sens la clé se déplace(0 = gauche , 1, droite)
	int i =1; //positionnement de la clé (1,2,3,4)
	struct timespec temps;//Le temps entre chaque mouvement de clé
      temps.tv_sec = 0;
      temps.tv_nsec = 700000000;
	while(1)
	{
		effacerCarres(3,12,2,3);
		if(sens)
		{
			i++;
			if(i == 4)
			{
				sens = 0;
			}
			afficherCle(i);
		}
		else
		{
			i--;
			if(i == 1)
			{
				sens = 1;
				pthread_mutex_lock(&mutexGrilleJeu);
				grilleJeu[0][1].type = 4;
				printf("Attrape moi !\n");
				pthread_mutex_unlock(&mutexGrilleJeu);
			}
			afficherCle(i);
		}
		nanosleep(&temps,NULL);
		pthread_mutex_lock(&mutexGrilleJeu);
		grilleJeu[0][1].type = 0;
		pthread_mutex_unlock(&mutexGrilleJeu);
	}

	pthread_mutex_lock(&mutexGrilleJeu);
	grilleJeu[0][1].type = 0;
	effacerCarres(3,12,2,3);
	pthread_mutex_unlock(&mutexGrilleJeu);

	printf("Fin du thread de la cle\n");
}

void* FctThreadEvenements(void *)
{	
	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGQUIT);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);

	struct timespec temps;//Le temps entre attente de mouvement
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
		pthread_kill(threadDKJr,SIGQUIT);
		nanosleep(&temps,NULL);

		pthread_mutex_lock(&mutexEvenement);
		evenement = AUCUN_EVENEMENT;
		pthread_mutex_unlock(&mutexEvenement);
	}
}

void HandlerSIGQUIT(int)
{
	//sert juste a quitter la pause
}

void* FctThreadDKJr(void *)
{	
	struct timespec temps;//Le temps entre chaque decomposition de mouvement
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
				if (positionDKJr > 0)
				{
					if(positionDKJr == 1)
					{
						//Si le singe tombe dans le buisson à droite sur la ligne du bas
						setGrilleJeu(3, positionDKJr);
						effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
						positionDKJr--;
						setGrilleJeu(3, positionDKJr, DKJR);
						afficherDKJr(11, (positionDKJr * 2) + 7,13);

						//Vie en moins
						//BUG
						temps.tv_sec = 2;
						temps.tv_nsec = 0;
						printf("Buisson\n");
						pthread_mutex_unlock(&mutexEvenement);
						pthread_mutex_unlock(&mutexGrilleJeu);
						nanosleep(&temps,NULL);
						//pthread_mutex_lock(&mutexEvenement);
						pthread_mutex_lock(&mutexGrilleJeu);
						setGrilleJeu(3, positionDKJr);
						effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
						//pthread_mutex_unlock(&mutexEvenement);
						pthread_mutex_unlock(&mutexGrilleJeu);
						vie++;
						afficherEchec(vie);
						pthread_exit(NULL);
					}
					else
					{
						setGrilleJeu(3, positionDKJr);
						effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
						positionDKJr--;
						setGrilleJeu(3, positionDKJr, DKJR);
						afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
					}

				}
				break;
				case SDLK_RIGHT:
				if(positionDKJr < 7)
				{
					setGrilleJeu(3, positionDKJr);
					effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
					positionDKJr++;
					setGrilleJeu(3, positionDKJr, DKJR);
					afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
				}
				break;
				case SDLK_UP:
				if(positionDKJr == 1 || positionDKJr == 5)
				{
					setGrilleJeu(3, positionDKJr);
					effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
					setGrilleJeu(2,positionDKJr,DKJR);
					afficherDKJr(10, (positionDKJr * 2) + 7,7);
					etatDKJr = LIANE_BAS;
				}
				else
				{
					if(positionDKJr == 7)
					{
						setGrilleJeu(3, positionDKJr);
						effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
						setGrilleJeu(2,positionDKJr,DKJR);
						afficherDKJr(10, (positionDKJr * 2) + 7,5);
						etatDKJr = DOUBLE_LIANE_BAS;
					}
					else
					{
						//Le faire sauter au dessus du buisson, puis le mettre sur la lianne de droite
						/*
						if(positionDKJr == 0)
						{
							//le faire sauter normalement
							setGrilleJeu(3, positionDKJr);
							effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
							setGrilleJeu(2,positionDKJr,DKJR);
							afficherDKJr(10, (positionDKJr * 2) + 7,8);

							//petite pause pour découper le mouvement
							pthread_mutex_unlock(&mutexGrilleJeu);
							temps.tv_sec = 0;
							temps.tv_nsec = 10000000000;
							nanosleep(&temps, NULL);
							pthread_mutex_lock(&mutexGrilleJeu);

							//le remettre sur la première liane de base à gauche
							setGrilleJeu(2, positionDKJr);
							effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
							positionDKJr++;
							setGrilleJeu(2,positionDKJr,DKJR);
							afficherDKJr(10, (positionDKJr * 2) + 7,7);
							etatDKJr = LIANE_BAS;
						}
						else
						{
						*/
							setGrilleJeu(3, positionDKJr);
							effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
							setGrilleJeu(2,positionDKJr,DKJR);
							afficherDKJr(10, (positionDKJr * 2) + 7,8);
							etatDKJr = LIANE_BAS;
						//}
					}
				}
				break;
			}
			case LIANE_BAS:
			if(evenement == SDLK_DOWN)
			{
				setGrilleJeu(2, positionDKJr);
				effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
				setGrilleJeu(3, positionDKJr, DKJR);
				afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
				etatDKJr = LIBRE_BAS;
			}
			break;
			case DOUBLE_LIANE_BAS:
				switch(evenement)
				{
					case SDLK_DOWN:
					setGrilleJeu(2, positionDKJr);
					effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
					setGrilleJeu(3, positionDKJr, DKJR);
					afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
					etatDKJr = LIBRE_BAS;
					break;
					case SDLK_UP:
					setGrilleJeu(2, positionDKJr);
					effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
					setGrilleJeu(1, positionDKJr, DKJR);
					afficherDKJr(7, (positionDKJr * 2) + 7,6);
					etatDKJr = LIBRE_HAUT;
					break;
				}
			break;
			case LIBRE_HAUT:
				switch(evenement)
				{
					case SDLK_DOWN : 
						if(positionDKJr == 7)
						{
							setGrilleJeu(1, positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
							setGrilleJeu(2, positionDKJr, DKJR);
							afficherDKJr(10, (positionDKJr * 2) + 7,5);
							etatDKJr = DOUBLE_LIANE_BAS;
						}
					break;
					case SDLK_LEFT:
						if(positionDKJr > 3)
						{
							setGrilleJeu(1, positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
							positionDKJr--;
							setGrilleJeu(1, positionDKJr, DKJR);
							afficherDKJr(7, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
						}
						else
						{
							if(positionDKJr == 3)
							{
								//Perte d'une vie si clé innacessible aussi non une cage en moins
								if(grilleJeu[0][1].type == 4)
								{
									printf("Vous avez attrapé la clé\n");
								}
								else
								{
									temps.tv_sec = 2;
									temps.tv_nsec = 0;
									printf("Raté\n");
									pthread_mutex_unlock(&mutexEvenement);
									pthread_mutex_unlock(&mutexGrilleJeu);
									nanosleep(&temps,NULL);
									pthread_mutex_lock(&mutexGrilleJeu);
									setGrilleJeu(1, positionDKJr);
									effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
									pthread_mutex_unlock(&mutexGrilleJeu);
									vie++;
									afficherEchec(vie);
									pthread_exit(NULL);
								}
							}
						}

					break;
					case SDLK_RIGHT: 
						if(positionDKJr < 6)
						{
							setGrilleJeu(1, positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
							positionDKJr++;
							setGrilleJeu(2, positionDKJr, DKJR);
							afficherDKJr(7, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
						}
						else
						{
							if(positionDKJr == 6)
							{
								setGrilleJeu(1, positionDKJr);
								effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
								positionDKJr++;
								setGrilleJeu(2, positionDKJr, DKJR);
								afficherDKJr(7, (positionDKJr * 2) + 7,6);
							}
						}
					break;
					case SDLK_UP : 
						if(positionDKJr == 6)
						{
							setGrilleJeu(1, positionDKJr);
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
							setGrilleJeu(0, positionDKJr, DKJR);
							afficherDKJr(6, (positionDKJr * 2) + 7,7);
							etatDKJr = LIANE_HAUT;
						}
						else
						{
							if(positionDKJr == 3 || positionDKJr == 4)
							{
								setGrilleJeu(1, positionDKJr);
								effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
								setGrilleJeu(0, positionDKJr, DKJR);
								afficherDKJr(6, (positionDKJr * 2) + 7,8);
								etatDKJr = LIANE_HAUT;
							}
						}
					break;
				}
			break;
			case LIANE_HAUT:
				switch(evenement)
				{
					case SDLK_DOWN : 
						setGrilleJeu(0, positionDKJr);
						effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
						setGrilleJeu(1, positionDKJr, DKJR);
						afficherDKJr(7, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
						etatDKJr = LIBRE_HAUT;
					break;
				}
			break;
		}
		pthread_mutex_unlock(&mutexEvenement);
		pthread_mutex_unlock(&mutexGrilleJeu);
	}
	pthread_exit(0);
}
	