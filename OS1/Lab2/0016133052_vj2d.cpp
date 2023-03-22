#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/ipc.h>

using namespace std;

int Zajednicka_Memorija,Broj_Procesa;

class Spremnik{

	public:
	int Trazim[100], Broj[100];

};

Spremnik *Pokazivac_Spremnika;

void Kriticni_Odsjecak(int Argument){
    Pokazivac_Spremnika->Trazim[Argument] = 1;
    Pokazivac_Spremnika->Broj[Argument] = 0;

    for(int Index = 0; Index < Broj_Procesa; Index++) {
        
	if(Pokazivac_Spremnika->Broj[Argument] < Pokazivac_Spremnika->Broj[Index]){
			Pokazivac_Spremnika->Broj[Argument] = Pokazivac_Spremnika->Broj[Index];
		}
    }

    Pokazivac_Spremnika->Broj[Argument]++;
    Pokazivac_Spremnika->Trazim[Argument] = 0;

    for(int Index = 0; Index < Broj_Procesa; Index++) {

        while(Pokazivac_Spremnika->Trazim[Index] != 0){
		
        }

        while(Pokazivac_Spremnika->Broj[Index] != 0 && (Pokazivac_Spremnika->Broj[Index] < Pokazivac_Spremnika->Broj[Argument] || (Pokazivac_Spremnika->Broj[Index] == Pokazivac_Spremnika->Broj[Argument] && Index < Argument))){
            
        }
    }
}

void Izlaz_Iz_KO(int Argument){
    Pokazivac_Spremnika->Broj[Argument] = 0;
}

void Zadatak_Djeteta(int Argument_1){

    for(int Brojac = 1; Brojac <= 5; Brojac++){

        Kriticni_Odsjecak(Argument_1);

        for(int m = 1; m <= 5; m++){
            cout << "Proces " << Argument_1 + 1 << ": KO " << Brojac << " (" << m << "/5)" << endl;
            usleep(500000);
        }

        Izlaz_Iz_KO(Argument_1);
    }
}

void Praznjenje_Memorije(int Argument) {
   
	(void) shmdt((char*) Pokazivac_Spremnika);
	(void) shmctl(Zajednicka_Memorija, IPC_RMID, NULL);
	exit(0);
}

int main(int Broj_Argumenata, char** String_Argumenata){
	
	Broj_Procesa = stoi(String_Argumenata[1]);
	Zajednicka_Memorija = shmget(IPC_PRIVATE, sizeof(Spremnik), 0600);
	sigset(SIGINT, Praznjenje_Memorije);
	Pokazivac_Spremnika = (Spremnik*) shmat(Zajednicka_Memorija, NULL, 0);
	
    for(int i = 0; i < Broj_Procesa; i++){
 
        if (fork() == 0){
            Zadatak_Djeteta(i);
            exit(0);
        }
    }

    for(int i = 0; i < Broj_Procesa; i++){

	wait(NULL);
	
	}

    Praznjenje_Memorije(0);
    return 0;

}
