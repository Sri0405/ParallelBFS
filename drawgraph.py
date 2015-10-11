import networkx as nx
import matplotlib.pyplot as plt


class Object():
    def draw_graph(self, graph):

        # extract nodes from graph
        nodes = set([n1 for n1, n2 in graph] + [n2 for n1, n2 in graph])

        # create networkx graph
        G = nx.Graph()

        # add nodes
        for node in nodes:
            G.add_node(node)

        # add edges
        for edge in graph:
            G.add_edge(edge[0], edge[1])

        # draw graph
        pos = nx.shell_layout(G)
        nx.draw(G, pos)

        # show graph
        plt.show()


G = nx.fast_gnp_random_graph(7, 0.5, directed=True)
print nx.degree(G)
print G.in_degree()
print G.out_degree()

Object().draw_graph(G.edges())
