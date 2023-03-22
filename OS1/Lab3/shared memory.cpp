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


int n=3;
struct dijeljeno
{
   int broj[3];
   char slovo;
   sem_t semafor;
};

int main()
{
    int id;
    dijeljeno *d;
    id=shmget(IPC_PRIVATE,sizeof(dijeljeno),0600);
    d=(dijeljeno*) shmat(id,NULL,0);
    std::cout<<"radi"<<std::endl;
    std::cout<<"stvoreno"<<std::endl;
    int *polje;
    polje=new int[10];
    std::cout<<"stvoreno"<<std::endl;

    polje[1]=2;
    std::cout<<"funky stuff"<<std::endl;
    std::cout<<polje[1]<<std::endl;
    std::cout<<"funky stuff"<<std::endl;
    std::cout<<"broj: "<<d->broj[0]<<std::endl;
    std::cout<<"radi"<<std::endl;


   
   sem_init(&d->semafor,1,0);
   sem_post(&d->semafor);
   int vrijednost;
   sem_getvalue (&d->semafor,&vrijednost);
   sem_wait(&d->semafor);
   sem_destroy(&d->semafor);
   
   std::cout<<"Vrijednost semafora: "<<vrijednost<<std::endl;
    shmdt(d);
    shmctl(id,IPC_RMID,NULL);
    return 0;
}
