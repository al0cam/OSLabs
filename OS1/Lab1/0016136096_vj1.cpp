#include<iostream>
#include<unistd.h>
#include<signal.h>
#include<ctime>
#define N 3
using namespace std;

int tekuci_prioritet=0;
int *prioritet=new int[N];

void obrada_prekida(int i)
{
   for (int j = 1; j <= 5; j++)
    {
        sleep(1);
        cout<<"Prekid razine: "<<i<<"\t"<<j<<"/5"<<endl;
    }
}
void prekidna_rutina(int signum)
{
 time_t vrijeme;
 int i=0;

 time(&vrijeme);
 
 
 switch (signum) 
{
   case SIGINT: i = 1; break; 
   case SIGQUIT: i = 2; break; 
   case SIGTSTP: i = 3; break;
   default: return;
}

 if (i>tekuci_prioritet)
 {
   cout<<endl<<"Prekidna rutina pozvana u:"<<endl<<ctime(&vrijeme)<<endl;
   cout<<endl<<"Zapoceo prekid razine: "<<i<<endl;
    prioritet[i]=tekuci_prioritet;
    tekuci_prioritet=i;

    sigrelse(SIGINT);
    sigrelse(SIGQUIT);
    sigrelse(SIGTSTP);
   
    obrada_prekida(i);

    cout<<"Zavrsen prekid razine: "<<i<<endl;
    sighold(SIGINT);
    sighold(SIGQUIT);
    sighold(SIGTSTP);
    tekuci_prioritet=prioritet[i];
 }
 
}

int main()
{
 int i=0;
 sigset (SIGINT, prekidna_rutina);
 sigset (SIGQUIT, prekidna_rutina);
 sigset (SIGTSTP, prekidna_rutina);
 while(i<20)
 {
  i++;
  sleep(1);
  cout<<"Glavni program: "<<"\t"<<i<<"/20"<<endl;
 }
 return 0;
}
