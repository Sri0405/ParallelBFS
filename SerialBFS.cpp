#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <stdio.h>
#include <time.h>
#include <chrono>

using namespace std;

long long int counts=0;

class Graph
{

public:

    long long int n_vertices; 
    vector<vector<long long int> > adjList;
    std::vector<long long int> sources;
    bool *check;

    Graph(long long int n): adjList(n+1)
    {
        n_vertices = n;
        check = new bool[n];
    }

    void addEdge(long long int u,long long int v)
    {
        adjList[u].push_back(v);
    }

    vector<long long int>& getadjList(long long int u)
    {
        return adjList[u];
    }

    void bfs(long long int n);

};

void Graph::bfs(long long int n)
{
    
    long long int i;
    for(i=0; i<n_vertices; i++)
    {
        check[i]=false;
    }

    check[n]= true;
  
    std::queue<long long int> q;
    q.push(n);
    while (!q.empty())
    {
        long long int currentvertex = q.front();
        counts = counts+1;
        q.pop();
        vector<long long int>::iterator iter;
        for(iter = adjList[currentvertex].begin(); iter!=adjList[currentvertex].end(); ++iter)
        {
            long long int indx = *iter;
            if(!check[indx])
            {
                cout<<" node is ::"<<indx<<endl;
                check[indx]=true;
                q.push(indx);
            }
        }
    }
}

int main(int argc,char* argv[])
{
    
    ifstream in(argv[1]);
    long long int n, dm, a, b;
    in >> n >> dm;
    Graph g(n);
    while (in >> a >> b)
    {
        g.addEdge(a,b);
    }
    in.close();
    long long int v= 1;
    auto start = chrono::steady_clock::now();

    g.bfs(v);
    
    auto end = chrono::steady_clock::now();

    auto diff = end - start;

    cout<<"counts is "<<counts<<endl;
    cout << chrono::duration <double, milli> (diff).count() << " ms" << endl;
    
    return 0;
}

