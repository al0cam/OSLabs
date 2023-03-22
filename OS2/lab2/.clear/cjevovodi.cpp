#include<iostream>
#include<signal.h>
#include<stdio.h>
#include<ctime>
#include<stdlib.h>
#include<cstring>
#include<sys/types.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<fstream>

#define MAX_READ 20

using namespace std;

int main(int argc, char *argv[]){
	
	int fd1[2];
	pipe(fd1);
	int fd2[2];
	pipe(fd2);
	
	if(fork() == 0){
		
		int err;
		close(fd1[1]);
		close(fd2[0]);
		
		close(0);
		dup(fd1[0]); //UMJESTO CITANJA SA STANDARDNOG ULAZA, NEKA DIJETE CITA SA CJEVOVODA FD1	
				
		close(1);
		dup(fd2[1]); //UMJESTO PISANJA NA STANDARDNI IZLAZ, NEKA DIJETE PISE U CJEVOVOD FD2
		
		err = execl("/usr/bin/bc", "bc --quiet", (char*) NULL);
		
		close(fd1[0]);
		close(fd2[1]);
		
		exit(err);	
			
	}
	else{
		if(argc>1){
			ofstream file ("zadaci.txt");
			for(int i=1;i<argc;i++){
				
				file<<argv[i];
				file<<"\n";
				
			}
			file.close();
		}
		ifstream dat ("zadaci.txt");
		if(dat.is_open()){
			string line;
			cout<<endl;
			close(fd1[0]);
			close(fd2[1]);
			while(getline(dat,line)){
				if(dat.eof()){
					break;
				}
				int n = line.length();
				char lineArray[n];
				strcpy(lineArray,line.c_str());
				int prethodnaOperacija;
				int x;
				bool neispravanIzraz = false;
				for(int j=0;j<n;j++){
					char number = lineArray[j];
					if((j==0)&&(int(number)-48)<0){
						neispravanIzraz = true;
						break;
					}
					if(((int)number-48) < 0){
						if((int(number)-48)!=-1&&(int(number)-48)!=-3&&(int(number)-48)!=-5&&(int(number)-48)!=-6){
							neispravanIzraz = true;
							break;
						}
						x = j;
						if(j>1){
							if((x-prethodnaOperacija)==1){
								neispravanIzraz = true;
								break;
							}
						}
					prethodnaOperacija = x;
					}
					if((j==n-1)&&(int(number)-48)<0){
						neispravanIzraz = true;
						break;
					}		
				}
				char buffer[MAX_READ] = "";
			
				cout<<line<<"=";
				int rjesenje = 0;
				cin>>rjesenje;
			
				if(neispravanIzraz == true){
					cout<<"NEISPRAVAN IZRAZ"<<endl;
					cout<<endl;
				}
				else{
					write(fd1[1], lineArray, strlen(lineArray));
					write(fd1[1], "\n", 1);	
					read(fd2[0],buffer,sizeof(buffer));
				
					if(rjesenje == atoi(buffer)){
						cout<<"ISPRAVNO"<<endl;
						cout<<endl;
					}
					else if (rjesenje != atoi(buffer)){
						cout<<"NEISPRAVNO"<<","<<"tocan odgovor je "<<atoi(buffer)<<endl;
						cout<<endl;
					}
				}
			}
			dat.close();
			
			close(fd1[1]);
			close(fd2[0]);
		
			wait(NULL);
			return 0;
		}
		else{
			cout<<"Ponovno pokrenite program s aritmetickim zadacima za unos u datoteku u parametrima istoga"<<endl;
			return 0;
		}
										   
	}
		
}
