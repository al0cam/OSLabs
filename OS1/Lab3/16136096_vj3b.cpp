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

int SemId;   /* identifikacijski broj skupa semafora */

void SemGet(int n)
{  /* dobavi skup semafora sa ukupno n semafora */
   SemId = semget(IPC_PRIVATE, n, 0600);
   if (SemId == -1) {
      std::cout<<"Nema semafora!\n";
      exit(1);
   }
}

int SemSetVal(int SemNum, int SemVal)
{  /* postavi vrijednost semafora SemNum na SemVal */
   return semctl(SemId, SemNum, SETVAL, SemVal);
}

int SemGetVal(int SemNum)
{  /* citaj vrijednost semafora SemNum na SemVal */
   return semctl(SemId, SemNum, GETVAL, NULL);
}

int SemOp(int SemNum, int SemOp)
{  /* obavi operaciju SemOp sa semaforom SemNum */
   struct sembuf SemBuf;
   SemBuf.sem_num = SemNum;
   SemBuf.sem_op  = SemOp;
   SemBuf.sem_flg = 0;
   return semop(SemId, &SemBuf, 1);
}

void SemRemove()
{  /* uništi skup semafora */
   semctl(SemId, 0, IPC_RMID, 0);
}

int broj_prodavaca_global,broj_proizvoda_global;
void postavi_brojeve_sa_main(int broj_proizvodaca, int broj_elemenata_po_proizvodacu)
{
    broj_prodavaca_global=broj_proizvodaca;
    broj_proizvoda_global=broj_elemenata_po_proizvodacu;
}
struct something
{
    int polje[20];
    int procesi=0;
    int ulaz,izlaz,ukupno=0;
};

        int br_procesa,br_elemenata_po_proizvodacu,id;
        something *s;
    
    void postavi(int broj_proizvodaca, int broj_elemenata_po_proizvodacu)
    {
        br_procesa=broj_proizvodaca+1;
        br_elemenata_po_proizvodacu=broj_elemenata_po_proizvodacu;
        //djeljena mem
        typedef int key_t;
        id=shmget(IPC_PRIVATE,sizeof(something),0600);
        s=(something*) shmat(id,NULL,0);
        //stvaranje proizvodjaca
        SemGet(broj_proizvodaca);
    }

    void proizvodac()
    {
        int i=0;
        int broj_semafora=s->procesi-1;
        s->procesi++;
        srand(time(0));
        rand();
        std::cout<<"proizvodac: "<<broj_semafora+1<<std::endl;
        SemSetVal(broj_semafora,0);
        do
        {
            while(SemGetVal(broj_semafora)) 
            {
                sleep(1);
            }//ako sem pun
            s->polje[broj_semafora]=rand()/(broj_semafora+1);
            std::cout<<"Prizvodac "<<broj_semafora+1<<" je generirao broj: "<<s->polje[broj_semafora]<<std::endl;
            SemOp(broj_semafora,1);//postavi sem

        } while (i++<broj_proizvoda_global-1);
        
    }

    void potrosac()
    {
        s->procesi++;
        int i=0;
        std::cout<<"potrosac"<<std::endl;
        do
        {
            bool puno=false;
            while (!puno)
            {
                puno=true;
                for(int j=0;j<broj_prodavaca_global;j++)
                    if(!SemGetVal(j)) puno=false;
                sleep(1);
            }
            for(int j=0;j<broj_prodavaca_global;j++)
                {
                    std::cout<<"Potrosac je primio broj: "<< s->polje[j]<<std::endl;
                    s->ukupno+=s->polje[j];
                }
            for(int j=0;j<broj_prodavaca_global;j++)
                    SemOp(j,-1);
        } while (i++<broj_proizvoda_global-1);
        std::cout<<"Zbroj je: "<<s->ukupno<<std::endl;
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
                                if(s->procesi==0)
                                    potrosac();
                                else
                                    proizvodac(); 
                                exit(0);
                            }
                        case -1: 
                            std::cout<<"nije mogao biti stvoren proces"<<std::endl;break;
                        default:
                            break;
                    }
                }

            for(i=0;i<s->procesi;i++)
                wait(NULL);
            
        }
        void ubi()
        {
            std::cout<<"Kill"<<std::endl;
            for(int i=0;i<s->procesi;i++)
                wait(NULL);
            SemRemove();
            shmdt(s);
            shmctl(id,IPC_RMID,NULL);
        }

};



void brisi(int sig)
{
    ubi();
    exit(0);
}
int main(int argch,char **argv)
{
    sigset(SIGINT,brisi);

    int broj_proizvodaca=atoi(argv[1]);
    int broj_elemenata_po_proizvodacu=atoi(argv[2]); 

    postavi_brojeve_sa_main(broj_proizvodaca,broj_elemenata_po_proizvodacu);

    postavi(broj_proizvodaca,broj_elemenata_po_proizvodacu);
    starter();

    ubi();
    return 0;
}
