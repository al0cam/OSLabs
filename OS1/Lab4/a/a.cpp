#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <csignal>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

class Barijera
{
    private:
        int br_dretava;
        
    public:
        std::vector<std::thread> t;
        
        void postavi(int broj_dretava)
        {
            br_dretava=broj_dretava;
        }
        
        void pokretac();
        ~Barijera()
        {
            for(auto& i:t)
                i.join();
            std::cout<<"kill"<<std::endl;
            t.clear();
        }
};

std::mutex mtx;
std::condition_variable cond;
int brojac=0;

void dretva(int id,int br_dretava)
{
    mtx.lock();
    std::cout<<"dretva broj: "<<id<<std::endl;
    int broj; 
    std::cin>>broj;
    mtx.unlock();
    std::unique_lock<std::mutex> lck(mtx);
    if(++brojac<br_dretava) cond.wait(lck);
    else cond.notify_all();
    std::cout<<"dretva broj: "<<id<<" broj: "<<broj<<std::endl;
}

void Barijera::pokretac()
{

    for(auto i=0;i<br_dretava;i++)
        t.push_back(std::thread(dretva,i,br_dretava));
        
}
Barijera *b=new Barijera;

void brisi(int sig)
{
    delete b;
    exit(0);
}
int main(int argch,char **argv)
{
    sigset(SIGINT,brisi);
    int broj_dretava=atoi(argv[1]);
    std::cout<<"broj dretava: "<<broj_dretava<<std::endl;
    b->postavi(broj_dretava);
    b->pokretac();
    delete b;
}
