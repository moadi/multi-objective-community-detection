/*
 * graph.cpp
 *
 *  Created on: Jun 20, 2013
 *      Author: muddy
 */

#include "graph.h"
#include "logging.h"

#include <unordered_set>
#include <chrono>

#include <cinttypes>

using Clock = std::chrono::steady_clock;

void Graph::reset_file_pointer()
{
    fin.clear();
    fin.seekg(0, fin.beg);
}

/*
 * Scans the GML file for "node" entries and updates the counter.
 */
uint64_t Graph::count_vertices_gml()
{
    log_msg("Entering %s", __FUNCTION__);
    std::string line;
    uint64_t result = 0;
    while (!fin.eof()) {
        getline(fin, line);
        if (std::string::npos != line.find("node"))
            result++;
        if (std::string::npos != line.find("edge"))
            break;
    }
    reset_file_pointer();
    log_msg("Leaving %s", __FUNCTION__);
    return result;
}

/**
 * Note: This function does not reset the file pointer.
 * After the vertices section, the edges are expected
 */
uint64_t Graph::count_vertices_pajek()
{
    log_msg("Entering %s", __FUNCTION__);
    std::string line;
    char* location;
    uint64_t result = 0;

    while(std::getline(fin, line)) {
        if (std::string::npos != line.find("Vertices")) {
            location = std::strstr(line.c_str(), "Vertices");
            uint64_t vertex_count = 0;
            int count = sscanf(location, "Vertices %lld", &vertex_count);
            // if no match found
            if (count == 0) {
                std::cerr << "Unable to parse number of vertices in pajek file\n";
                exit(EXIT_FAILURE);
            }
            else
                result = vertex_count;
            break;
        }
    }
    log_msg("Leaving %s", __FUNCTION__);
    return result;
}

void Graph::parse_pajek()
{
    log_msg("Entering %s", __FUNCTION__);

    std::string line;
    num_vertices = count_vertices_pajek();
    vertices.resize(num_vertices);

    //Skip till edge list starts
    do
    {
        std::getline(fin, line);
        if (std::string::npos != line.find("Edges") ||
            std::string::npos != line.find("Arcs")) {

            break;
        }
    } while (!fin.eof());

    int64_t source = -1, target = -1;
    while (fin >> source >> target)
    {
        process_edge(source, target);
    }

    log_msg("Leaving %s", __FUNCTION__);
}

void Graph::parse_gml()
{
    log_msg("Entering %s", __FUNCTION__);

    num_vertices = count_vertices_gml();
    vertices.resize(num_vertices);

    /**
     * Get edges in the GML file
     * The GML edge information looks something like:
     *      edge
     *      [
     *          source <x>
     *          target <y>
     *      ]
     */
    std::string line;
    char* ptr;
    const char* location;
    std::size_t index = -1; // i know, size_t is unsigned

    int64_t source = -1, target = -1;
    while (!fin.eof())
    {
        std::getline(fin, line);
        if (!(std::string::npos != line.find("edge")))
            continue;

        source = target = -1;
        do
        {
            std::getline(fin, line);
            index = line.find("source");
            if (std::string::npos != (index = line.find("source"))) {
                location = &line[index];
                sscanf(location, "source %lli", &source);
            }
            if (std::string::npos != (index = line.find("target"))) {
                location = &line[index];
                sscanf(location, "target %lli", &target);
            }
            if (std::string::npos != line.find("]"))
                break;
        } while (!fin.eof());

        process_edge(source, target);
    }
}

double Graph::calc_avg_degree()
{
    assert(num_vertices > 0 && "Number of vertices must be > 0");
    degree_sum = 0;
    for(const auto& v : vertices)
    {
        degree_sum += v.degree;
    }
    avg_degree = static_cast<double>(degree_sum) / num_vertices;
    return avg_degree;
}


void Graph::done_parsing()
{
    log_msg("Successfully parsed input graph with %lld vertices, %lld edges, average degree: %f",
            num_vertices,
            edges.size(),
            calc_avg_degree());
}

Graph::Graph(const std::string& file_name,
             GraphFormat _format,
             GraphIndex _idx_type) :

        idx_type(_idx_type),
        format(_format)
{
#ifdef TIME_GRAPH_PARSE
    auto start = Clock::now();
#endif

    if(format == GraphFormat::GRAPH_FORMAT_UNKNOWN)
    {
        std::cerr << "Unknown file format, aborting...\n";
        exit(EXIT_FAILURE);
    }

    fin.open(file_name, std::ios::in);
    if (!fin)
    {
        std::cerr << "Unable to open file: " << file_name.c_str() << "\n";
        exit(EXIT_FAILURE);
    }

    if (format == GraphFormat::GRAPH_FORMAT_PAJEK)
        parse_pajek();
    else if (format == GraphFormat::GRAPH_FORMAT_EDGELIST)
        parse_edgelist();
    else
        parse_gml();

    fin.close();

#ifdef TIME_GRAPH_PARSE
    std::chrono::duration<double> time_taken = Clock::now() - start;
    log_msg("Time taken to parse graph = %lld ms", std::chrono::duration_cast<std::chrono::milliseconds>(time_taken).count());
#endif
    done_parsing();
}

void Graph::build_neighb_edges()
{
#if 0
    for (int i = 0; i < num_vertices; ++i) {
        vertex[i].neighb_edge.reserve(vertex[i].degree);
        for (int j = 0; j < vertex[i].degree; ++j) {
            std::pair<int, int> edge;
            if (i < vertex[i].neighbors[j]) {
                edge = std::make_pair(i, vertex[i].neighbors[j]);
            } else {
                edge = std::make_pair(vertex[i].neighbors[j], i);
            }
            auto it = edges.find(edge);
            vertex[i].neighb_edge[j] = &(it->second);
        }
    }
#endif
}

void Graph::parse_edgelist()
{
    log_msg("Entering %s", __func__);

    int64_t source = -1;
    int64_t target = -1;
    std::unordered_set<int64_t> nodes;

    while(fin >> source >> target)
    {
        update_index(source, target);
        check_vertex_ids(source, target);
        nodes.insert(source);
        nodes.insert(target);
        add_edge_to_map(source, target);
    }

    num_vertices = nodes.size();
    vertices.resize(num_vertices);

    for(const auto& e : edges)
    {
        const EdgeKey& key = e.first;
        update_vertices(key.first, key.second);
    }
    log_msg("Leaving %s", __func__);
}
