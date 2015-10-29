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
	bool *check;
	
	Graph(int n): adjList(n+1){
		n_vertices = n;
		check = new bool[n];
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

    for(int i=0;i<n_vertices;i++)
        check[i]=false;
    std::queue<int> q;
    q.push(n);
    while (!q.empty()) {
        int currentvertex = q.front();
        check[currentvertex] = true;
        std::cout<<currentvertex<<" ";
        q.pop();
       vector<int>::iterator iter;
       for(iter = adjList[currentvertex].begin();iter!=adjList[currentvertex].end();++iter) {
            	if(!check[*iter]){
                q.push(*iter);
        	}  
       }
    }
}

Graph ReadfromFile(){
		
		ifstream in;
		in.open("/home/heller/Codes-PDP/edges1");	
		int n,e,i,u,v;
		in>>n>>e;
		Graph grph(n);
		std::cout<<n<<"---"<<e<<std::endl;
		for(i =1;i<=e;i++){
			in>>u>>v;
			grph.addEdge(u,v);
			std::cout<<u<<"-"<<v<<std::endl;
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

int main(int argc,char *argv[]){
	// print_time();
	clock_t t1, t2;
	t1 = clock();
	Graph grph1 = ReadfromFile();
	grph1.bfs(2);
	t2 = clock();
	float diff = (((float)t2 - (float)t1) / 1000000.0F ) * 1000;
	std::cout<<diff<<endl;
	// print_time();
	return 0;
}
