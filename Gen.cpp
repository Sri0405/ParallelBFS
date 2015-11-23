#include <iostream>
#include <random>
#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>

using namespace std;

int main(int argc, char* argv[]) {
    typedef boost::adjacency_list<> Graph;
    typedef boost::erdos_renyi_iterator<mt19937, Graph> erg;

    if (argc < 3) return -1;

    long int n = atoi(argv[1]);
    double eps = atof(argv[2]);

    random_device rd;
    int seed = -1;

    long int s = 0 ;
    if (argc > 3) seed = atoi(argv[3]);
    if (argc == 5) s = atol(argv[4]);

    if (seed == -1) seed = rd();
    double p = (eps / n);

    mt19937 rng(seed);
    Graph G(erg(rng, n, p), erg(), n);

    boost::graph_traits<Graph>::edge_iterator e, end;
    tie(e, end) = boost::edges(G);
    
    ofstream outputfile;
    outputfile.open("/home/heller/testing/edges");
    
    outputfile<<n<<" "<<n<<"\n";    
    for (; e != end; ++e) {
        outputfile << (s + boost::source(*e, G)) << " "
                  << (s + boost::target(*e, G)) << "\n";
    }

    outputfile.close();
    return 0;
}
