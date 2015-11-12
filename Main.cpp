#include <iostream>
#include<fstream>
#include <queue>
#include <time.h>
#include <cilk/cilk.h>
#include <deque>
#include <set>
#include <cilkview.h>
#include <cilk/cilk_api.h>
#include <fake_mutex.h>

using namespace std;


const int num_processors = 4;
cilkscreen::fake_mutex mutexes[num_processors+1];
const int MAX_STEAL_ATTEMPTS = 10;
int MIN_STEAL_SIZE = 20;

class CollectionQueues
{

public:
    deque<long> **q;
    bool bflag=true;

    CollectionQueues()
    {
//        std::cout<<"check creating queue"<<std::endl;

        int p = num_processors;
        q = new deque<long>*[p];
        for (int i =0; i<p; i++)
        {
            q[i]=new deque<long>(0);
        }

//        std::cout<<"finished initialzing"<<std::endl;
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

    void print()
    {
//    cout<<"print function"<<endl;

//    cout<<"size:"<<q[0]->size()<<endl;

    for(int i=0;i<num_processors;i++){
        if(q[i]->size()!=0){
        for(deque<long>::iterator it=q[i]->begin();it!=q[i]->end();it++){
            cout<<" "<<*it<<endl;
        }}
    }

    //cout<<*((*q[0]).begin())<<endl;
    //cout<<*((*q[0]).end())<<endl;
//                for(std::deque<long>::iterator it=(*q[0]).begin();it!=(*q[0]).end();it++){
//                cout<<' '<<*it;
//            }

//    cout<<"print completed"<<endl;
    }

    std::deque<long>* get(long i)
    {
        return q[i];
    }

    void insertinto(int loc, long val)
    {
//        std::cout<<"in insertion"<<std::endl;
        if(bflag)
            q[loc]->push_front(val);
        else
            q[loc]->push_back(val);
//        std::cout<<"completed insertion"<<std::endl;
    }

    void setqueue(int loc,std::deque<long> *newqu)
    {
        delete q[loc];
        q[loc]= new std::deque<long>;
        for (int i =0; i<newqu->size(); i++)
        {
            long val = newqu->at(i);
            q[loc]->push_back(val);
        }
    }

    bool isempty()
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

    long popfromque(long i)
    {
        long lret=(long)-1;
        if (!q[i]->empty())
        {
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
    std::vector<long> sources; // list of sources

    std::vector<long> d;
    Graph(int n):
        adjList(n + 1)
    {
        n_vertices = n;
    }
    void addEdge(long u, long v)
    {
        adjList[u].push_back(v);
    }
    std::vector<long> getadjList(long u)
    {
        return adjList[u];
    }

    void parallelbfsthread(int i, CollectionQueues* Qout, std::vector<long> *d,CollectionQueues* Qin)
    {
//        cout<<"parallel bfs thread"<<endl;
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
//                    cout<<"indexxxxxxxxxxxxxxxxxxxxxxx"<<indx<<endl;
                    long comparev = -1;
                    if (d->at(indx) == comparev)
                    {
                        d->at(indx)= d->at(u)+(long)1;
//                        cout<<"inserting into Qout"<<endl;
//                        cout<< "inserting"<<indx<<endl;
                        Qout->insertinto(i,indx);
                    }
                }
//                cout<<"afterrrrrrrrrrrrrrrrrrrrrrrrrrr"<<endl;

            }

//            cout<<"finished while"<<endl;

            int t = 0;

            mutexes[i].lock();
            while(Qin->isempty() && t < MAX_STEAL_ATTEMPTS)
            {
//                cout<<"inside next while"<<endl;
                int p = num_processors;
                int r = rand() % p;
//                cout<<"random number generator"<<endl;
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
//                        cout<<"ikade bokka"<<endl;
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

        cilk_for(int n =0; n<n_vertices; n++)
        {
            d[n]= -1;
        }

        int p =num_processors;
        CollectionQueues *Qin = new CollectionQueues();

//        std::cout<< " intialized one "<<std::endl;
        CollectionQueues *Qout = new CollectionQueues();
//        std::cout<<"check if initialized"<<std::endl;

        Qin->insertinto(0,s);
        d[s]=0;

//        cout<<"in print only";
//        Qin->print();

        while(!Qin->isempty())
        {
//            cout<<"inside loop"<<endl;
            for (int i =0; i<p; i++)
            {
                cilk_spawn parallelbfsthread(i,Qout,&d,Qin);
            }
            parallelbfsthread(p-1,Qout,&d,Qin);
            cilk_sync;

//            cout<<"deleting Qin and initiling Qout"<<endl;
            delete Qin;
            Qin = Qout;
            Qout = new CollectionQueues();
        }

        cout<<"printing d"<<endl;
        for(int i =0; i<d.size()-1; i++)
        { std::cout<<d[i]<<std::endl;
        }
        delete Qout;
    }

};


Graph ReadfromFile()
{

    ifstream in;
    in.open("/home/heller/Codes-PDP/edges2");
    int n, e, s, i, u, v;
    in >> n >> e >>s;
    Graph grph(n);
    for (i = 0; i < e; i++)
    {
        in >> u >> v;
        grph.addEdge(u, v);
    }
    int val;
    for (i =0; i<s; i++)
    {
        in>>val;
        grph.sources.push_back(val);
    }
    in.close();

    return grph;

}

int main()
{
//    std::cout<<"Starting"<<std::endl;
    Graph g = ReadfromFile();

//    std::cout<<"read from graph"<<std::endl;

//    for (int i=0; i<g.sources.size(); i++)
    {
//        std::cout<<"source:::"<<g.sources[i]<<std::endl;
        g.parallelbfs(g.sources[0]);
    }
    return 0;

}

