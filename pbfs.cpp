#include <iostream>
#include <fstream>
#include <queue>
#include <time.h>
#include <cilk/cilk.h>
#include <deque>
#include <stdlib.h>
#include <set>
#include <cilk/cilk_api.h>
#include <math.h>
#include <thread>
#include <mutex>

using namespace std;

std::mutex mtxs;

int num_processors = 32;

class CollectionQueues
{
public:
	deque<long long int> **q;
	bool bflag = true;
	long long int Sseg = 0;
	
	CollectionQueues()
	{
		long long int p = num_processors;
		q = new deque<long long int>*[p];
		for (long long int i = 0; i < p; i++)
		{
			q[i] = new deque<long long int>(0);
		}
	}

	long long int size()
	{
		long long int s = 0;
		long long int p = num_processors;
		for (long long int i = 0; i < p; i++)
		{
			s = s + q[i]->size();
		}
		return s;
	}

	std::deque<long long int>* get(long long int i)
	{
		return q[i];
	}

	void insertinto(long long int loc, long long int val)
	{
		if (mtxs.try_lock())
		{
			q[loc]->push_back(val);
			mtxs.unlock();
		}
	}

	bool isallempty()
	{
		long long int p = num_processors;
		
		for (long long int i = 0; i < p; i++)
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

		long long int lret = (long long int) - 1;
		int siez = q[i]->size();
		if (q[i] && !q[i]->empty())
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
		for (i = 0; i < p; i++)
		{
			if (!(q[i]->empty()))
				break;
		}
		return i;
	}

	deque<long long int> nextSegment()
	{
		mtxs.lock();
		std::deque<long long int> S;
		{
			if (!isallempty())
			{
				long long int i = getsmallestnonemptyqueue();
				long long int k = min(Sseg, (long long int)q[i]->size());
				
				for (long long int l = 0; l < k; l++)
				{
					{
						S.push_front(popfromque(i));
					}
				}
			}
		}
		mtxs.unlock();
		return S;
	}
};


class Graph
{
public:
	long long int n_vertices;
	std::vector<std::vector<long long int> > adjList;

	Graph(long long int n) :adjList(n + 1)
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

};

class PBS {

public:

	void parallelbfsthread(int i, Graph* g, CollectionQueues* Qout, std::vector<long long int> *d, CollectionQueues* Qin)
	{
		std::deque<long long int> S;
		{
			S = Qin->nextSegment();
		}

		while (!(S.empty()))
		{
			while (!(S.empty()))
			{
				long long int u = S.front();
				S.pop_front();
				std::vector<long long int> neighbors = g->getadjList(u);	
				long long int size = neighbors.size();
				std::vector<long long int>::iterator iter;
				for (iter = neighbors.begin(); iter != neighbors.end(); ++iter)
				{
					long long int indx = *iter;
					long long int comparev = -1;

					if (d->at(indx) == comparev)
					{
						d->at(indx) = d->at(u) + (long long int)1;
						Qout->insertinto(rand()% num_processors, indx);
					}
				}
			}
		}
	}

	void parallelbfs(Graph g, long long int s)
	{
		std::vector<long long int> d(g.n_vertices + 1);
		long long int nn;
		for (nn = 0; nn < g.n_vertices; nn++)
		{
			d[nn] = -1;
		}

		d[s] = 0;
		long long int p = num_processors;

		CollectionQueues *Qin = new CollectionQueues();
		CollectionQueues *Qout = new CollectionQueues();

		Qin->insertinto(0, s);

		auto start = chrono::steady_clock::now();
		while (!Qin->isallempty())
		{
			long long int n_seg = 1;
			long long int ss = Qin->size();
			float df = ss / n_seg;
			df = df + 0.5;
			long long int vv = (long long int)round(df);
			Qin->Sseg = vv;

			for (int i = 0; i<p; i++)
			{
				cilk_spawn parallelbfsthread(i, &g, Qout, &d, Qin);
			}
			parallelbfsthread(p-1, &g, Qout, &d, Qin);
			cilk_sync;
			delete Qin;
			Qin = Qout;
			Qout = new CollectionQueues();
		}
		auto end = chrono::steady_clock::now();
		auto diff = end - start;

		long counts =0;

		for (nn = 0; nn < g.n_vertices; nn++)
		{
			if(d[nn] != -1)
			{
				counts=counts+1;
			}
			
		}
		
		fstream ffile;
		ffile.open("/gpfs/courses/cse603/students/sridharb/timespbs32.txt", std::ios_base::app);
		ffile << "serial for counts is " << counts << endl;
		ffile<<"counts "<<counts<<endl;
		ffile<< chrono::duration <double, milli>(diff).count() << "ms" << endl;
		ffile.close();
		delete Qout;
	}
};

int main(int argc, char* argv[])
{
	ifstream in(argv[1]);
	__cilkrts_set_param("nworkers", "32");
	long long int n, dm, a, b;
	in >> n >> dm;
	Graph g(n);
	while (in >> a >> b)
	{
		g.addEdge(a, b);
		g.addEdge(b, a);
	}
	in.close();
	long long int v = 0;
	PBS pbs;
	pbs.parallelbfs(g, v);
	return 0;
}

