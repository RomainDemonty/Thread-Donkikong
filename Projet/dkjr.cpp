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
	pthread_mutex_init(&mutexEvenement,NULL);
	pthread_mutex_init(&mutexGrilleJeu,NULL);
	pthread_mutex_init(&mutexDK,NULL);
	pthread_mutex_init(&mutexScore,NULL);

	pthread_cond_init(&condScore,NULL);
	pthread_cond_init(&condDK,NULL);

	sigAct.sa_handler = HandlerSIGQUIT;
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_flags = 0;
	sigaction(SIGQUIT,&sigAct,NULL);

	sigAct.sa_handler = HandlerSIGALRM;
	//sigemptyset(&sigAct.sa_mask);
	//sigAct.sa_flags = 0;
	sigaction(SIGALRM,&sigAct,NULL);

	sigAct.sa_handler = HandlerSIGINT;
	//sigemptyset(&sigAct.sa_mask);
	//sigAct.sa_flags = 0;
	sigaction(SIGINT,&sigAct,NULL);

	sigAct.sa_handler = HandlerSIGUSR1;
	//sigemptyset(&sigAct.sa_mask);
	//sigAct.sa_flags = 0;
	sigaction(SIGUSR1,&sigAct,NULL);

	pthread_key_create(&keySpec,NULL);

	srand((unsigned) time(NULL));

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

	pthread_create(&threadCle,NULL,(void*(*)(void*))FctThreadCle,NULL);
	pthread_create(&threadEvenements,NULL,(void*(*)(void*))FctThreadEvenements,NULL);
	pthread_create(&threadDK,NULL,(void*(*)(void*))FctThreadDK,NULL);
	pthread_create(&threadScore,NULL,(void*(*)(void*))FctThreadScore,NULL);
	pthread_create(&threadEnnemis,NULL,(void*(*)(void*))FctThreadEnnemis,NULL);

	//Sert a bloquer tous sauf ça et la on cherche a tout bloquer
	//Test1
	
	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGQUIT);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);

	//Test2
	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGALRM);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);
	
	//Test3
	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGINT);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);

	//Test4
	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGUSR1);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);
	

	while(vie < 3)
	{
		afficherEchec(vie);
		pthread_create(&threadDKJr,NULL,(void*(*)(void*))FctThreadDKJr,NULL);
		pthread_join(threadDKJr, (void **) NULL);
	}

	afficherEchec(vie);

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
				grilleJeu[0][1].type = 4;
				printf("Attrape moi !\n");
			}
			afficherCle(i);
		}
		pthread_mutex_unlock(&mutexGrilleJeu);
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
	//sert juste a quitter la pause pour les déplacements
}

void HandlerSIGINT(int)
{
	pthread_mutex_unlock(&mutexEvenement);
	printf("Aie j'ai été touché par un corbeau\n");
	effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);	
	vie++;

	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(2, positionDKJr, VIDE);
	pthread_mutex_unlock(&mutexGrilleJeu);
	
	pthread_exit(0);
}

void* FctThreadDKJr(void *)
{	
	/*
	sigset_t Mask_DKJR;
	sigfillset(&Mask_DKJR);
	sigdelset(&Mask_DKJR, SIGINT);
	sigprocmask(SIG_SETMASK, &Mask_DKJR, NULL);
	*/
	sigset_t set;
   	sigemptyset(&set);
    	sigaddset(&set, SIGINT);
	//sigaddset(&set, SIGQUIT);
   	pthread_sigmask(SIG_UNBLOCK, &set, NULL);
	
	struct timespec temps;//Le temps entre chaque decomposition de mouvement
	bool on = true;
	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(3, 1, DKJR);
	afficherDKJr(11, 9, 1);
	afficherGrilleJeu();
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
						temps.tv_sec = 1;
						temps.tv_nsec = 0;
						printf("Buisson\n");
						pthread_mutex_unlock(&mutexEvenement);
						pthread_mutex_unlock(&mutexGrilleJeu);
						nanosleep(&temps,NULL);
						
						pthread_mutex_lock(&mutexGrilleJeu);
						setGrilleJeu(3, positionDKJr);
						effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);

						pthread_mutex_unlock(&mutexGrilleJeu);
						afficherGrilleJeu();

						vie++;

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
				//si liane bas et que le corbeau est au dessus et on saute directement vie en moins
				if(grilleJeu[2][positionDKJr].type == 3)
				{
					pthread_mutex_unlock(&mutexEvenement);
					setGrilleJeu(3, positionDKJr, VIDE);
					pthread_mutex_unlock(&mutexGrilleJeu);
					printf("Aie j'ai été touché par un corbeau\n");

					//envoie de SIGUSR1
					pthread_kill(pthread_self(),SIGUSR1);

					effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);	
					vie++;
					
					pthread_exit(0);
				}

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
							setGrilleJeu(3, positionDKJr);
							effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
							setGrilleJeu(2,positionDKJr,DKJR);
							afficherDKJr(10, (positionDKJr * 2) + 7,8);
							etatDKJr = LIANE_BAS;

							pthread_mutex_unlock(&mutexGrilleJeu);
							temps.tv_sec = 1;
							temps.tv_nsec = 400000000;
							nanosleep(&temps,NULL);
							afficherGrilleJeu();

							pthread_mutex_lock(&mutexGrilleJeu);
							setGrilleJeu(2, positionDKJr);
							setGrilleJeu(3,positionDKJr,DKJR);
							effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
							afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
							etatDKJr = LIBRE_BAS;
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
					setGrilleJeu(2, positionDKJr,VIDE);
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
							afficherGrilleJeu();
						}
						else
						{
							if(positionDKJr == 3)
							{
								//Perte d'une vie si clé innacessible aussi non une cage en moins
								if(grilleJeu[0][1].type == 4)
								{
									temps.tv_sec = 0;
									temps.tv_nsec = 500000000;
									printf("Vous avez attrapé la clé\n");
									setGrilleJeu(1, positionDKJr);
									afficherGrilleJeu();
									positionDKJr--;

									effacerCarres(7,13,2,2);
									setGrilleJeu(0, positionDKJr,DKJR);
									afficherDKJr(5, 12,9);
									nanosleep(&temps,NULL);
									
									setGrilleJeu(0, positionDKJr,VIDE);
									positionDKJr--;
									setGrilleJeu(0, positionDKJr,DKJR);
									afficherGrilleJeu();
									
									effacerCarres(5,12,3,2);
									effacerCarres(3,12,2,3);
									setGrilleJeu(0, positionDKJr);
									afficherDKJr(3, 11,10);
									nanosleep(&temps,NULL);
									effacerCarres(3,11,3,2);

									pthread_mutex_unlock(&mutexEvenement);
									pthread_mutex_unlock(&mutexGrilleJeu);
									afficherGrilleJeu();

									//Changement du score
									pthread_mutex_lock(&mutexScore);
									//valeur a changer
									score = score +10;
									MAJScore = true;
									pthread_mutex_unlock(&mutexScore);
									pthread_cond_signal(&condScore);

									//Changement de la cage
									pthread_mutex_lock(&mutexDK);
									//valeur a changer
									MAJDK = true;
									pthread_mutex_unlock(&mutexDK);
									pthread_cond_signal(&condDK);

									pthread_exit(NULL);
								}
								else
								{
									temps.tv_sec = 0;
									//temps.tv_nsec = 500000000;
									temps.tv_nsec = 250000000;
									printf("Raté\n");

									
									//affichage de 11 12 13
									printf("M1\n");
									setGrilleJeu(1, positionDKJr);
									positionDKJr--;
									effacerCarres(7,13,2,2);
									setGrilleJeu(0, positionDKJr,DKJR);
									afficherDKJr(5, 12,9);
									pthread_mutex_unlock(&mutexGrilleJeu);
									afficherGrilleJeu();
									nanosleep(&temps,NULL);

									//M2
									printf("M2\n");
									pthread_mutex_lock(&mutexGrilleJeu);
									effacerCarres(5,12,3,2);
									setGrilleJeu(0, positionDKJr,VIDE);
									afficherDKJr(6, 11,12);
									setGrilleJeu(1, positionDKJr,DKJR);
									pthread_mutex_unlock(&mutexGrilleJeu);
									afficherGrilleJeu();
									nanosleep(&temps,NULL);

									//M3
									printf("M3\n");
									pthread_mutex_lock(&mutexGrilleJeu);
									effacerCarres(6,11,3,2);
									setGrilleJeu(1, positionDKJr,VIDE);
									setGrilleJeu(3, 0, DKJR);
									afficherDKJr(3, 11,13);
									pthread_mutex_unlock(&mutexGrilleJeu);
									afficherGrilleJeu();
									nanosleep(&temps,NULL);

									pthread_mutex_lock(&mutexGrilleJeu);
									effacerCarres(11,7,2,2);
									setGrilleJeu(3, 0, VIDE);
									afficherGrilleJeu();

									pthread_mutex_unlock(&mutexEvenement);
									pthread_mutex_unlock(&mutexGrilleJeu);
									afficherGrilleJeu();

									vie++;

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
							setGrilleJeu(1, positionDKJr, DKJR);
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

								pthread_mutex_unlock(&mutexGrilleJeu);
								temps.tv_sec = 1;
								temps.tv_nsec = 400000000;
								nanosleep(&temps,NULL);
								afficherGrilleJeu();

								pthread_mutex_lock(&mutexGrilleJeu);
								setGrilleJeu(0, positionDKJr);
								setGrilleJeu(1,positionDKJr,DKJR);
								effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
								afficherDKJr(7, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
								etatDKJr = LIBRE_HAUT;
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
		afficherGrilleJeu();
	}
	pthread_exit(0);
}
	
void* FctThreadDK(void *)
{
	struct timespec temps;
	temps.tv_sec = 0;
	temps.tv_nsec = 700000000;
	int NbCage = 4;
	while(1)
	{
		pthread_mutex_lock(&mutexDK);
		pthread_cond_wait(&condDK,&mutexDK);
		if(MAJDK == true)
		{
			pthread_mutex_lock(&mutexGrilleJeu);
			switch (NbCage) 
			{
				case 1 :
					NbCage = 4;
					effacerCarres(4,9,2,3);

					//Changement du score
					pthread_mutex_lock(&mutexScore);
					//valeur a changer
					score = score +10;
					MAJScore = true;
					pthread_mutex_unlock(&mutexScore);
					pthread_cond_signal(&condScore);

					afficherRireDK();
					nanosleep(&temps,NULL);

					effacerCarres(3,8,2,2);
					afficherCage(1);
					afficherCage(2);
					afficherCage(3);
					afficherCage(4);
				break;
				case 2 :
					effacerCarres(4,7,2,2);
					afficherCage(4);
					NbCage--;
				break;
				case 3 :
					effacerCarres(2,9,2,2);
					afficherCage(4);
					NbCage--;
				break;
				case 4 :
					effacerCarres(2,7,2,2);
					afficherCage(4);
					NbCage--;
				break;
			}
			MAJDK == false;
			pthread_mutex_unlock(&mutexGrilleJeu);
		}
		printf("J'ai bien reçu le changement de cage\n");
		pthread_mutex_unlock(&mutexDK);
	}

	return 0;
}

void* FctThreadScore (void *)
{
	int AncienScore = 0;
	afficherScore(0);
	while(1)
	{
		pthread_mutex_lock(&mutexScore);
		pthread_cond_wait(&condScore,&mutexScore);
		if(MAJScore == true)
		{
			if(AncienScore != score)
			{
				afficherScore(score);
				AncienScore = score;
			}
			MAJDK == false;
		}
		printf("J'ai bien reçu le score\n");
		pthread_mutex_unlock(&mutexScore);
	}
	return 0;
}

void HandlerSIGALRM(int)
{
	//sert juste a mettre a jour le delai de génération d'ennemi
	if(delaiEnnemis > 2500)
	{
		
		delaiEnnemis = delaiEnnemis -250;
		printf("DelaiEnnemis : %d\n",delaiEnnemis);
		alarm(15);
	}
}

void* FctThreadEnnemis (void *)
{
	//Test
	/*
	sigset_t Mask_Ennemis;
	sigfillset(&Mask_Ennemis);
	sigdelset(&Mask_Ennemis, SIGALRM);
	sigprocmask(SIG_SETMASK, &Mask_Ennemis, NULL);
	*/
	struct timespec temps;
	pthread_t threadCorbeau;
	pthread_t threadCroco;

	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGALRM);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);

	alarm(15);
	
	while(1)
	{
		temps.tv_sec = delaiEnnemis/1000;
		temps.tv_nsec = (delaiEnnemis%1000)*100000;

		nanosleep(&temps,NULL);

		if((rand()%2))
		{
			printf("Le croco arrive\n");
			//pthread_create(&threadCroco,NULL,(void*(*)(void*))FctThreadCroco,NULL);
		}
		else
		{
			printf("Le corbeau arrive\n");
			pthread_create(&threadCorbeau,NULL,(void*(*)(void*))FctThreadCorbeau,NULL);
		}
	}
}

void HandlerSIGUSR1(int)
{
	//Tue le thread Corbeau
	printf("Hello");
	int *pos = (int*) pthread_getspecific(keySpec) ;//Position du corbeau

	effacerCarres(9, ((*pos)*2) + 8, 2, 1);

	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(2,*pos, VIDE);
	pthread_mutex_unlock(&mutexGrilleJeu);

	free(pos);
	pthread_exit(NULL);
}

void* FctThreadCorbeau (void *)
{
	/*
	sigset_t Mask_Corbeau;
	sigfillset(&Mask_Corbeau);
	sigdelset(&Mask_Corbeau, SIGUSR1);
	sigprocmask(SIG_SETMASK, &Mask_Corbeau, NULL);
	*/

	sigset_t set;
   	sigemptyset(&set);
    	sigaddset(&set, SIGUSR1);
	//sigaddset(&set, SIGQUIT);
   	pthread_sigmask(SIG_UNBLOCK, &set, NULL);

	struct timespec temps;
      temps.tv_sec = 0;
      temps.tv_nsec = 700000000;

	//allocation dynamique
	int *p = (int*) malloc(sizeof(int));

	int coco = 0;
	*p = coco;
	//setspecific
	pthread_setspecific(keySpec,p);

	//initialiser ça position 
	while(coco < 8)//tant que il est bon dans ça position ça continue
	{
		pthread_mutex_lock(&mutexGrilleJeu);

		//effacer ces traces
		effacerCarres(9, ((coco-1)*2) + 8, 2, 1);
		setGrilleJeu(2,coco-1, VIDE);

		if(grilleJeu[2][coco].type != 1)
		{
			//afficher le nouveau
			afficherCorbeau( (coco*2) + 8,(coco%2)+1);
			setGrilleJeu(2,coco, CORBEAU,pthread_self());
		}
		else
		{
			pthread_mutex_unlock(&mutexGrilleJeu);
			//Envoie du signal SIGINT
			pthread_kill(threadDKJr,SIGINT);
			free(p);
			pthread_exit(NULL);
		}

		pthread_mutex_unlock(&mutexGrilleJeu);

		//Pause entre chaque déplacement
		nanosleep(&temps,NULL);

		//Incrémentation
		coco++;
		*p = coco;
		pthread_setspecific(keySpec,p);
	}

	printf("Le corbeau est parti\n");
	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(2,coco-1, VIDE);
	pthread_mutex_unlock(&mutexGrilleJeu);

	effacerCarres(9, ((coco-1)*2) + 8, 2, 1);
	free(p);
	pthread_exit(NULL);
}