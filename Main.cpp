#include <iostream>
#include <fstream>
#include <queue>
#include <time.h>
#include <cilk/cilk.h>
#include <deque>
#include <set>
#include <cilkview.h>
#include <cilk/cilk_api.h>
#include <fake_mutex.h>

using namespace std;

const int num_processors = 2;
cilkscreen::fake_mutex mutexes[num_processors+1];
const int MAX_STEAL_ATTEMPTS = 50;
int MIN_STEAL_SIZE = 5;

class CollectionQueues
{
public:
    deque<long> **q;
    bool bflag=true;
    CollectionQueues(){
        int p = num_processors;
        q = new deque<long>*[p];
        for (int i =0; i<p; i++)
        {
            q[i]=new deque<long>(0);
        }
    }
    ~CollectionQueues(){
    int p = num_processors;
        for (int i =0; i<p; i++)
        {
            delete q[i];
        }
        delete [] q;

    }
    void print(){
    for(int i=0;i<num_processors;i++){
        if(q[i]->size()!=0){
        for(deque<long>::iterator it=q[i]->begin();it!=q[i]->end();it++){
                cout<<" "<<*it<<endl;
                }
            }
        }
    }

    std::deque<long>* get(long i){
        return q[i];
    }

    void insertinto(int loc, long val){
        if(bflag)
            q[loc]->push_front(val);
        else
            q[loc]->push_back(val);
    }

    void setqueue(int loc,std::deque<long> *newqu){
        delete q[loc];
        q[loc]= new std::deque<long>;
        for (int i =0; i<newqu->size(); i++)
        {
            long val = newqu->at(i);
            q[loc]->push_back(val);
        }
    }

    bool isempty(){
        int p = num_processors;
        for (int i=0; i<p; i++){
            if (!(q[i]->empty())){
                return false;
            }
        }
        return true;
    }

    long popfromque(long i){
        long lret=(long)-1;
        if (!q[i]->empty()){
            lret = q[i]->front();
            q[i]->pop_front();
        }
        return lret;
    }
};


class Graph
{

public:
    int n_vertices; //no. of vertices
    std::vector<std::vector<long> > adjList; //adjaceny list
    std::vector<long> d;
    Graph(int n):
        adjList(n + 1){
        n_vertices = n;
    }
    void addEdge(long u, long v){
        adjList[u].push_back(v);
    }
    std::vector<long> getadjList(long u){
        return adjList[u];
    }

    void parallelbfsthread(int i, CollectionQueues* Qout, std::vector<long> *d,CollectionQueues* Qin){
        while(1)
        {
            while(!Qin->isempty())
            {
                long u = Qin->popfromque(i);
                if(u==(long)-1)
                    break;
                std::vector<long> neighbors = getadjList(u);
                long size = neighbors.size();
                if(size<=0)
                {
                    continue;
                }

                std::vector<long>::iterator iter;
                for (iter = neighbors.begin(); iter != neighbors.end(); ++iter)
                {
                    long indx = *iter;
                    long comparev = -1;
                    if (d->at(indx) == comparev)
                    {
                        d->at(indx)= d->at(u)+(long)1;
                        Qout->insertinto(i,indx);
                    }
                }
            }
            int t = 0;
            mutexes[i].lock();
            while(Qin->isempty() && t < MAX_STEAL_ATTEMPTS)
            {
                int p = num_processors;
                int r = rand() % p;
                if (r != i && mutexes[r].try_lock())
                {
                    if (Qin->get(r)->size()>MIN_STEAL_SIZE)
                    {
                        std::deque<long>* cur = Qin->get(r);
                        int temp =0;
                        int csize = cur->size();
                        std:: deque<long> newdeq (csize/2);
                        for (int i = csize/2; i<csize; i++)
                        {
                            newdeq.at(temp)=cur->at(i);
                            temp = temp+1;
                        }
                        Qin->setqueue(i,&newdeq);
                    }
                    mutexes[r].unlock();
                }
                t = t+1;
            }
            mutexes[i].unlock();
            if (Qin->isempty())
                break;
        }
    }

    void parallelbfs(long s)
    {
        std::vector<long> d (n_vertices+1);
        cilk_for(int n =0; n<n_vertices; n++){
            d[n]= -1;
        }
        int p =num_processors;
        CollectionQueues *Qin = new CollectionQueues();
        CollectionQueues *Qout = new CollectionQueues();
        Qin->insertinto(0,s);
        d[s]=0;
        while(!Qin->isempty())
        {
            for (int i =0; i<p; i++)            {
                cilk_spawn parallelbfsthread(i,Qout,&d,Qin);
            }
            parallelbfsthread(p-1,Qout,&d,Qin);
            cilk_sync;
            delete Qin;
            Qin = Qout;
            Qout = new CollectionQueues();
        }
        for(int i =0; i<d.size()-1; i++){
            std::cout<<d[i]<<std::endl;
        }
        delete Qout;
    }
};


Graph ReadfromFile()
{

    ifstream in;
    in.open("/home/heller/Codes-PDP/edges5");
    int n, e, s, i, u, v;
    in >> n >> e;
    Graph grph(n);
    for (i = 0; i < e; i++){
        in >> u >> v;
        grph.addEdge(u, v);
    }
    in.close();
    return grph;
}

int main()
{
    clock_t t1, t2;
    t1 = clock();
    Graph g = ReadfromFile();
    int v=2;
    g.parallelbfs(v);
    t2 = clock();
    float diff = (((float)t2 - (float)t1) / 1000000.0F ) * 1000;
    std::cout<<diff<<endl;
    return 0;
}


