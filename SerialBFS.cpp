// Serial BFS

#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <stdio.h>
#include <time.h>

using namespace std;

class Graph
{

public:

    int n_vertices; //no. of vertices
    vector<vector<int> > adjList;
    std::vector<int> sources;
    bool *check;
    std::vector<int> d;
    Graph(int n): adjList(n+1)
    {
        n_vertices = n;
        check = new bool[n];
    }

    void addEdge(int u, int v)
    {
        adjList[u].push_back(v);
    }

    vector<int>& getadjList(int u)
    {
        return adjList[u];
    }

    void bfs(int n);

};

void Graph::bfs(int n)
{
//	cout<<"worked till here"<<endl;

    for(int i=0; i<n_vertices; i++)
    {

        check[i]=false;
    }

    check[n]= true;
    std::queue<int> q;
    q.push(n);

    while (!q.empty())
    {
        int currentvertex = q.front();
        std::cout<<currentvertex<<std::endl;
        q.pop();
        vector<int>::iterator iter;
        for(iter = adjList[currentvertex].begin(); iter!=adjList[currentvertex].end(); ++iter)
        {
            int indx = *iter;
            if(!check[indx])
            {
                check[indx]=true;
                q.push(indx);
            }
        }
    }
}

Graph ReadfromFile()
{

    ifstream in;
    in.open("/home/heller/Codes-PDP/edges7");
    int n,e,i,u,v,s;
    in>>n>>e>>s;
    Graph grph(n);
    // std::cout<<n<<"---"<<e<<std::endl;
    for(i =1; i<=e; i++)
    {
        in>>u>>v;
        grph.addEdge(u,v);
        // std::cout<<u<<"-"<<v<<std::endl;
    }
    int val;
    for (i =1; i<=s; i++)
    {
        in>>val;
        grph.sources.push_back(val);
    }
    in.close();

    return grph;

}

// void print_time(){
// 	// source : stack overflow
// 	// time_t t =time(0);
// 	// struct tm * now = localtime(&t);
// 	// std::cout<< (now->tm_hour<<":"<<now->tm_min<<":"<<now->tm_sec<<now->std::endl;
// }

int main(int argc,char *argv[])
{
    // print_time();
    clock_t t1, t2;
    Graph grph1 = ReadfromFile();
    t1 = clock();

    int v = 2;
    grph1.bfs(v);


    t2 = clock();
    float diff = (((float)t2 - (float)t1));
    float seconds = diff/CLOCKS_PER_SEC;
	std::cout<<seconds<<endl;
    // print_time();
    return 0;
}

