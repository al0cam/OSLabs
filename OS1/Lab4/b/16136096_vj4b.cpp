#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <sys/shm.h>
#include <cstring>

struct shared
{
    pthread_mutex_t monitor;
    pthread_cond_t uvjet[2]; 
    int tr, red, ceka[2]; 
    int u_restoranu=-1; 
};
class programeri
{
    private:
        int id_podjeljeno; 
        int mjesta,ljudi;
        shared *s;
    public:
    void postavi(int mjesta_main, int ljudi_main)
    {
        mjesta=mjesta_main;
        ljudi=ljudi_main;
        typedef int key_t;
        id_podjeljeno=shmget(IPC_PRIVATE,sizeof(shared),0600);
        s=(shared*) shmat(id_podjeljeno,NULL,0);

        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&s->monitor,&attr); 

        pthread_condattr_t cattr;
        pthread_condattr_init(&cattr);
        pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
        for(int i=0;i<2;i++)
            pthread_cond_init(&s->uvjet[i],&cattr); 
    }
    void ispis(int vrsta, std::string pozicija)
    {
        std::cout<<"Red Linuxasa:"; 
        for(int i=0;i<ljudi;i++)
            if(i<s->ceka[0]) std::cout<<"L"; 
            else std::cout<<"-"; 

        std::cout<<"Red Microsoftasa:"; 
        for(int i=0;i<ljudi;i++)
            if(i<s->ceka[1]) std::cout<<"M";
            else std::cout<<"-";

        std::cout<<"Restoran:";

        if(s->u_restoranu==1) 
            for(int i=0;i<s->tr;i++) std::cout<<"M";
        else 
            for(int i=0;i<s->tr;i++) std::cout<<"L";
        std::cout<<"-->";

        if(vrsta==1) std::cout<<"M "<<pozicija;
        else std::cout<<"L "<<pozicija;
        std::cout<<std::endl;
    }

    void ulaz(int &vrsta) 
    {
        pthread_mutex_lock(&s->monitor); 
        s->ceka[vrsta]++;  

        while(s->u_restoranu==1-vrsta || s->red>=mjesta) 
        {
            ispis(vrsta, "u redu cekanja"); 
            pthread_cond_wait(&s->uvjet[vrsta], &s->monitor); 
        }           

        s->ceka[vrsta]--; 
        s->u_restoranu=vrsta; 
        s->tr++; 
        if(s->ceka[1-vrsta]>0) 
            s->red++;  
        ispis(vrsta," u restoranu"); 
        pthread_mutex_unlock(&s->monitor); 
    }
    void izlaz(int &vrsta) 
    {
        pthread_mutex_lock(&s->monitor);
        s->tr--; 
        if(s->tr==0) 
        {
            if(s->ceka[1-vrsta]>0) 
            {
                s->u_restoranu=1-vrsta; 
                //pthread_cond_broadcast(&s->uvjet[1-vrsta]); 
            	pthread_cond_signal(&s->uvjet[1-vrsta]);
            }
            else 
            {
                s->u_restoranu=-1;
            }
            s->red=0; 
        }
        ispis(vrsta,"iz restorana");
        pthread_mutex_unlock(&s->monitor);
    }
    void programer1(int vrsta)
    {
        usleep(rand()%100000); 
        ulaz(vrsta);
        usleep(rand()%100000); 
        izlaz(vrsta);
    }
    void programer2(int vrsta)
    {
        usleep(rand()%100000); 
        ulaz(vrsta);
        usleep(rand()%100000); 
        izlaz(vrsta);
    }
    void starter()
    {
        int polje[ljudi*2];
        for(int i=0;i<ljudi*2;i++) 
            polje[i]=i%2; 
        for(int i=0;i<ljudi*2;i++)
            if(i%2)
            {
                switch (fork())
                {
                    case 0:
                        {
                            programer2(polje[i]);
                            exit(0);
                        }
                    case -1: 
                        std::cout<<"nije mogao biti stvoren proces"<<std::endl;break;
                    default:
                        break;
                }
            }
            else
            {
                switch (fork())
                {
                    case 0:
                        {
                            programer1(polje[i]);
                            exit(0);
                        }
                    case -1: 
                        std::cout<<"nije mogao biti stvoren proces"<<std::endl;break;
                    default:
                        break;
                }
            }
        for(int i=0;i<ljudi*2;i++)
            wait(NULL);
    }

    ~programeri()
    {
        pthread_mutex_destroy(&s->monitor); 
        for(int i=0;i<2;i++)
            pthread_cond_destroy(&s->uvjet[i]);   
        shmdt(s);
        shmctl(id_podjeljeno,IPC_RMID,NULL);
    }
};

programeri *p=new programeri;
void brisi(int sig)
{
    delete p;
    exit(0);
}
int main(int argch, char **argv)
{
    sigset(SIGINT,brisi);
    int mjesta=atoi(argv[1]);
    int ljudi=atoi(argv[2]);
    p->postavi(mjesta,ljudi);
    p->starter();
    delete p;
}
