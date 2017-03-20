#include "community.h"
#include "logging.h"
#include "clustering.h"
#include "graph.h"

Community::Community(const Graph& _graph,
                     Clustering& _cluster)
    : m_graph(_graph)
    , m_cluster(_cluster)
{
    m_graph_num_vertices = m_graph.num_vertices;
    m_graph_num_edges = m_graph.edges.size();
}

void Community::add_vertex(int64_t vertex)
{

}

void Community::remove_vertex(int64_t vertex)
{

}
