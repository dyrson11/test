#include "../headers/graph.h"

template<typename N, typename E>
bool graph<N,E>::insertNode(Node* x)
{
    m_nodes_list.insert(x);
}

template<typename N, typename E>
bool graph<N,E>::insertEdge(Node *a, Node *b)
{
    Edge *edge1 = new Edge;
    edge1->m_nodes[0] = a;
    edge1->m_nodes[1] = b;
    a->m_edges.insert(edge1);
    b->m_edges.insert(edge1);
    m_edges_list.insert(edge1);
}

template<typename N, typename E>
bool graph<N,E>::removeNode(Node *x)
{
    typename unordered_set <Edge*>::iterator i;
    for( i = x->m_edges.begin(); i != x->m_edges.end(); i++)
    {
        removeEdge(*i, x == (*i)->m_nodes[0]);
    }
    m_nodes_list.erase(m_nodes_list.find(x));
    delete x;
}

template<typename N, typename E>
bool graph<N,E>::removeEdge(Edge *x, bool dir)//can change with a direction bool
{
    Node *a = x->m_nodes[dir];
    if(a->m_edges.find(x) == a->m_edges.end())
    {
        return false;
    }
    a->m_edges.erase(a->m_edges.find(x));
    m_edges_list.erase(m_edges_list.find(x));
    return true;
}

//cGraph<float, float> graph;