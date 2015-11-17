#include <iostream>
#include <fstream>
#include <queue>
#include <time.h>
#include <cilk/cilk.h>
#include <deque>
#include <set>
#include <cilkview.h>
#include <cilk/cilk_api.h>
//#include <fake_mutex.h>
#include <thread>         // std::thread
#include <mutex>          // std::mutex

using namespace std;

const int num_processors = 16;
//cilkscreen::fake_mutex mutexes[num_processors];
std::mutex mtx[num_processors+1];

const int MAX_STEAL_ATTEMPTS = 30;
const int MIN_STEAL_SIZE = 20;

class CollectionQueues
{
public:
    deque<int> **q;
    bool bflag=true;
    CollectionQueues(){
        int p = num_processors;
        q = new deque<int>*[p];
        for (int i =0; i<p; i++)
        {
            q[i]=new deque<int>(0);
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
//    void print(){
//    for(int i=0;i<num_processors;i++){
//        if(q[i]->size()!=0){
//        for(deque<int>::iterator it=q[i]->begin();it!=q[i]->end();it++){
//                cout<<" "<<*it<<endl;
//                }
//            }
//        }
//    }

    std::deque<int>* get(int i){
        return q[i];
    }

    void insertinto(int loc, int val){
        if(bflag)
            q[loc]->push_front(val);
        else
            q[loc]->push_back(val);
    }

    void setqueue(int loc,std::deque<int> *newqu){
        delete q[loc];
        q[loc]= new std::deque<int>;
        for (int i =0; i<newqu->size(); i++)
        {
            int val = newqu->at(i);
            q[loc]->push_back(val);
        }
    }

    bool isallempty(){
        int p = num_processors;
        for (int i=0; i<p; i++){
            if (!(q[i]->empty())){
                return false;
            }
        }
        return true;
    }

    bool isempty(int i){
            if (!(q[i]->empty()))
                return false;
        return true;}

    int popfromque(int i){
        int lret=(int)-1;
        if (!q[i]->empty()){
            lret = q[i]->front();
            q[i]->pop_front();
        }
        return lret;}
};


class Graph
{

public:
    int n_vertices; //no. of vertices
    std::vector<std::vector<int> > adjList; //adjaceny list
    std::vector<int> d;
    Graph(int n):
        adjList(n + 1){
        n_vertices = n;
    }
    void addEdge(int u, int v){
        adjList[u].push_back(v);
    }
    std::vector<int> getadjList(int u){
        return adjList[u];
    }

    void parallelbfsthread(int i, CollectionQueues* Qout, std::vector<int> *d,CollectionQueues* Qin){
        while(1)
        {
            while(!Qin->isempty(i))
            {
                mtx[i].lock();
                int u = Qin->popfromque(i);
                mtx[i].unlock();
                if(u==(int)-1)
                    break;
                std::vector<int> neighbors = getadjList(u);
                int size = neighbors.size();
                if(size<=0)
                {continue;}

                std::vector<int>::iterator iter;
                for (iter = neighbors.begin(); iter != neighbors.end(); ++iter)
                {
                    int indx = *iter;
                    int comparev = -1;
                    if (d->at(indx) == comparev)
                    {
                        d->at(indx)= d->at(u)+(int)1;
                        std::cout<<indx<<std::endl;
                        Qout->insertinto(i,indx);
                    }
                }
            }
            int t = 0;
            mtx[i].lock();
            while(Qin->isempty(i) && t < MAX_STEAL_ATTEMPTS)
            {
                int p = num_processors;
                int r = rand() % p ;

                if (r != i && mtx[r].try_lock())
                {
//                    std::cout<<"inside mutex try lock"<<std::endl;
                    std::deque<int>* cur = Qin->get(r);
                    if(!cur->empty())
                    {
                        if (cur->size()> MIN_STEAL_SIZE)
                        {
                            int temp =0;
                            int csize = cur->size();
                            std:: deque<int> newdeq (csize/2);
                            for (int jj = csize/2; jj<csize; jj++)
                            {
                                newdeq[temp]=cur->at(jj);
                                temp = temp+1;
                            }
                            Qin->setqueue(i,&newdeq);
                        }
                    }
                    mtx[r].unlock();
                }
                t = t+1;
            }
            mtx[i].unlock();

            if (Qin->get(i)->empty())
                break;
        }
    }

    void parallelbfs(int s)
    {
        std::vector<int> d (n_vertices+1);
        cilk_for(int n =0; n<n_vertices; n++){
            d[n]= -1;
        }
        int p =num_processors;
        CollectionQueues *Qin = new CollectionQueues();
        CollectionQueues *Qout = new CollectionQueues();
        Qin->insertinto(0,s);
        d[s]=0;
        std::cout<<s<<endl;
        while(!Qin->isallempty())
        {
            for (int i =0; i<p; i++){
                cilk_spawn parallelbfsthread(i,Qout,&d,Qin);
            }
            parallelbfsthread(p-1,Qout,&d,Qin);
            cilk_sync;
            delete Qin;
            Qin = Qout;
            Qout = new CollectionQueues();
        }
//        for(int i =0; i<d.size()-1; i++){
//            std::cout<<d[i]<<std::endl;
//        }
        delete Qout;
    }
};


Graph ReadfromFile()
{

    ifstream in;
    in.open("/home/heller/Codes-PDP/edges7");
    int n, e, i, u, v;
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
    __cilkrts_set_param("nworkers", "4");
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


