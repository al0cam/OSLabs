#include<pthread.h>
#include<cstdlib>
#include<iostream>
#include<unistd.h>
#include<csignal>
using namespace std;

class dekker
{
    private:
        int pravo=0,
            zastavica[2]={0,1};
        pthread_t tid;
    public:
        void ulaz_ko(int i,int j)
        {
            zastavica[i]=1;
            while (zastavica[j]!=0)
            {
                if(pravo==j)
                {
                    zastavica[i]=0;
                    while (pravo==j);
                    zastavica[i]=1;
                }
            }
        }
        void izlaz_ko(int i,int j)
        {
            pravo=j;
            zastavica[i]=0;
        }
        void izrada_dretve();
        ~dekker()
        {
            cout<<endl<<"ubijeno"<<endl;
            for(int i=0;i<2;i++)
                int pthread_detach(tid);
        }
};
dekker d;

void *dretva(void *arg)
{
    int i=*((int *)arg);
    int j = 1-i;
    int k=0;
    while (k++<5)
    {
        int m=0;
        d.ulaz_ko(i,j);
        while(m++<5)
        {
            cout<<"Dretva: "<<i+1<<" K.O. br: "<<k<<" ("<<m<<"/5)"<<endl;
            sleep(1);
        }
        d.izlaz_ko(i,j);
    }
    return NULL;
}
void dekker::izrada_dretve()
{
    int arg [2] = {0, 1};
    for(int i=0;i<2;i++)
    if(pthread_create(&tid,NULL,*dretva,&arg[i])<0)
    {
        printf("Pogreska\n");
        exit(1);
    }
    
    for(int i=0;i<2;i++)
        pthread_join(tid,NULL);
}
void izlaz(int sig)
{
    exit(0);
}
int main()
{
    sigset(SIGINT,izlaz);
    d.izrada_dretve();
    return 0;
}
