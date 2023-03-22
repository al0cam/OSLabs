#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <sys/wait.h> 
#include <limits.h>
#include <sys/mman.h>
#include <sys/time.h>


//VRSTA PORUKE
#define ZATHJEV 1
#define ODGVOR 2
#define IZLAZ 3

// Ovo je od socketa
int so;
struct sockaddr_in sa;

//Ovo je za lamporta
int p;
int countProc = 0;
int numOfElems;
int id;
int broj_ulazaka_KO = 0;
int* red;

typedef struct{
        int id;
        int pocetna_vrijednost;
} proces;


proces proc;
int* T;
int** T_KO;

struct timeval tv;

void otvori() {
        so = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (so < 0) {
            exit(EXIT_FAILURE);
        }
        tv.tv_sec = 0;
        tv.tv_usec = 100000;
        if (setsockopt(so, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
            perror("Error");    
        }       
}

void pripregni() {
        sa.sin_family = AF_INET;
        sa.sin_port = htons(10000 + 0 * 10 + id);
        sa.sin_addr.s_addr = htonl(INADDR_ANY);
        if (bind(so, (struct sockaddr *) &sa, sizeof(sa)) < 0) {
                printf("Error: unable to bind\n");
                printf("Error code: %d\n", errno);
                exit(EXIT_FAILURE);
        }
}

void primi(int *j) {
        ssize_t vel = sizeof(sa);

        vel = recvfrom(so, &p, sizeof(p), 0, (struct sockaddr *) &sa, (socklen_t *) &vel);

        if(errno == 11){
            return;
        }
        if (vel < 0) {
                if (errno == EINTR) {
                        return;
                }
                exit(EXIT_FAILURE);
        }
        if (vel < sizeof(p)) {
                exit(EXIT_FAILURE);
        }
        *j = ntohs(sa.sin_port) - 10000 - 0 * 10;
}

void posalji(int j) {
        ssize_t vel;

        sa.sin_port = htons(10000 + 0 * 10 + j);
        sa.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        vel = sendto(so, &p, sizeof(p), 0, (struct sockaddr *) &sa, sizeof(sa));
        if (vel < 0) {
                exit(EXIT_FAILURE);
        }
        if (vel < sizeof(p)) {
                exit(EXIT_FAILURE);
        }
}


void KO(){
        int* proc_id;
        int numOfO = 0;
        int prethodi = 0;
        int count;

        p=ZATHJEV;
        red[id] = T[id];

        for(count=0;count<countProc;count++){
                if(count==id){
                        continue;
                }
                posalji(count);
                printf("P%d je posalo Z(%d, %d) k P%d\n",id,id,T[id],count);
                fflush(stdout);
        }

        do{
                primi(proc_id);
                if(proc_id!=NULL){
                        if(p==ZATHJEV){
                                printf("P%d primio Z(%d,%d) od P%d\n",id,*proc_id,T[*proc_id],*proc_id);
                                fflush(stdout);
                                T[id] = T[*proc_id];
                                printf("T(%d) = %d\n",id,T[id]);
                                numOfElems++;
                                red[*proc_id] = T[*proc_id];
                                p = ODGVOR;
                                posalji(*proc_id);
                                printf("P%d je posalo O(%d, %d) k P%d\n",id,id,T[id],*proc_id);
                                fflush(stdout);
                        }
                        else if(p==IZLAZ){
                                //obrisi zahtjev od j
                                numOfElems--;
                                red[*proc_id]=-1;
                                printf("P%d primio I(%d,%d) od P%d\n",id,*proc_id,T[*proc_id],*proc_id);
                                fflush(stdout);
                        }
                        else{
                                T[id] = T[*proc_id];
                                numOfO++;
                                printf("P%d primio O(%d,%d) od P%d\n",id,*proc_id,T[*proc_id],*proc_id);
                                fflush(stdout);
                        }
                        prethodi=1;
                        for(count=0; count<countProc; count++){
                                //vlastiti Z mora prethodit svim drugim
                                if(count == id) continue;
                                if(red[count]==-1) continue;
                                if(red[id]>=red[count]){
                                        prethodi=0;
                                }
                        }
                }
        }while( (numOfO!=(countProc-1)) && prethodi==0);

        printf("P%d je usao u KO\n",id);
        fflush(stdout);
        sleep(3);

        p=IZLAZ;

        for(count = 0; count<countProc;count++){
                if(count == id){
                        continue;
                }
                printf("P%d je posalo I(%d, %d) k P%d\n",id,id,T[id],count);
                fflush(stdout);
                posalji(count);
        }
        printf("P%d je izasao iz KO\n",id);
        fflush(stdout);
        numOfElems=0; 
}

void prazanRed(){
        int i;
        for(i=0;i<countProc;i++){
                red[i]=-1;
        }
}

void posao(){
        int broj_ko = 0;
        int udi_u_ko = 0;
        int count;
        int zahtjev = 0;
        numOfElems = 0;
        int* proc_id;
        red = calloc(countProc, sizeof(int));
        prazanRed();
        otvori();
        pripregni();
        sleep(2);
        while(1){
                if(T[id]>=T_KO[id][broj_ko] && numOfElems==0 && T_KO[id][broj_ko]!=0){
                        broj_ko++;
                        KO();
                }                        
                primi(proc_id);
                if(proc_id!=NULL && p==ZATHJEV && errno==0){
                        printf("P%d je primio Z(%d, %d) od P%d\n",id,*proc_id,T[*proc_id],*proc_id);
                        fflush(stdout);
                        T[id]=T[*proc_id];
                        red[numOfElems] = *proc_id;
                        numOfElems++;
                        p = ODGVOR;
                        posalji(*proc_id);
                        printf("P%d je posalo O(%d, %d) k P%d\n",id,id,T[id],*proc_id);
                        fflush(stdout);
                }
                else if(proc_id!=NULL && p==IZLAZ && errno==0){
                        red[*proc_id] = -1;
                        numOfElems--;
                        printf("P%d je primio I(%d, %d) od P%d\n",id,*proc_id,T[*proc_id],*proc_id);
                        fflush(stdout);
                }
                sleep(1);
                T[id]=T[id]+1;
                printf("Dogadaj(%d)\n",id);
                printf("T(%d) = %d\n",id,T[id]);
                fflush(stdout);
        }
}

int main(int argc, char* argv[]){

        //PARSIRANJE AGRUMENATA I DODAVANJE U PROCESE
        int count;
        int countAt = 0;
        int numOfAt = 0;
        int prvi = 1;
        broj_ulazaka_KO = 0;
        for(count = 1; count<argc;count++){
                if(argv[count]==NULL) break;
                if(*argv[count]=='@'){
                        break;
                }
                countProc++;
        }
        printf("Broj procesa: %d", countProc);
        printf("\n");
        fflush(stdout);
        for(count = 1; count<argc;count++){
                if(*argv[count]=='@'){
                        numOfAt++;
                }
        }
        printf("Broj @: %d", numOfAt);
        printf("\n");
        fflush(stdout);

        int prviAt=0;
        for(count = 1; count<argc;count++){
                if(argv[count]==NULL) break;
                if(*argv[count]=='@'){
                        prviAt++;
                        continue;
                }
                if(prviAt==1){
                        broj_ulazaka_KO++;
                }
        }

        printf("Broj ulazaka u KO: %d", broj_ulazaka_KO);
        printf("\n");
        fflush(stdout);

        //PROCES I VREMENA
        proces procesi[countProc];
        T = mmap(NULL, countProc*sizeof(int*), PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        T_KO = (int**)malloc(countProc * sizeof(int*));
        int redni_broj_ulaska_KO = 0;
        for(count = 0; count<countProc;count++){
                T_KO[count] = (int*)malloc(broj_ulazaka_KO * sizeof(int));
        }
        for(count = 1; count<argc;count++){
                if(count <= countProc){
                        T[count-1] = atoi(argv[count]);
                }
                else{
                        if(*argv[count]=='@'){
                                countAt++;
                                redni_broj_ulaska_KO=0;
                        }
                        else{
                                        T_KO[countAt-1][redni_broj_ulaska_KO]=atoi(argv[count]);
                                        redni_broj_ulaska_KO++;
                        }
                }
        }

        for(count = 0; count<countProc; count++){
                printf("ID procesa: %d ", count);
                printf("početak: %d ", T[count]);
                printf("vrijeme1: %d ", T_KO[count][0]);
                printf("vrijeme2: %d ", T_KO[count][1]);
                printf("\n");
                fflush(stdout);
        }
        


        //KREIRANJE PROCESA
        int status=0;
        pid_t wpid, child_pid ;
        for(count = 0;count < countProc; count++)
	{
		if ((child_pid = fork()) == 0)
		{
                        id=count;
                        posao();
		}
	}
        wait(NULL);

        printf("\n");
        return 0;
}
