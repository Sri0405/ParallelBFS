
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

const int num_processors = 20;
cilkscreen::fake_mutex mutexes[num_processors+1];
const int MAX_STEAL_ATTEMPTS = 50;
int MIN_STEAL_SIZE = 20;


class CollectionQueues{

public:
    std::deque<long> **q;

    CollectionQueues()
    {
        int p = num_processors;
        for (int i =0; i<p;i++)
        {
            q[i] = new std::deque <long>;
        }
    }

    std::deque<long>* get(long i)
    {
     return q[i];
    }

    void insertinto(int loc, long val)
    {
        if (q[loc]== NULL)
        {
            q[loc] = new std::deque<long>;
        }
        q[loc]->push_back(val);
    }

    void setqueue(int loc,std::deque<long> *newqu)
    {
        q[loc]= new std::deque<long>;
        for (int i =0;i<newqu->size();i++)
        {
         long val = newqu->at(i);
         q[loc]->push_back(val);
        }
    }

    bool isempty()
    {
        int flag = 0;
        int p = num_processors;
        for (int i=0; i<p;i++)
        {
            if (!(q[i]->empty())){
                flag = 1;
                break;
            }
        }
        if( flag ==1)
        {
            return false;
        }
        else{
            return true;
        }
    }

    long popfromque(long i)
    {
        if (q[i]->empty())
        {
            return -1;
        }
        else{
        // void value not ignored as it ought to be
//            return *q[i]->pop_front();


    /// rectify this error
        }
    }
};



class Graph {

public:
	int n_vertices; //no. of vertices
	std::vector<std::vector<long> > adjList; //adjaceny list
	std::vector<long> sources; // list of sources
	std::vector<long> d;

	Graph(int n):
		adjList(n + 1) {
		n_vertices = n;
	}
	void addEdge(long u, long v) {
		adjList[u].push_back(v);
	}
	std::vector<long>& getadjList(long u) {
		return adjList[u];
	}

	void parallelbfsthread(int i, CollectionQueues* Qout, std::vector<long> *d,CollectionQueues* Qin)
	{

	while(1)
	{
        while(Qin->get(i)!= NULL)
        {
            long u = Qin->popfromque(i);
            std::vector<long>& neighbors = getadjList(u);
            long size = neighbors.size();
            if(size<=0)
            {
                continue;
            }

            std::vector<long>::iterator iter;
            for (iter = neighbors.begin(); iter != neighbors.end(); ++iter) {
                long indx = *iter;
                long comparev = -1;
                if (d->at(indx) == comparev) {
                    d->at(indx)= d->at(u)+(long)1;
                    Qout->insertinto(i,indx);
                }
            }

        }

        int t = 0;

        mutexes[i].lock();
        while(Qin->get(i) == NULL && t < MAX_STEAL_ATTEMPTS)
        {
         int var = num_processors;
         int r = rand() % var+ 1;
         if (r != i && mutexes[r].try_lock())
         {
            if (Qin->get(r)->size()>MIN_STEAL_SIZE)
            {
                std::deque<long>* cur = Qin->get(r);

                int temp =0;
                int csize = cur->size();
                std:: deque<long> newdeq (csize/2);

                for (int i = csize/2; i<csize;i++)
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
        if (Qin->get(i)==NULL)
        {
        break;
        }
	}
    }



	void parallelbfs(long s)
	{
        std::vector<long> d (n_vertices+1);

        cilk_for(int n =1; n<=n_vertices;n++)
         {   d[n]= -1;}


        int p =num_processors;

        CollectionQueues *Qin , *Qout;
        Qin = new CollectionQueues();
        Qout = new CollectionQueues();

        Qin->insertinto(1,s);

        while(!Qin->isempty())
        {
            for (int i =1;i<p;i++)
            {
                cilk_spawn parallelbfsthread(i,Qout,&d,Qin);
            }
            parallelbfsthread(p,Qout,&d,Qin);
            cilk_sync;

            Qin = Qout;
            Qout = new CollectionQueues();
        }

        for(int i =0;i<d.size();i++)
        {
            std::cout<<d[i]<<std::endl;
        }

	}



};


Graph ReadfromFile() {

	ifstream in;
	in.open("/home/heller/Codes-PDP/edges2");
	int n, e, s, i, u, v;
	in >> n >> e >>s;
	Graph grph(n);
	for (i = 1; i <= e; i++) {
		in >> u >> v;
		grph.addEdge(u, v);
	}
	int val;
	for (i =1; i<=s;i++)
	{
		in>>val;
		grph.sources.push_back(val);
	}
	in.close();

	return grph;

}

int cilk_main(){

Graph g = ReadfromFile();

for (int i=0; i<g.sources.size();i++)
{
    g.parallelbfs(g.sources[i]);
}

}


