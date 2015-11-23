#include <iostream>
#include <fstream>
#include <queue>
#include <time.h>
#include <cilk/cilk.h>
#include <deque>
#include <set>
#include <cilk/cilk_api.h>
#include <thread>        
#include <mutex>         

using namespace std;

const long long int num_processors = 1;
std::mutex mtx[num_processors+1];

const long long int MAX_STEAL_ATTEMPTS = 50;
const long long int MIN_STEAL_SIZE = 10;

float d1 =0;

class CollectionQueues
{
public:
    deque<long long int> **q;
    bool bflag=true;
    CollectionQueues(){
        long long int p = num_processors;
        q = new deque<long long int>*[p];
        for (long long int i =1; i<=p; i++)
        {
            q[i]=new deque<long long int>(0);
        }
    }
    ~CollectionQueues(){
    long long int p = num_processors;
        for (long long int i =1; i<=p; i++)
        {
            delete q[i];
        }
        delete [] q;
    }

    std::deque<long long int>* get(long long int i){
        return q[i];
    }

    void insertinto(long long int loc, long long int val){
       clock_t t1,t2;
       t1 = clock();

        if(bflag)
            q[loc]->push_front(val);
        else
            q[loc]->push_back(val);
       t2=clock();
       float diff = (((float)t2 - (float)t1));
       d1 = d1+ (diff / CLOCKS_PER_SEC);

    }

    void setqueue(long long int loc,std::deque<long long int> *newqu){
        clock_t t1,t2;
        t1 = clock();
        delete q[loc];
        q[loc]= new std::deque<long long int>;
        for (long long int i =1; i<=newqu->size(); i++)
        {
            long long int val = newqu->at(i);
            q[loc]->push_back(val);
        }
       t2=clock();
       float diff = (((float)t2 - (float)t1));
       float xx= (diff / CLOCKS_PER_SEC);
       d1 = d1 +xx;
    }

    bool isallempty(){
        long long int p = num_processors;
        for (long long int i=1; i<=p; i++){
            if (!(q[i]->empty())){
                return false;
            }
        }
        return true;
    }

    bool isempty(long long int i){
            if (!(q[i]->empty()))
                return false;
        return true;}

    long long int popfromque(long long int i){
        long long int lret=(long long int)-1;
        if (!q[i]->empty()){
            lret = q[i]->front();
            q[i]->pop_front();
        }
        return lret;}
};


class Graph
{

public:
    long long int n_vertices; 
    std::vector<std::vector<long long int> > adjList; 
    std::vector<long long int> d;
    Graph(long long int n):
        adjList(n + 1){
        n_vertices = n;
    }
    void addEdge(long long int u, long long int v){
        adjList[u].push_back(v);
    }
    std::vector<long long int> getadjList(long long int u){
        return adjList[u];
    }

    void parallelbfsthread(long long int i, CollectionQueues* Qout, std::vector<long long int> *d,CollectionQueues* Qin){
        while(1)
        {
            while(!Qin->isempty(i))
            {
                mtx[i].lock();
                long long int u = Qin->popfromque(i);
                mtx[i].unlock();
                if(u==(long long int)-1)
                    break;
                std::vector<long long int> neighbors = getadjList(u);
                long long int size = neighbors.size();
                if(size<=0)
                {continue;}

                std::vector<long long int>::iterator iter;
               fstream file;
               file.open("/home/heller/testing/out2.txt", std::ios_base::app);

                for (iter = neighbors.begin(); iter != neighbors.end(); ++iter)
                {
                    long long int indx = *iter;
                    long long int comparev = -1;
                    if (d->at(indx) == comparev)
                    {
                        d->at(indx)= d->at(u)+(long long int)1;
                       file<<indx;
                       file<<endl;
                        Qout->insertinto(i,indx);
                    }
                }
               file.close();
            }
            long long int t = 0;
            mtx[i].lock();
            while(Qin->isempty(i) && t < MAX_STEAL_ATTEMPTS)
            {
                long long int p = num_processors;
                long long int r = rand() % p+1 ;

                if (r != i && mtx[r].try_lock())
                {
                    std::deque<long long int>* cur = Qin->get(r);
                    if(!cur->empty())
                    {
                        if (cur->size()> MIN_STEAL_SIZE)
                        {
                            long long int temp =0;
                            long long int csize = cur->size();
                            std:: deque<long long int> newdeq (csize/2);
                            for (long long int jj = csize/2; jj<csize; jj++)
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

    void parallelbfs(long long int s)
    {
        std::vector<long long int> d (n_vertices+1);
        long long int nn;
        for(nn =1; nn<=n_vertices; nn++){
            d[nn]= -1;
        }
        long long int p =num_processors;
        CollectionQueues *Qin = new CollectionQueues();
        CollectionQueues *Qout = new CollectionQueues();
        Qin->insertinto(1,s);
        clock_t t1, t2;
        t1 = clock();
        d[s]=0;
       ofstream outputfile;
       outputfile.open("/home/heller/testing/out2.txt");
       outputfile<<s;
       outputfile<<endl;
       outputfile.close();


        while(!Qin->isallempty())
        {
            for (long long int i =1; i<p; i++){
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
        float nsec = seconds;
        nsec = nsec -d1;
        fstream ffile;
        ffile.open("/home/heller/testing/timesd",std::ios_base::app);
        ffile<<"cores-time:::"<<seconds<<"---"<<d1<<"--"<<nsec<<endl;
        ffile.close();

        delete Qout;
    }
};

int main(int argc, char *argv[])
{

   ifstream in(argv[1]);
    long long int n, dm, a, b;
    in >> n >> dm;
    Graph g(n);
    while (in >> a >> b)
    {
        g.addEdge(a,b);
        g.addEdge(b,a);
    }
    in.close();

    long long int v= 1;
    g.parallelbfs(v);
 
    return 0;
}

