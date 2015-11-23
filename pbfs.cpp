#include <iostream>
#include <fstream>
#include <queue>
#include <time.h>
#include <cilk/cilk.h>
#include <deque>
#include <set>
#include <cilk/cilk_api.h>
#include <math.h>
#include <thread>
#include <mutex>


std::mutex mtx;

float counts =0;

using namespace std;

const long long int num_processors = 1;

float d1=0;

class CollectionQueues
{
public:
    deque<long long int> **q;
    bool bflag=true;
    long long int Sseg = 0;
    CollectionQueues()
    {
        long long int p = num_processors;
        q = new deque<long long int>*[p];
        for (long long int i =1; i<=p; i++)
        {
            q[i]=new deque<long long int>(0);
        }
    }

    long long int size()
    {
        long long int s = 0;
        long long int p= num_processors;
        for(long long int i=1; i<=p; i++)
        {
            s = s + q[i]->size();
        }
        return s;
    }

    ~CollectionQueues()
    {
        long long int p = num_processors;
        for (long long int i =1; i<=p; i++)
        {
            delete q[i];
        }
        delete [] q;
    }

    std::deque<long long int>* get(long long int i)
    {
        return q[i];
    }

    void insertinto(long long int loc, long long int val)
    {
        if(bflag)
            q[loc]->push_front(val);
        else
            q[loc]->push_back(val);
    }

    bool isallempty()
    {
        long long int p = num_processors;
        for (long long int i=1; i<=p; i++)
        {
            if (!(q[i]->empty()))
            {
                return false;
            }
        }
        return true;
    }

    bool isempty(long long int i)
    {
        if (!(q[i]->empty()))
            return false;
        return true;
    }

    long long int popfromque(long long int i)
    {
        long long int lret=(long long int)-1;
        if (!q[i]->empty())
        {
            lret = q[i]->front();
            q[i]->pop_front();
        }
        return lret;
    }

    long long int getsmallestnonemptyqueue()
    {
        long long int i;
        long long int p = num_processors;
        for (i=1; i<=p; i++)
        {
            if (!(q[i]->empty()))
                break;
        }
        return i;
    }

    deque<long long int> nextSegment()
    {
    	mtx.lock();
        clock_t tt1,tt2;
    	tt1 =clock();
        std::deque<long long int> S;
        {
            if (!isallempty())
            {
                long long int i = getsmallestnonemptyqueue();
                long long int k = min(Sseg,(long long int)q[i]->size());
                for (long long int l=1; l<=k ; l++)
                {
                    if (l ==1){ S.push_front(popfromque(i));
                }
                else{
                     S.push_back(popfromque(i));
                   }
                }
            }
        }
	    tt2 = clock();
        float d=(float)tt2-(float)tt1;
        d1 = d1+d;
        mtx.unlock();
        return S;
    }

};


class Graph
{
public:
    long long int n_vertices;
    std::vector<std::vector<long long int> > adjList;
    std::vector<long long int> d;
    Graph(long long int n):adjList(n + 1)
    {
        n_vertices = n;
    }
    void addEdge(long long int u, long long int v)
    {
        adjList[u].push_back(v);
    }
    std::vector<long long int> getadjList(long long int u)
    {
        return adjList[u];
    }

    void parallelbfsthread(long long int i, CollectionQueues* Qout, std::vector<long long int> *d,CollectionQueues* Qin)
    {
        std::deque<long long int> S = Qin->nextSegment();
        while(!(S.empty()))
        {
            while(!(S.empty()))
            {
                long long int u = S.front();
                S.pop_front();
                if(u==-1)
                    break;
                std::vector<long long int> neighbors = getadjList(u);
                long long int size = neighbors.size();
                if(size<=0)
                {
                    continue;
                }
               fstream file;
               file.open("/home/heller/testing/out1.txt", std::ios_base::app);

                std::vector<long long int>::iterator iter;
                for (iter = neighbors.begin(); iter != neighbors.end(); ++iter)
                {
                    long long int indx = *iter;
                    long long int comparev = -1;

                    if (d->at(indx) == comparev)
                    {
                        d->at(indx)= d->at(u)+(long long int)1;
                        // cout<<indx<<endl;
                       file<<indx;
                       file<<endl;
                        counts = counts +1;
                        Qout->insertinto(i,indx);
                    }
                }
               file.close();
            }
        }
    }

    void parallelbfs(long long int s)
    {
        std::vector<long long int> d (n_vertices+1);
        long long int nn;
	    for(nn =1; nn<=n_vertices; nn++)
            {
                d[nn]= -1;
            }

        d[s]=0;
        counts = counts +1;
        // cout<<s<<endl;
	    clock_t t1,t2;
       ofstream outputfile;
       outputfile.open("/home/heller/testing/out1.txt");
       outputfile<<s;
       outputfile<<endl;
       outputfile.close();
        long long int p =num_processors;

        CollectionQueues *Qin = new CollectionQueues();
        CollectionQueues *Qout = new CollectionQueues();

        Qin->insertinto(1,s);

	    t1= clock();
        while(!Qin->isallempty())
        {
            long long int n_seg = 1;
            long long int ss= Qin->size();
            float df = ss/n_seg;
            df = df + 0.5;
            long long int vv= (long long int)round(df);
            Qin->Sseg = vv;

            for (long long int i =1; i<p; i++)
            {
                cilk_spawn parallelbfsthread(i,Qout,&d,Qin);
            }
            parallelbfsthread(p,Qout,&d,Qin);
            cilk_sync;
            delete Qin;
            Qin = Qout;
            Qout = new CollectionQueues();
        }
    	t2 = clock();
    	float diff = (((float)t2 - (float)t1));
       	float seconds = diff / CLOCKS_PER_SEC;
    	d1 = d1 / CLOCKS_PER_SEC;
    	float new_sc = seconds - d1;
      	fstream ffile;
       ffile.open("/home/heller/testing/timesf",std::ios_base::app);
       ffile<<"cores-time:::"<<seconds<<"---"<<d1<<"---"<<new_sc<<endl;
       ffile.close();
        delete Qout;
    }
};


int main(int argc,char* argv[])
{
    ifstream in("/home/heller/testing/edges");
    long long int n, dm, a, b;
    in >> n >> dm;
    Graph g(n);
    while (in >> a >> b)
    {
        g.addEdge(a,b);
    }
    in.close();

    long long int v= 1;
    g.parallelbfs(v);
    cout<< "Count is "<< counts<<endl;
    return 0;
}

