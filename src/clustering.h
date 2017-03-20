#pragma once

#include <vector>
#include <unordered_set>

#include "community.h"

class Graph;

using CommunityEdge = std::pair<Community*, Community*>;

class Clustering
{
    public:
        Clustering(const Graph& _graph);

    private:
        const Graph& m_graph;

        std::vector<Community> m_communities;

        /**
         * Represents an edge between 2 communities and the total
         * number of edges from the original graph that connect vertices
         * in both communities
         */
        std::unordered_map<CommunityEdge, uint32_t> m_community_edges;

        // cache a pointer to each community that
        // a vertex in the graph is part of
        std::vector<Community*> m_vertex_community;

        // stores the initial clustering
        // generated when a safe individual is created.
        // the initial set of partitions will be built
        // using this vector
        std::vector<int64_t> m_initial_safe_clusters;

        /**
         * Store the total in and out degrees for each vertex
         * in the graph
         */
        std::vector<int64_t> m_vertex_out_degree;
        std::vector<int64_t> m_vertex_in_degree;

        // stores the different communities each vertex is connected to, along
        // with the corresponding outdegree
        std::vector<std::unordered_map<Community*, uint32_t>> m_vertex_conns;

        long double m_modularity = 0;
        long double m_conductance = 0;
};
