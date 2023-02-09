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

//Questions: 
//Demander si c'est exit dans les fonction ou truc spécial au thread

struct par{
	int tab;
	char mot[10];
	char Fich[20];
};

void*fct(struct par *);

pthread_t trheadHandle1;
pthread_t trheadHandle2;
pthread_t trheadHandle3;
pthread_t trheadHandle4;

int main()
{
	int ret;//Vérifie si l'opération c'est bien passée
	int *rt; //Nombre d'occurence renvoyée;
	struct par Par1, Par2 , Par3 , Par4;

	//------------------------------------------------------------Création des processus-------------------------------------------
	Par1.tab = 0;
	strcpy(Par1.mot,"printf");
	strcpy(Par1.Fich,"Test1.txt");
	printf("Lancement du processus 1:\n"); 
	if((ret = pthread_create(&trheadHandle1,NULL,(void*(*)(void*))fct,&Par1))!=0)
	{
		printf("AieAieAie problème\n");
	}

	Par2.tab = 1;
	strcpy(Par2.mot,"for");
	strcpy(Par2.Fich,"Test1.txt");
	printf("Lancement du processus 2:\n"); 
	if((ret = pthread_create(&trheadHandle2,NULL,(void*(*)(void*))fct,&Par2))!=0)
	{
		printf("AieAieAie problème\n");
	}

	Par3.tab = 2;
	strcpy(Par3.mot,"while");
	strcpy(Par3.Fich,"Test1.txt");
	printf("Lancement du processus 3:\n"); 
	if((ret = pthread_create(&trheadHandle3,NULL,(void*(*)(void*))fct,&Par3))!=0)
	{
		printf("AieAieAie problème\n");
	}

	Par4.tab = 3;
	strcpy(Par4.mot,"Ajouter");
	strcpy(Par4.Fich,"test.txt");
	printf("Lancement du processus 4:\n"); 
	if((ret = pthread_create(&trheadHandle4,NULL,(void*(*)(void*))fct,&Par4))!=0)
	{
		printf("AieAieAie problème\n");
	}

	//------------------------------------------------------------Attente des retours-------------------------------------------
	rt = 0;
	ret = pthread_join(trheadHandle1,(void**)&rt);
	if(*rt != -1)
	{
		printf("Thread 1 : %d occurences\n",*rt);
		free(rt);
	}
	else
	{
		printf("Une erreur c est produite\n");
	}
	

	rt = 0;
	ret = pthread_join(trheadHandle2,(void**)&rt);
	if(*rt != -1)
	{
		printf("Thread 2 : %d occurences\n",*rt);
		free(rt);
	}
	else
	{
		printf("Une erreur c est produite\n");
	}

	rt = 0;
	ret = pthread_join(trheadHandle3,(void**)&rt);
	if(*rt != -1)
	{
		printf("Thread 3 : %d occurences\n",*rt);
		free(rt);
	}
	else
	{
		printf("Une erreur c est produite\n");
	}

	rt = 0;
	ret = pthread_join(trheadHandle4,(void**)&rt);
	if(*rt != -1)
	{
		printf("Thread 4 : %d occurences\n",*rt);
		free(rt);
	}
	else
	{
		printf("Une erreur c est produite\n");
	}
	printf("Fin du thread principal\n");
	return 0;
}

void*fct(struct par * Par)
{
	int i; //boucle pour savoir a partire de quelle élément on doit lire
	int taille; //taille dy fichier
	int T; //nombre de tab
	int f; //fichier
	char comp[11]; //Mot à comparer
	int *occ = (int*)malloc(sizeof (int));
	*occ = 0; //Nombre de fois que le mot est revenu
	int rc ; //nombre d'élément lu

	//ouvrir une première fois pour savoir la taille du fichier
	if((f = open(Par->Fich,O_RDONLY))== -1)
	{
		perror("Erreur de open()");
		*occ = -1;
		pthread_exit(occ);
	}
	else
	{
		//calcul de la taille
		if((taille = lseek(f,0,SEEK_END)) == -1)
		{
			perror("Erreur de lseek(1)");
			*occ = -1;
			pthread_exit(occ);
		}
		else
		{
			//printf("Taille : %d\n",taille);
			//fermeture une fois que on sait la taille
			if(close(f))
			{
				perror("Erreur de close()");
				*occ = -1;
				pthread_exit(occ);
			}
			else
			{
				for(i = 0 ; i < (taille - (strlen(Par->mot)-1)) ; i++)
				{
					//printf("JM 1 : %d\n",i);
					//open nom du fichier dans Par
					if((f = open(Par->Fich,O_RDONLY))== -1)
					{
						perror("Erreur de open()");
						*occ = -1;
						pthread_exit(occ);
					}
					else
					{
						//read
						//Je me demande si ça ne va pas posser problème avec les \0 et pq pas utiliser malloc
						//se déplacer au bonne endroit
						if((rc = lseek(f,i,SEEK_SET)) == -1)
						{
							perror("Erreur de lseek(1)");
							*occ = -1;
							pthread_exit(occ);
						}
						else
						{	
							//printf("JM 2 : %d\n",i);
							//read de la taille du mot
							if((rc = read(f,comp,strlen(Par->mot))) == -1)
							{
								perror("Erreur de lseek()");
								*occ = -1;
								pthread_exit(occ);
							}
							else
							{
								comp[rc] = '\0';
								if(strcmp(comp,Par->mot) == 0)
								{
									(*occ)++;
								}

								for(T = 0 ; T != Par->tab ;  T++)
								{
									printf("\t");
								}
								printf("*\n");

								//close
								if(close(f))
								{
									perror("Erreur de close()");
									*occ = -1;
									pthread_exit(occ);
								}
							}
						}
					}
				}
				//printf("fin de lecture du fichier");
				//printf("\nIl y a eu %dx le mot %s",occ,Par->mot);
			}
		}
	}
	printf("fin du thread %d\n",Par->tab+1);
	pthread_exit(occ);
	//return 0;
}