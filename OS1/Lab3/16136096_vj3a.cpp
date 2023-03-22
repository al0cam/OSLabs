#include <cstdlib>
#include <iostream>
#include <ctime>
#include <unistd.h>
#include <csignal>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <cstring>
#include <semaphore.h>

struct djeljeno
{
   int podjeljeno,procesi;
   sem_t semafor;
};



        int broj,*podjeljeno,id_podjeljeno,*procesi,id_procesa,br_procesa,id_d;
        djeljeno *d;
    
        int postavi(int broj_procesa, int broj_gl)
        {

            br_procesa=broj_procesa+1;
            broj=broj_gl;
            
            typedef int key_t;
            id_d=shmget(IPC_PRIVATE,sizeof(djeljeno),0600);
            d=(djeljeno*) shmat(id_d,NULL,0);
            d->podjeljeno=0;
            d->procesi=0;
            
            sem_init(&(d->semafor),1,0);
        }
        int generiraj()
        {
            std::cout<<"PROCES"<<" generiranje STVOREN"<<std::endl;
            srand(time(0));
            rand();
            int generirano,brojac=0;
            int vrijednost;
            while(brojac++<broj)
            {
                generirano=rand();
                d->podjeljeno=generirano;
                std::cout<<"Generiran broj: "<<d->podjeljeno<<std::endl;
                sem_post(&(d->semafor));
                sem_getvalue(&(d->semafor),&vrijednost);
                while(vrijednost){sem_getvalue(&(d->semafor),&vrijednost);};
            }
            d->podjeljeno=-1;
        }
        void racunaj()
        {
            int broj_procesa=d->procesi-1;
            std::cout<<"PROCES "<<broj_procesa<<". STVOREN"<<std::endl;
            int vrijednost;
            while(d->podjeljeno>=0)
            {
                sem_getvalue(&(d->semafor),&vrijednost);
                while(!vrijednost)
                {
                    if(d->podjeljeno<0) break;
                    sem_getvalue(&(d->semafor),&vrijednost);
                }
                if(d->podjeljeno<0) break;
                std::cout<<"PROCES "<<broj_procesa<<". preuzeo zadatak: "<<d->podjeljeno<<std::endl;
                int posebna_varijabla=d->podjeljeno;
                d->podjeljeno=0;

                sem_wait(&(d->semafor));

                int zbroj=0;
                for (int i = 0; i <= posebna_varijabla; i++)
                    zbroj+=i;
                std::cout<<"PROCES "<<broj_procesa<<". zbroj: "<<zbroj<<std::endl;
            }
        }
        void starter()
        {
            int i=0;
            for(i=0;i<br_procesa;i++)
                {
                    switch (fork())
                    {
                        case 0:
                            {
                                if((d->procesi)==0)
                                {
                                    (d->procesi)++;
                                    generiraj();
                                }
                                else
                                {
                                    (d->procesi)++;
                                    racunaj();
                                }
                                exit(0);
                            }
                        case -1: 
                            std::cout<<"nije mogao biti stvoren proces"<<std::endl;break;
                        default:
                            break;
                    }
                }
            for(i=0;i<d->procesi;i++)
                wait(NULL);
            
        }

        void ubi()
        {
            std::cout<<"Kill"<<std::endl;
            for(int i=0;i<br_procesa;i++) wait(NULL);
            sem_destroy(&(d->semafor));
            shmdt(d);
            shmctl(id_d,IPC_RMID,NULL);
        }


void brisi(int sig)
{
    ubi();
    exit(0);
}
int main(int argch,char **argv)
{
    sigset(SIGINT,brisi);
    int broj_procesa=atoi(argv[1]);
    int broj_elemenata=atoi(argv[2]); 
    postavi(broj_procesa,broj_elemenata);
    starter();
    ubi();
    return 0;
}


