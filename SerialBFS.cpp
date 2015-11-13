// test program to check graph and time collected

#include <ctime>
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <stdio.h>
#include <time.h>

using namespace std;

class Graph{

public:

	int n_vertices; //no. of vertices
	vector<vector<int> > adjList;
	std::vector<int> d;
	Graph(int n): adjList(n+1){
		n_vertices = n;
	}

	void addEdge(int u, int v){
		adjList[u].push_back(v);
	}

	vector<int>& getadjList(int u){
		return adjList[u];
	}

	void bfs(int n);

};

void Graph::bfs(int n){
	std::vector<int> d(n_vertices);
    for(int i=0;i<n_vertices;i++)
        {
    	d[i]=-1;
        }

    std::queue<int> q;
    q.push(n);
    d[n]=0;
    while (!q.empty()) {
        int currentvertex = q.front();
        q.pop();
        vector<int>::iterator iter;
        for(iter = adjList[currentvertex].begin();iter!=adjList[currentvertex].end();++iter) {
       		int indx = *iter;
            	if(d[indx]==-1){
            		d[indx]=d[currentvertex]+1;
                	q.push(indx);
        	}
       }
    }

    for(int i=0;i<n_vertices;i++)
    {
    	cout<<"dval::"<<d[i]<<endl;
    }
}

Graph ReadfromFile(){

		ifstream in;
		in.open("/home/heller/Codes-PDP/edges5");
		int n,e,i,u,v,s;
		in>>n>>e;
		Graph grph(n);
		for(i =1;i<=e;i++){
			in>>u>>v;
			grph.addEdge(u,v);
			}
		in.close();
		return grph;
}

int main(int argc,char *argv[]){
	clock_t t1, t2;
	t1 = clock();
	Graph grph1 = ReadfromFile();
	
	// source vertex random
	int v = 2;
	grph1.bfs(v);
	
	t2 = clock();
	float diff = (((float)t2 - (float)t1) / 1000000.0F ) * 1000;
	std::cout<<diff<<endl;
	return 0;
}
