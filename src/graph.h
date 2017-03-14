#ifndef GRAPH_H_
#define GRAPH_H_

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <map>
#include <functional>
#include <cstring>
#include <utility>
#include <string>
#include <vector>
#include <cstdint>

/*
 * Specialize std::hash for pair so we can use it as a key
 * in the hashtable.
 * Found here:
 * http://stackoverflow.com/questions/19655733/how-to-use-
 * unordered-set-that-has-elements-that-are-vector-of-pairint-int?rq=1
 */
template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
  std::hash<T> hasher;
  seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

namespace std
{
  template<typename S, typename T>
  struct hash<pair<S, T>>
  {
    inline size_t operator()(const pair<S, T> & v) const
    {
      size_t seed = 0;
      ::hash_combine(seed, v.first);
      ::hash_combine(seed, v.second);
      return seed;
    }
  };
}

enum class GraphFormat : uint8_t
{
    GRAPH_FORMAT_PAJEK,
    GRAPH_FORMAT_GML,
    GRAPH_FORMAT_EDGELIST
};

/**
 * Describes whether the vertex indices in the source
 * file are 1-based or 0-based
 */
enum class GraphIndex : uint8_t
{
    GRAPH_INDEX_ZERO_BASED,
    GRAPH_INDEX_ONE_BASED
};


struct Edge
{
    int64_t v1 = -1;
    int64_t v2 = -1;
};

using EdgeKey = std::pair<int64_t, int64_t>;

struct Vertex
{
    int64_t id      = -1;
    int64_t degree  = 0;
    std::vector<int64_t> neighbors;
    std::vector<int64_t> common;
    //std::vector<Edge*> neighb_edges;
};

class Graph
{
	public:
		uint64_t num_vertices   = 0;
		uint64_t num_edges      = 0;
		std::vector<Vertex> vertices;
		std::map<int64_t, int64_t> edges;

		Graph(const std::string& file_name,
		      GraphFormat _format = GraphFormat::GRAPH_FORMAT_PAJEK,
		      GraphIndex _idx_type = GraphIndex::GRAPH_INDEX_ONE_BASED);

		void displayCount();
        void build_neighb_edges();

	private:
        // assume that the vertices in the source file are 1-based
        // and the format is Pajek
        GraphIndex idx_type = GraphIndex::GRAPH_INDEX_ONE_BASED;
        GraphFormat format = GraphFormat::GRAPH_FORMAT_PAJEK;

        // internal file handle for the source file, closed after constructor completes
        std::ifstream fin;

        void parse_pajek();
        void parse_gml();
        void parse_edgelist();

        uint64_t count_vertices_pajek();
        uint64_t count_vertices_gml();
        void reset_file_pointer();
};

#endif /* GRAPH_H_ */
