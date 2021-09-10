"""Suurballe's algorithm for vertex disjoint paths"""

import sys
from heapq import heappush, heappop
from typing import Dict, List, Set, Tuple
import networkx as nx
from tspwplib import (
    DisjointPaths,
    EdgeList,
    EdgeFunction,
    VertexList,
    Vertex,
    VertexLookup,
    VertexFunction,
)
from .constants import NULL_VERTEX


def find_parents_in_shortest_path_tree(
    shortest_path_from_source: Dict[Vertex, VertexList]
) -> VertexLookup:
    """For each vertex v, find the parent of v in the shortest path tree"""
    parents: VertexLookup = {}
    for vertex, path in shortest_path_from_source.items():
        num_vertices_in_path = len(path)
        if num_vertices_in_path <= 1:
            parents[vertex] = NULL_VERTEX
        else:
            parents[vertex] = path[num_vertices_in_path - 2]
    return parents


# pylint: disable=too-many-instance-attributes
class SuurballeTree(nx.DiGraph):
    """
    Store the tree and all other datastructures for Suurballe's algorithm
    for finding least-cost edge-disjoint paths
    from s to every vertex in the graph.
    """

    def __init__(
        self,
        source: Vertex,
        distance_from_source: VertexFunction,
        parent_in_shortest_path_tree: VertexLookup,
    ) -> None:
        """
        Args:
            source: The source vertex
            distance_from_source: The distance of the shortest path from source to a vertex v
            parent_in_shortest_path_tree: The parent of a vertex in the shortest path tree
        """

        super().__init__()
        self.source: Vertex = source  # source vertex
        self.tentative_predecessor: VertexLookup = {}  # denoted p in the paper
        self.tentative_distance: VertexFunction = {}  # denoted d
        self.transformed_cost: EdgeFunction = {}  # transformed cost
        self.children: Dict[Vertex, VertexList] = {}  # children of vertex
        self.parent: VertexLookup = {}  # parent of vertex
        self.edges_incident_to_vertex: Dict[Vertex, EdgeList] = {}  # denoted i
        self.labeled: Dict[Vertex, bool] = {}  # label of vertex (T/F)
        self.heap_queue: List[Tuple[int, Vertex]] = []  # heap queue
        self.distance_from_source: VertexFunction = (
            distance_from_source  # denoted w in the paper
        )
        self.process_cause: VertexLookup = {}  # q(v) causes {p(v), v} to be processed

        for vertex, parent_in_tree in parent_in_shortest_path_tree.items():
            if self.source != vertex:
                self.add_edge(parent_in_tree, vertex)

        for v in distance_from_source:
            self.add_node(v)
            if v == self.source:
                # special case for the start vertex
                self.tentative_distance[v] = 0
            else:
                self.tentative_distance[v] = sys.maxsize
            self.tentative_predecessor[v] = NULL_VERTEX
            self.process_cause[v] = NULL_VERTEX
            self.labeled[v] = False
            heappush(self.heap_queue, (self.tentative_distance[v], v))

    def print_node_data(self, u):
        """Print the data for a given vertex"""
        print()
        print("Node is", u)
        print("p[u] is", self.tentative_predecessor[u])
        print("d[u] is", self.tentative_distance[u])
        print("c[u] is", self.transformed_cost[u])
        print("children of u are", self.children[u])
        # print("parent of u is", self.parent[u])
        print("i[u] is", self.edges_incident_to_vertex[u])
        print("l[u] is", self.labeled[u])
        print("w of u is", self.distance_from_source[u])
        print("q of u is", self.process_cause[u])


def preorder(T: nx.DiGraph, source: Vertex) -> VertexList:
    """Computes a preorder traversal of the tree.

    Args:
        T: A directed graph that must be a tree
        source: Vertex to start traversal at

    Returns:
        List of nodes in preorder.

    Note:
        Assigns nodes with the attribute "pre".
    """
    nx.set_node_attributes(T, sys.maxsize, name="pre")
    order: VertexList = []
    preorder_recursive(T, source, order)
    return order


def preorder_recursive(T: nx.DiGraph, vertex: Vertex, order: VertexList) -> None:
    """Recursive call in preorder traversal"""
    T.nodes[vertex]["pre"] = len(order)
    order.append(vertex)
    for successor in T.successors(vertex):
        preorder_recursive(T, successor, order)


def postorder(T: nx.DiGraph, source: Vertex) -> VertexList:
    """Postorder traversal of the tree.

    Args:
        T: A directed graph that must be a tree
        source: Vertex to start traversal at

    Returns:
        Returns a list of nodes in postorder.

    Note:
        Assigns nodes with the attribute "post".
    """
    nx.set_node_attributes(T, sys.maxsize, name="post")
    order: VertexList = []
    postorder_recursive(T, source, order)
    return order


def postorder_recursive(T: nx.DiGraph, vertex: Vertex, order: VertexList) -> None:
    """Recursive call for postorder traversal"""
    for successor in T.successors(vertex):
        postorder_recursive(T, successor, order)
    T.nodes[vertex]["post"] = len(order)
    order.append(vertex)


def adjust_edge_cost(
    original_cost_of_edge: int, distance_from_s_to_u: int, distance_from_s_to_v: int
) -> int:
    """Adjust the cost of an edge from u to v in the DiGraph

    Args:
        original_cost_of_edge: The cost of the edge in the original graph
        distance_from_s_to_u: The cost of the shortest path from s to u
        distance_from_s_to_v: The cost of the shortest path from s to v

    Returns:
        Adjusted cost
    """
    return distance_from_s_to_u - distance_from_s_to_v + original_cost_of_edge


def adjust_edge_cost_for_graph(
    G: nx.Graph, distance_from_source: VertexFunction, weight: str = "weight"
) -> EdgeFunction:
    """For every edge in the graph, adjust the cost function

    Args:
        G: Directed input graph
        distance_from_source: The shortest distance in G from source to a vertex v

    Returns:
        Transformed cost for every edge in the graph. Adjacency list format.
    """
    # transform the cost of edges from u to v
    adjusted_cost: EdgeFunction = {}
    for u in G:
        adjusted_cost[u] = {}
        for v in G[u]:
            adjusted_cost[u][v] = adjust_edge_cost(
                G[u][v][weight], distance_from_source[u], distance_from_source[v]
            )
    return adjusted_cost


def add_edges_incident_to_endpoints(
    tree: SuurballeTree, tail_vertex: Vertex, head_vertex: Vertex
) -> None:
    """Give the head and tail of an edge, add any edges that are incident to the head and tail
    to their respective edge incident lists

    Args:
        tree: Initialised Suurballe tree
        tail_vertex: Tail vertex of the edge
        head_vertex: Head vertex of the edge

    Note:
        An edge goes from the tail to the head
    """
    # the head and tail take it in turns being in the spotlight and shadow
    for spotlight_vertex, shadow_vertex in zip(
        [tail_vertex, head_vertex], [head_vertex, tail_vertex]
    ):
        if len(tree.edges_incident_to_vertex[spotlight_vertex]) == 0:
            tree.edges_incident_to_vertex[spotlight_vertex].append(
                (tail_vertex, head_vertex)
            )
        else:
            inserted = False
            j = 0
            while (
                j < len(tree.edges_incident_to_vertex[spotlight_vertex])
                and not inserted
            ):
                # get an edge incident to the spotlight vertex
                incident_edge = tree.edges_incident_to_vertex[spotlight_vertex][j]

                # get the other distinct endpoint of the edge - denoted x in the paper
                other_endpoint = incident_edge[0]
                if other_endpoint == spotlight_vertex:
                    other_endpoint = incident_edge[1]

                if tree.nodes[other_endpoint]["pre"] > tree.nodes[shadow_vertex]["pre"]:
                    tree.edges_incident_to_vertex[spotlight_vertex].insert(
                        j, (tail_vertex, head_vertex)
                    )
                    inserted = True
                j += 1
            if not inserted:
                tree.edges_incident_to_vertex[spotlight_vertex].append(
                    (tail_vertex, head_vertex)
                )


def suurballe_shortest_vertex_disjoint_paths(
    G: nx.DiGraph, source: Vertex, weight: str = "weight"
) -> SuurballeTree:
    """
    Suurballe's algorithm for multiple targets.
    For every vertex in G, finds the least-cost edge-disjoint paths from s to v.

    Args:
        G : DiGraph
            Directed input graph.
            Must be asymmetric, i.e. if (u,v) is in G then (v,u) is not in G.
            If your graph is asymmetric then use the preprocessing.create_dummies method.
        source: Source vertex

    Keyword args:
        weight: Name of the edge attribute. Defaults to 'weight'.

    Returns:
        T: A vertex for which a disjoint path exists has its label set to true.
            Each reachable vertex has p, q, and d assigned.
            The edge-disjoint paths can be reconstructed from p and q.
    """
    # run Dijkstra's algorithm to get shortest path from source to every other vertex
    distance_from_source, shortest_path_from_source = nx.single_source_dijkstra(
        G, source, weight=weight
    )

    vertices_reachable_from_source = set(distance_from_source.keys())

    # remove unreachable vertices
    subgraph = G.subgraph(vertices_reachable_from_source)

    parent_in_tree = find_parents_in_shortest_path_tree(shortest_path_from_source)
    T = SuurballeTree(source, distance_from_source, parent_in_tree)

    # transform the cost of edges from u to v
    T.transformed_cost = adjust_edge_cost_for_graph(
        subgraph, distance_from_source, weight=weight
    )

    # compute pre and post order numbering of T
    preorder(T, source)
    postorder(T, source)

    # initialise incident dict
    T.edges_incident_to_vertex = {v: [] for v in subgraph}
    # only look at edges not in tree
    for u, v in subgraph.edges():
        if not T.has_edge(u, v):
            add_edges_incident_to_endpoints(T, u, v)

    # get the children of each node in T
    T.children = {v: list(T.successors(v)) for v in T}
    T.parent = {v: list(T.predecessors(v))[0] for v in T if v != source}
    T.parent[source] = NULL_VERTEX

    # Dijkstra's algorithm with smart labelling
    while len(T.heap_queue) > 0:
        _, v = heappop(T.heap_queue)
        if not T.labeled[v] and T.tentative_distance[v] < sys.maxsize:
            T.labeled[v] = True  # make v labeled

            # process edges incident to v
            for (u, w) in T.edges_incident_to_vertex[v]:
                process(T, u, w, v)

            # denoted x in the paper
            parent_of_v = T.parent[v]

            # remove v from children of p[v]
            if parent_of_v != NULL_VERTEX and not T.labeled[parent_of_v]:
                # traverse the subtree S containing p[v] by visiting every vertex x in S
                if v in T.children[parent_of_v]:
                    T.children[parent_of_v].remove(v)
                traverse_subtree(T, parent_of_v, v, parent=True)

            # for each child w of v
            children = T.children[v].copy()
            for w in children:
                T.parent[w] = NULL_VERTEX
                T.children[v].remove(w)
                if not T.labeled[w]:
                    traverse_subtree(T, w, v)

    return T


def process(T, u, w, v):
    """
    Process a non-tree edge (u,w).
    The processing of (u,w) is caused by labeling v.
    """
    if not T.has_edge(u, w):
        T.edges_incident_to_vertex[u].remove((u, w))
        T.edges_incident_to_vertex[w].remove((u, w))
        if T.tentative_distance[v] == sys.maxsize:
            raise Exception()
        if T.tentative_distance[v] + T.transformed_cost[u][w] < T.tentative_distance[w]:
            T.tentative_distance[w] = T.tentative_distance[v] + T.transformed_cost[u][w]
            T.tentative_predecessor[w] = u
            T.process_cause[w] = v
            heappush(T.heap_queue, (T.tentative_distance[w], w))


def traverse_subtree(T, y, v, parent=False):
    """
    Traverse every vertex x in the graph, starting at y.
    """
    queue = []
    queue.append(y)
    while len(queue) > 0:
        x = queue.pop(0)
        for child in T.children[x]:
            queue.append(child)
        if parent:
            scan_parent(T, x, v)
        else:
            scan(T, x, y, v)


def scan(T, x, y, v):
    """Scan the edges incident to x

    Args:
        T: Input tree.
        x: Node to scan.
        y: Root of subtree S created by labeling v. y is a child of v.
        v: Node which could cause edges in i[x] to be processed.
    """
    size = len(T.edges_incident_to_vertex[x])
    f_index = 0  # forward index
    b_index = size - 1  # backward index

    # begin scan in forward direction
    while f_index < len(T.edges_incident_to_vertex[x]):
        edge = T.edges_incident_to_vertex[x][f_index]
        u = edge[0]
        w = edge[1]

        # u and w are in the subtree of y
        if u != x and is_ancestor(T, y, u):
            # (u,w) is wasted, abort forward scan
            break

        # u and w are in the subtree of y
        if w != x and is_ancestor(T, y, w):
            # (u,w) is wasted', abort forward scan
            break

        # u and w are in different subtrees, so process edge
        process(T, u, w, v)

        if size > len(T.edges_incident_to_vertex[x]):
            size = len(T.edges_incident_to_vertex[x])
            f_index -= 1
            b_index -= 1
        f_index += 1

    # begin backward scan
    while b_index > f_index:
        edge = T.edges_incident_to_vertex[x][b_index]
        u = edge[0]
        w = edge[1]
        if u != x and is_ancestor(T, y, u):
            # (u,w) is wasted, abort backward scan
            break
        # u and w are in the subtree of y
        if w != x and is_ancestor(T, y, w):
            break

        process(T, u, w, v)
        b_index -= 1


def scan_parent(T, x, v):
    """
    Scan edge edge that is incident to x.
    This method deals with the case when the p(v) is unlabeled when v is labeled.
    Parameters
    __________
    T : SuurballeTree
        Input tree.
    x : node
        Node to scan.
    v : node
        Node which causes edge (u,w) to be processed.
    """
    for (u, w) in T.edges_incident_to_vertex[x]:
        # u and w are in different subtrees so process edge
        if u != x and is_ancestor(T, v, u):
            process(T, u, w, v)
        elif w != x and is_ancestor(T, v, w):
            process(T, u, w, v)

        # u and w are in the same subtree so pass
        else:
            pass  # (u,w) is wasted


def extract_suurballe_edge_disjoint_paths(
    T: SuurballeTree, source: Vertex, target: Vertex
) -> DisjointPaths:
    """Get the two vertex disjoint paths from source to target.

    Algorithm consists of initialisation and two backward traversals
    for each edge-disjoint path.

    Args:
        T: A tree that is the output of Suurballe's algorithm
        source: Source vertex
        target: Target vertex

    Returns:
        Two disjoint paths from source to target

    Note:
        If the target is the source, return ([source], [])

    Raises:
        ValueError: Unexpected problem in the traversal step
    """
    if source == target:
        return ([source], [])

    # INITIALISATION
    paths: DisjointPaths = ([], [])
    mark: Set[Vertex] = set()  # mark vertices when extracting paths

    # case where no edge disjoint paths are found
    if T.tentative_distance[target] == sys.maxsize:
        return paths
    # define x to be t
    x = target

    # repeat the following step until x=s
    while x != source:
        # mark x. Replace x by q[x]
        mark.add(x)
        if T.process_cause[x] == NULL_VERTEX:
            raise ValueError("The process cause q[x] is the null vertex")
        if x == T.process_cause[x]:
            x = source
        else:
            x = T.process_cause[x]

    # TRAVERSAL STEP
    for i in range(2):
        x = target
        paths[i].insert(0, x)
        # repeat following steps until x=s
        while x != source:
            if x in mark:
                mark.remove(x)  # unmark x
                paths[i].insert(
                    0, T.tentative_predecessor[x]
                )  # add (p[x], x) to the front of the path
                if T.tentative_predecessor[x] == NULL_VERTEX:
                    raise ValueError("tentative predecessor p[", x, "] is null vertex")
                x = T.tentative_predecessor[x]  # replace x by p[x].
            else:
                # y is the parent of x in T
                y = list(T.predecessors(x))[0]
                paths[i].insert(0, y)  # add (y,x) to the front of the path
                if y == NULL_VERTEX:
                    raise ValueError("Parent of", x, "is the null vertex")
                x = y  # replace x by y

    return paths


def edge_disjoint_path_cost(tree: SuurballeTree, target: Vertex) -> int:
    """Cost of the least-cost edge-disjoint pair of paths from source to target

    Args:
        tree: Tree returned after executing Suurballe's algorithm
        target: Vertex to calculate cost for

    Returns:
        Total cost of both edge-disjoint paths.
        If no edge-disjoint paths exist, then return sys.maxsize.
    """
    if tree.tentative_distance[target] == sys.maxsize:
        return sys.maxsize
    return 2 * tree.distance_from_source[target] + tree.tentative_distance[target]


def is_ancestor(T: SuurballeTree, ancestor: Vertex, descendant: Vertex) -> bool:
    """True if the passed ancestor vertex is the ancestor of descendant.

    Args:
        T: Tree where each vertex has an attribute 'pre' and 'post' which denotes
            the preorder number and postorder number of the vertex respectively.
        ancestor: The node we are testing is an ancestor.
        descendant: The node we are testing is a descendant.

    Returns:
        True if ancestor is the ancestor of descendant, and false otherwise.

    Note:
        Use the preorder and postorder methods to compute these numberings.
    """
    return (
        T.nodes[ancestor]["pre"] <= T.nodes[descendant]["pre"]
        and T.nodes[ancestor]["post"] >= T.nodes[descendant]["post"]
    )
