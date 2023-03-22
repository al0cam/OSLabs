#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <cstring>

using namespace std;
int br,m;

struct shared
{
    pthread_mutex_t monitor;
    pthread_cond_t uvjet[2]; 
    int tr, zvr, ceka[2]; 
    int u_restoranu=-1; 
};
int id_podjeljeno; 
shared *s;
void postavi()
{
    typedef int key_t;
    id_podjeljeno=shmget(IPC_PRIVATE,sizeof(shared),0600);
    s=(shared*) shmat(id_podjeljeno,NULL,0);
}
void ispis(int vr, string tx)
{
    cout<<"Red Linuxasa:"; 
    for(int i=0;i<m;i++)
        if(i<s->ceka[0]) 		
            cout<<"L"; 
        else
            cout<<"-"; 

    cout<<"Red Microsoftasa:"; 
    for(int i=0;i<m;i++)
        if(i<s->ceka[1])
            cout<<"M";
        else
            cout<<"-";
    cout<<"Restoran:";
    if(s->u_restoranu==1) 
        for(int i=0;i<s->tr;i++)
            cout<<"M";
    else 
        for(int i=0;i<s->tr;i++)
            cout<<"L";
    cout<<"-->";
    if(vr==1)
        cout<<"M "<<tx;
    else
        cout<<"L "<<tx;
    cout<<endl;
}

void udzi(int &vr) 
{
    pthread_mutex_lock(&s->monitor); 
    s->ceka[vr]++;  

    while(s->u_restoranu==1-vr || s->zvr>=br) 
    {
        ispis(vr, "u red cekanja"); 
        pthread_cond_wait(&s->uvjet[vr], &s->monitor); 
    }           

    s->ceka[vr]--; 
    s->u_restoranu=vr; 
    s->tr++; 
    if(s->ceka[1-vr]>0) 
        s->zvr++;  
    ispis(vr," u restoran"); 
    pthread_mutex_unlock(&s->monitor); 
}

void izadzi(int &vr) 
{
    pthread_mutex_lock(&s->monitor); 
    s->tr--; 
    if(s->tr==0) 
    {
        if(s->ceka[1-vr]>0) 
        {
            s->u_restoranu=1-vr; 
            pthread_cond_broadcast(&s->uvjet[1-vr]); 
        }

        else 
        {
            s->u_restoranu=-1;
        }
        s->zvr=0; 
    }
    ispis(vr,"iz restorana");
    pthread_mutex_unlock(&s->monitor);
}


void programer1(int vr)
{
    usleep(rand()%100000); 
    udzi(vr);
    usleep(rand()%100000); 
    izadzi(vr);
}
void programer2(int vr)
{
    usleep(rand()%100000); 
    udzi(vr);
    usleep(rand()%100000); 
    izadzi(vr);
}

void brisi(int sig)
{
    pthread_mutex_destroy(&s->monitor); 
    for(int i=0;i<2;i++)
        pthread_cond_destroy(&s->uvjet[i]);   
    shmdt(s);
    shmctl(id_podjeljeno,IPC_RMID,NULL);
    exit(0);
}

int main(int argch,char **argv)
{
    if(argch != 3){
 		cout << "Mora biti 2 arg"<<endl;
 		exit(0);
 	}
    br=atoi(argv[1]);
    m=atoi(argv[2]);
    sigset(SIGINT,brisi);
    srand(time(0));
    postavi();
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
    pthread_mutex_init(&s->monitor,&attr); 
    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);
    for(int i=0;i<2;i++)
        pthread_cond_init(&s->uvjet[i],&cattr); 
    
    int polje[m*2];
    for(int i=0;i<m*2;i++) 
        polje[i]=i%2; 
    for(int i=0;i<m*2;i++)
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
    for(int i=0;i<m*2;i++)
        wait(NULL);
    brisi(1);
}
