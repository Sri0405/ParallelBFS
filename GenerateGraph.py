

import networkx as nx
#number of vertices
n = 5000  
G = nx.fast_gnp_random_graph(n,0.25,directed=True)
lst = G.edges()
print n,lst.__len__()
for i in lst:
    print str(i[0])+" "+str(i[1])










        
   
