#include "clustering.h"
#include "logging.h"

#include <algorithm>

Clustering::Clustering(const Graph& _graph)
    : m_graph(_graph)
{
    m_vertex_community.resize(m_graph.num_vertices, nullptr);
    m_initial_safe_clusters.resize(m_graph.num_vertices, -1);
}
