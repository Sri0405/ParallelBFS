#include <iostream>
#include <fstream>
#include <queue>
#include <time.h>
#include <cilk/cilk.h>
#include <deque>
#include <set>
#include <cilk/cilk_api.h>
//#include <fake_mutex.h>
#include <math.h>
#include <thread>         // std::thread
#include <mutex>          // std::mutex


std::mutex mtx;

using namespace std;

const int num_processors = 1;
//cilkscreen::fake_mutex mutex;

class CollectionQueues
{
public:
    deque<int> **q;
    bool bflag=true;
    int Sseg = 0;
    CollectionQueues()
    {
        int p = num_processors;
        q = new deque<int>*[p];
        for (int i =0; i<p; i++)
        {
            q[i]=new deque<int>(0);
        }
    }

    int size()
    {
        int s = 0;
        int p= num_processors;
        for(int i=0; i<p; i++)
        {
            s = s + q[i]->size();
        }
        return s;
    }

    ~CollectionQueues()
    {
        int p = num_processors;
        for (int i =0; i<p; i++)
        {
            delete q[i];
        }
        delete [] q;
    }

    std::deque<int>* get(int i)
    {
        return q[i];
    }

    void insertinto(int loc, int val)
    {
        if(bflag)
            q[loc]->push_front(val);
        else
            q[loc]->push_back(val);
    }

    void setqueue(int loc,std::deque<int> *newqu)
    {
        delete q[loc];
        q[loc]= new std::deque<int>;
        for (int i =0; i<newqu->size(); i++)
        {
            int val = newqu->at(i);
            q[loc]->push_back(val);
        }
    }

    bool isallempty()
    {
        int p = num_processors;
        for (int i=0; i<p; i++)
        {
            if (!(q[i]->empty()))
            {
                return false;
            }
        }
        return true;
    }

    bool isempty(int i)
    {
        if (!(q[i]->empty()))
            return false;
        return true;
    }

    int popfromque(int i)
    {
        int lret=(int)-1;
        if (!q[i]->empty())
        {
            lret = q[i]->front();
            q[i]->pop_front();
        }
        return lret;
    }

    int getsmallestnonemptyqueue()
    {
        int i;
        int p = num_processors;
        for (i=0; i< p; i++)
        {
            if (!(q[i]->empty()))
                break;
        }
        return i;
    }

    deque<int> nextSegment()
    {
        std::deque<int> S;
        mtx.lock();
        {

            if (!isallempty())
            {
                int i = getsmallestnonemptyqueue();
                int k = min(Sseg,(int)q[i]->size());

                for (int l=0; l<=k ; l++)
                {
                    S.push_back(popfromque(i));
                }
            }
        }
        mtx.unlock();
        return S;
    }

};


class Graph
{
public:
    int n_vertices; //no. of vertices
    std::vector<std::vector<int> > adjList; //adjaceny list
    std::vector<int> d;
    Graph(int n):
        adjList(n + 1)
    {
        n_vertices = n;
    }
    void addEdge(int u, int v)
    {
        adjList[u].push_back(v);
    }
    std::vector<int> getadjList(int u)
    {
        return adjList[u];
    }

    void parallelbfsthread(int i, CollectionQueues* Qout, std::vector<int> *d,CollectionQueues* Qin)
    {
        std::deque<int> S = Qin->nextSegment();
        while(!(S.empty()))
        {
            while(!(S.empty()))
            {
                int u = S.front();
                S.pop_front();
                if(u==-1)
                    break;
                std::vector<int> neighbors = getadjList(u);
                int size = neighbors.size();
                if(size<=0)
                {
                    continue;
                }

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
        }
    }

    void parallelbfs(int s)
    {
        std::vector<int> d (n_vertices+1);
        cilk_for(int n =0; n<n_vertices; n++)
        {
            d[n]= -1;
        }
        d[s]=0;

        std::cout<<s<<endl;

        int p =num_processors;

        CollectionQueues *Qin = new CollectionQueues();
        CollectionQueues *Qout = new CollectionQueues();

        Qin->insertinto(0,s);

        while(!Qin->isallempty())
        {
            int n_seg = 5;
            int ss= Qin->size();
            int vv= (ceil)(ss/n_seg);
            Qin->Sseg = vv;

            for (int i =0; i<p-1; i++)
            {
              cilk_spawn  parallelbfsthread(i,Qout,&d,Qin);
            }
            parallelbfsthread(p-1,Qout,&d,Qin);
            cilk_sync;
            delete Qin;
            Qin = Qout;
            Qout = new CollectionQueues();
        }
//        for(int i =0; i<d.size()-1; i++)
//        {
//            std::cout<<d[i]<<std::endl;
//        }
        delete Qout;
    }
};


Graph ReadfromFile()
{

    ifstream in;
    in.open("/home/heller/Codes-PDP/edges5");
    int n, e, i, u, v;
    in >> n >> e;
    Graph grph(n);
    for (i = 0; i < e; i++)
    {
        in >> u >> v;
        grph.addEdge(u, v);
    }
    in.close();
    return grph;
}

int main()
{
//    __cilkrts_set_param("nworkers", "1");
    clock_t t1, t2;
    Graph g = ReadfromFile();
    int v= 2;
    t1 = clock();
    g.parallelbfs(v);
    t2 = clock();
    float diff = (((float)t2 - (float)t1));
    float seconds = diff / CLOCKS_PER_SEC;
    std::cout<<"time:::"<<seconds<<endl;
    return 0;
}


