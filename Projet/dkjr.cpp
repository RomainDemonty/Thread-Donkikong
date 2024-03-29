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
	ouvrirFenetreGraphique();

	sigAct.sa_handler = HandlerSIGQUIT;
	sigemptyset(&sigAct.sa_mask);
	sigAct.sa_flags = 0;
	sigaction(SIGQUIT,&sigAct,NULL);

	sigAct.sa_handler = HandlerSIGALRM;
	sigaction(SIGALRM,&sigAct,NULL);

	sigAct.sa_handler = HandlerSIGINT;
	sigaction(SIGINT,&sigAct,NULL);

	sigAct.sa_handler = HandlerSIGHUP;
	sigaction(SIGHUP,&sigAct,NULL);

	sigAct.sa_handler = HandlerSIGCHLD;
	sigaction(SIGCHLD,&sigAct,NULL);

	pthread_key_create(&keySpec,NULL);

	pthread_mutex_init(&mutexEvenement,NULL);
	pthread_mutex_init(&mutexGrilleJeu,NULL);
	pthread_mutex_init(&mutexDK,NULL);
	pthread_mutex_init(&mutexScore,NULL);

	pthread_cond_init(&condScore,NULL);
	pthread_cond_init(&condDK,NULL);

	srand((unsigned) time(NULL));

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

	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGHUP);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);

	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGCHLD);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);

	while(vie < 3)
	{
		afficherEchec(vie);
		pthread_create(&threadDKJr,NULL,(void*(*)(void*))FctThreadDKJr,NULL);
		pthread_join(threadDKJr, (void **) NULL);
	}

	afficherEchec(vie);
	pthread_mutex_lock(&mutexGrilleJeu);
	printf("/********GAME OVER********/\n");

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

void HandlerSIGHUP(int)
{
	pthread_mutex_unlock(&mutexEvenement);
	printf("Aie j'ai été touché par un croco\n");
	effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);	
	vie++;

	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(1, positionDKJr, VIDE);
	pthread_mutex_unlock(&mutexGrilleJeu);
	
	pthread_exit(0);
}

void HandlerSIGCHLD(int)
{
	pthread_mutex_unlock(&mutexEvenement);
	printf("Aie j'ai été touché par un croco\n");
	effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);	
	vie++;

	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(3, positionDKJr, VIDE);
	pthread_mutex_unlock(&mutexGrilleJeu);
	
	pthread_exit(0);
}

void* FctThreadDKJr(void *)
{	
	sigset_t set;
   	sigemptyset(&set);
    	sigaddset(&set, SIGINT);
	sigaddset(&set, SIGHUP);
	sigaddset(&set, SIGCHLD);
   	pthread_sigmask(SIG_UNBLOCK, &set, NULL);
	int i;

	pthread_mutex_lock(&mutexGrilleJeu);
	
	for( i= 1 ; i < 4 ; i++)
	{
		if(grilleJeu[3][i].type == 2)
		{
			pthread_kill(grilleJeu[3][i].tid, SIGUSR2);
		}
	}
	
	for(i = 0 ; i < 3 ; i++)
	{
		if(grilleJeu[2][i].type == 3)
		{
			pthread_kill(grilleJeu[2][i].tid, SIGUSR1);
		}
	}
	
	
	struct timespec temps;//Le temps entre chaque decomposition de mouvement
	bool on = true;
	
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
						if(grilleJeu[3][positionDKJr-1].type != 2)
						{
							setGrilleJeu(3, positionDKJr);
							effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
							positionDKJr--;
							setGrilleJeu(3, positionDKJr, DKJR);
							afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
						}
						else
						{
							printf("Aie j'ai touché par un croco\n");

							//On met son ancienne position à zéro
							//pthread_mutex_lock(&mutexGrilleJeu);

							setGrilleJeu(3, positionDKJr, VIDE);

							pthread_mutex_unlock(&mutexGrilleJeu);
							afficherGrilleJeu();

							//On tue le croco et DKJR
							pthread_kill(grilleJeu[3][positionDKJr - 1].tid, SIGUSR2);

							//On Efface l'ancien DKJR
							effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
							vie++;
							pthread_mutex_unlock(&mutexEvenement);
							pthread_exit(0);
						}

					}

				}
				break;
				case SDLK_RIGHT:
				if(positionDKJr < 7)
				{
					if(grilleJeu[3][positionDKJr+1].type != 2)
					{
						setGrilleJeu(3, positionDKJr);
						effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
						positionDKJr++;
						setGrilleJeu(3, positionDKJr, DKJR);
						afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);	
					}
					else
					{
						printf("Aie j'ai touché par un croco\n");

						//On met son ancienne position à zéro
						//pthread_mutex_lock(&mutexGrilleJeu);

						setGrilleJeu(3, positionDKJr, VIDE);

						pthread_mutex_unlock(&mutexGrilleJeu);
						afficherGrilleJeu();

						//On tue le croco et DKJR
						pthread_kill(grilleJeu[3][positionDKJr + 1].tid, SIGUSR2);

						//On Efface l'ancien DKJR
						effacerCarres(11, (positionDKJr * 2) + 7, 2, 2);
						vie++;
						pthread_mutex_unlock(&mutexEvenement);
						pthread_exit(0);
					}
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
					pthread_kill(grilleJeu[2][positionDKJr].tid, SIGUSR1);

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

						if(grilleJeu[3][positionDKJr].type != 2)
						{
							pthread_mutex_lock(&mutexGrilleJeu);
							setGrilleJeu(2, positionDKJr);
							setGrilleJeu(3,positionDKJr,DKJR);
							pthread_mutex_unlock(&mutexGrilleJeu);
							effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
							afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
							etatDKJr = LIBRE_BAS;
						}
						else
						{
							printf("Aie j'ai été touché par un croco\n");

							//On met son ancienne position à zéro
							//pthread_mutex_lock(&mutexGrilleJeu);

							setGrilleJeu(2, positionDKJr, VIDE);

							pthread_mutex_unlock(&mutexGrilleJeu);
							afficherGrilleJeu();

							//On tue le croco et DKJR
							pthread_kill(grilleJeu[3][positionDKJr].tid, SIGUSR2);

							//On Efface l'ancien DKJR
							effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
							vie++;
							pthread_mutex_unlock(&mutexEvenement);
							pthread_exit(0);
						}
					}
				}
				break;
			}
			case LIANE_BAS:
			if(evenement == SDLK_DOWN)
			{
				if(grilleJeu[3][positionDKJr].type != 2)
				{
					setGrilleJeu(2, positionDKJr);
					effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
					setGrilleJeu(3, positionDKJr, DKJR);
					afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
					etatDKJr = LIBRE_BAS;
				}
				else
				{
					printf("Aie j'ai été touché par un croco\n");

					//On met son ancienne position à zéro
					//pthread_mutex_lock(&mutexGrilleJeu);

					setGrilleJeu(2, positionDKJr, VIDE);

					pthread_mutex_unlock(&mutexGrilleJeu);
					afficherGrilleJeu();

					//On tue le croco et DKJR
					pthread_kill(grilleJeu[3][positionDKJr].tid, SIGUSR2);

					//On Efface l'ancien DKJR
					effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
					vie++;
					pthread_mutex_unlock(&mutexEvenement);
					pthread_exit(0);
				}

			}
			break;
			case DOUBLE_LIANE_BAS:
				switch(evenement)
				{
					case SDLK_DOWN:
					if(grilleJeu[3][positionDKJr].type != 2)
					{
						setGrilleJeu(2, positionDKJr);
						effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
						setGrilleJeu(3, positionDKJr, DKJR);
						afficherDKJr(11, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
						etatDKJr = LIBRE_BAS;
					}
					else
					{
						printf("Aie j'ai été touché par un croco\n");

						//On met son ancienne position à zéro
						//pthread_mutex_lock(&mutexGrilleJeu);

						setGrilleJeu(2, positionDKJr, VIDE);

						pthread_mutex_unlock(&mutexGrilleJeu);
						afficherGrilleJeu();

						//On tue le croco et DKJR
						pthread_kill(grilleJeu[3][positionDKJr].tid, SIGUSR2);

						//On Efface l'ancien DKJR
						effacerCarres(10, (positionDKJr * 2) + 7, 2, 2);
						vie++;
						pthread_mutex_unlock(&mutexEvenement);
						pthread_exit(0);
					}
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
							if(grilleJeu[1][positionDKJr - 1].type != 2)
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
								printf("Aie j'ai été touché par un croco\n");

								//On met son ancienne position à zéro
								//pthread_mutex_lock(&mutexGrilleJeu);

								setGrilleJeu(1, positionDKJr, VIDE);

								pthread_mutex_unlock(&mutexGrilleJeu);
								afficherGrilleJeu();

								//On tue le croco et DKJR
								pthread_kill(grilleJeu[1][positionDKJr-1].tid, SIGUSR2);

								//On Efface l'ancien DKJR
								effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
								vie++;
								pthread_mutex_unlock(&mutexEvenement);
								pthread_exit(0);
							}
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
									temps.tv_nsec = 500000000;
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
						if(grilleJeu[1][positionDKJr+1].type != 2)
						{
							if(positionDKJr < 7)
							{
								setGrilleJeu(1, positionDKJr);
								effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
								positionDKJr++;
								setGrilleJeu(1, positionDKJr, DKJR);
								afficherDKJr(7, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
							}
							else
							{
								//if(positionDKJr == 7)
								//{
									setGrilleJeu(1, positionDKJr);
									effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
									positionDKJr++;
									setGrilleJeu(2, positionDKJr, DKJR);
									afficherDKJr(7, (positionDKJr * 2) + 7,6);
								//}
							}
						}
						else
						{
							printf("Aie j'ai touché par un croco\n");

							//On met son ancienne position à zéro
							//pthread_mutex_lock(&mutexGrilleJeu);

							setGrilleJeu(1, positionDKJr, VIDE);

							pthread_mutex_unlock(&mutexGrilleJeu);
							afficherGrilleJeu();

							//On tue le croco et DKJR
							pthread_kill(grilleJeu[1][positionDKJr + 1].tid, SIGUSR2);

							//On Efface l'ancien DKJR
							effacerCarres(7, (positionDKJr * 2) + 7, 2, 2);
							vie++;
							pthread_mutex_unlock(&mutexEvenement);
							pthread_exit(0);
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

								if(grilleJeu[1][positionDKJr].type != 2)//Si pas de croco en dessous
								{	
									
									pthread_mutex_lock(&mutexGrilleJeu);
									setGrilleJeu(0, positionDKJr);
									setGrilleJeu(1,positionDKJr,DKJR);
									effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
									afficherDKJr(7, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
									etatDKJr = LIBRE_HAUT;
								}
								else
								{
									printf("Aie j'ai été touché par un croco\n");

									//On met son ancienne position à zéro
									pthread_mutex_lock(&mutexGrilleJeu);

									setGrilleJeu(0, positionDKJr, VIDE);

									pthread_mutex_unlock(&mutexGrilleJeu);
									afficherGrilleJeu();

									//On tue le croco et DKJR
									pthread_kill(grilleJeu[1][positionDKJr].tid, SIGUSR2);

									//On Efface l'ancien DKJR
									effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
									vie++;
									pthread_mutex_unlock(&mutexEvenement);
									pthread_exit(0);
								}
							}
						}
					break;
				}
			break;
			case LIANE_HAUT:
				switch(evenement)
				{
					case SDLK_DOWN : 
						if(grilleJeu[1][positionDKJr].type != 2)
						{
							setGrilleJeu(0, positionDKJr);
							effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
							setGrilleJeu(1, positionDKJr, DKJR);
							afficherDKJr(7, (positionDKJr * 2) + 7,((positionDKJr - 1) % 4) + 1);
							etatDKJr = LIBRE_HAUT;
						}
						else
						{
							printf("Aie j'ai été touché par un croco\n");

							//On met son ancienne position à zéro
							//pthread_mutex_lock(&mutexGrilleJeu);//Dépend du cas (si le mouvement est décomposable on doit le lock)

							setGrilleJeu(0, positionDKJr, VIDE);

							pthread_mutex_unlock(&mutexGrilleJeu);
							afficherGrilleJeu();

							//On tue le croco et DKJR
							pthread_kill(grilleJeu[1][positionDKJr].tid, SIGUSR2);

							//On Efface l'ancien DKJR
							effacerCarres(6, (positionDKJr * 2) + 7, 2, 2);
							vie++;
							pthread_mutex_unlock(&mutexEvenement);
							pthread_exit(0);
						}
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

	afficherCage(1);
	afficherCage(2);
	afficherCage(3);
	afficherCage(4);

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
		afficherGrilleJeu();
	}

	return 0;
}

void* FctThreadScore (void *)
{
	afficherScore(0);
	while(1)
	{
		pthread_mutex_lock(&mutexScore);
		pthread_cond_wait(&condScore,&mutexScore);
		if(MAJScore == true)
		{
			printf("Score +10\n\n");
			afficherScore(score);
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
	struct timespec temps;
	pthread_t threadCorbeau;
	pthread_t threadCroco;

	sigemptyset(&sigpro);
	sigaddset(&sigpro,SIGALRM);
	sigprocmask(SIG_SETMASK,&sigpro,NULL);

	alarm(15);
	
	while(1)
	{
		temps.tv_sec = delaiEnnemis/1000 + 3;
		temps.tv_nsec = (delaiEnnemis%1000)*100000;

		nanosleep(&temps,NULL);

		if((rand()%2))
		{
			printf("Le corbeau arrive\n");
			pthread_create(&threadCorbeau,NULL,(void*(*)(void*))FctThreadCorbeau,NULL);//Crée un bug au niveau des scores
		}
		else
		{	
			printf("Le croco arrive\n");
			pthread_create(&threadCroco,NULL,(void*(*)(void*))FctThreadCroco,NULL);//Crée un bug au niveau des scores
		}
	}
}

void HandlerSIGUSR1(int)
{
	//Tue le thread Corbeau
	printf("Le corbeau est mort par DKJR\n");
	int *pos = (int*) pthread_getspecific(keySpec) ;//Position du corbeau

	effacerCarres(9, ((*pos)*2) + 8, 2, 1);

	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(2,*pos, VIDE);
	pthread_mutex_unlock(&mutexGrilleJeu);

	free(pos);

	pthread_cancel(pthread_self());
}

void* FctThreadCorbeau (void *)
{
	struct sigaction sa;
	sa.sa_handler = HandlerSIGUSR1;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGUSR1, &sa, NULL);

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

void HandlerSIGUSR2(int)
{
	
	//Tue le thread Cro
	printf("Le croco est mort pas DKJR\n");
	S_CROCO *pos = (S_CROCO*) pthread_getspecific(keySpec) ;//Position du croco

	if(pos->haut == true)//Si le croco est en haut
	{
		//printf("La position :%d\n\n",pos->position);
		effacerCarres(8, ((pos->position+1)*2) + 9, 1, 1);

		pthread_mutex_lock(&mutexGrilleJeu);
		setGrilleJeu(1,pos->position+2, VIDE);
		pthread_mutex_unlock(&mutexGrilleJeu);
	}
	else//Si le croco est en bas
	{
		effacerCarres(12, 22 - ((pos->position-7)*2), 1, 1);

		pthread_mutex_lock(&mutexGrilleJeu);
		setGrilleJeu(3,14-(pos->position), VIDE);
		pthread_mutex_unlock(&mutexGrilleJeu);
	}

	free(pos);

	pthread_cancel(pthread_self());
}

void* FctThreadCroco(void*)
{
	struct sigaction sa;
	sa.sa_handler = HandlerSIGUSR2;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGUSR2, &sa, NULL);


	S_CROCO Croco;
	Croco.position = 0;//Permet de savoir à quelle position se trouve le croco
	Croco.haut = true;
	struct timespec temps;
      temps.tv_sec = 0;
      temps.tv_nsec = 700000000;

	S_CROCO *p = (S_CROCO*) malloc(sizeof(S_CROCO));

	p->position = Croco.position;
	p->haut = Croco.haut;
	//setspecific
	pthread_setspecific(keySpec,p);

	while(Croco.position  < 14)
	{
		pthread_mutex_lock(&mutexGrilleJeu);
		if(Croco.haut == true)
		{
			if(grilleJeu[1][Croco.position+2].type != 1)//Si la position du croco est libre
			{
				if(Croco.position == 6)
				{
					//croco qui tombe
					effacerCarres(8, ((Croco.position)*2) + 9, 1, 1);
					setGrilleJeu(1,7,VIDE);
					//printf("Croco déplacement: \n");
					//afficherGrilleJeu();
					afficherCroco(22, 3);
					//Changement en false se haut
					Croco.haut = false;
				}
				else
				{
					effacerCarres(8, ((Croco.position)*2) + 9, 1, 1);
					setGrilleJeu(1,Croco.position +1,VIDE);
					setGrilleJeu(1,Croco.position + 2, CROCO,pthread_self());
					afficherCroco((Croco.position+1)*2 +9, Croco.position%2 +1);
					//printf("Croco déplacement: \n");
					//afficherGrilleJeu();
				}
			}
			else
			{
				setGrilleJeu(1,Croco.position +1,VIDE);
				pthread_mutex_unlock(&mutexGrilleJeu);
				//envoyer un message a DKJR
				pthread_kill(threadDKJr,SIGHUP);

				//désalouer la mémoire 
				free(p);

				//Effacer le croco
				effacerCarres(8, ((Croco.position)*2) + 9, 1, 1);

				//Se tuer
				pthread_exit(NULL);
			}

		}
		else
		{
			if(grilleJeu[3][14 - Croco.position].type != 1)//Si la position du croco est libre
			{
				if(Croco.position == 7)
				{
					effacerCarres(9, 23, 1, 1);
					setGrilleJeu(3,7, CROCO,pthread_self());
					afficherCroco(22 - ((Croco.position-7)*2), Croco.position%2 +3);
					printf("Croco déplacement: \n");
					//afficherGrilleJeu();
				}
				else
				{
					effacerCarres(12, 22 - ((Croco.position-8)*2), 1, 1);
					setGrilleJeu(3,14 - Croco.position+1, VIDE);
					setGrilleJeu(3,14 - Croco.position, CROCO,pthread_self());
					if(Croco.position%2 == 0)
					{
						afficherCroco(22 - ((Croco.position-7)*2), 5);
					}
					else
					{
						afficherCroco(22 - ((Croco.position-7)*2), 4);
					}
					
					printf("Croco déplacement: \n");
					//afficherGrilleJeu();
				}
			}
			else
			{
				//Effacer le croco
				if(Croco.position == 7)
				{
					effacerCarres(9, 23, 1, 1);
				}
				else
				{	
					setGrilleJeu(3,14 - Croco.position+1, VIDE);
					effacerCarres(12, 22 - ((Croco.position-8)*2), 1, 1);
				}

				pthread_mutex_unlock(&mutexGrilleJeu);

				//envoyer un message a DKJR
				pthread_kill(threadDKJr,SIGCHLD);

				//désalouer la mémoire 
				free(p);

				//Se tuer
				pthread_exit(NULL);
			}
		}
		p->position = Croco.position;
		p->haut = Croco.haut;
		pthread_setspecific(keySpec,p);
		pthread_mutex_unlock(&mutexGrilleJeu);
		nanosleep(&temps,NULL);
		Croco.position++;			
		printf("Déplacement Croco : \n");
		afficherGrilleJeu();
	}
	pthread_mutex_lock(&mutexGrilleJeu);
	setGrilleJeu(3,1, VIDE);
	//afficherGrilleJeu();
	pthread_mutex_unlock(&mutexGrilleJeu);
	effacerCarres(12, 22 - ((Croco.position-8)*2), 1, 1);
	printf("Croco disparu\n");

	free(p);
	
	pthread_exit(NULL);
}