#pragma once

#include <vector>
#include <list>
#include <unordered_set>
#include <tuple>

class Clustering;
class Graph;

class Community
{
    public:
        Community(const Graph& _graph,
                  Clustering& _clustering);

        void add_vertex(int64_t vertex);
        void remove_vertex(int64_t vertex);

    private:
        const Graph& m_graph;
        const uint64_t m_graph_num_vertices;
        const uint64_t m_graph_num_edges;

        // Store a reference to the top level cluster
        // for looking up vertex outdegree and indegree
        Clustering& m_cluster;

        /**
         * Following members describe the structure
         * of the community
         */
        int64_t m_id = -1;

        // the vertices of the original graph that belong
        // to this partition
        std::unordered_set<int64_t> m_vertices;

        // map of neighboring communities and the total
        // number of edges from the original graph that fall
        // between them
        std::unordered_set<Community*> m_neighbors;

        uint32_t m_total_outdegree = 0;
        uint32_t m_total_indegree = 0;
        uint32_t m_degree_sum = 0;
        uint32_t m_total_internal_edges = 0;
};
