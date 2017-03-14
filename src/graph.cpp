/*
 * graph.cpp
 *
 *  Created on: Jun 20, 2013
 *      Author: muddy
 */

#include "graph.h"
#include "logging.h"

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
            int count = sscanf(location, "Vertices %d", &vertex_count);
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
    do {
        getline(fin, line);
        if (std::string::npos != line.find("Edges") ||
            std::string::npos != line.find("Arcs")) {

            break;
        }
    } while (!fin.eof());

    // Parse the edges listed in the pajek file
    // and build up the degree count and neighbor
    // list
    uint64_t edge_counter = 0;
    int64_t source = -1, target = -1;
    while (fin >> source >> target)
    {
        if (idx_type == GraphIndex::GRAPH_INDEX_ONE_BASED)
        {
            --source;
            --target;
        }

        vertices[source].degree++;
        vertices[target].degree++;
        vertices[source].neighbors.push_back(target);
        vertices[target].neighbors.push_back(source);
        edges.emplace(std::make_pair(source, target));
    }

    if(!fin)
    {
        std::cerr << "An error occurred while parsing pajek edgelist \n";
        exit(EXIT_FAILURE);
    }
    fin.close();

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
    char * ptr;
    char * location;

    int64_t source = -1, target = -1;
    while (!fin.eof())
    {
        std::getline(fin, line);
        if (!(std::string::npos != line.find("edge")))
            continue;

        source = target = -1;
        do {
            getline(fin, line);
            if (string::npos != line.find("source")) {
                ptr = new char[line.size() + 1];
                ptr[line.size()] = '\0';
                memcpy(ptr, line.c_str(), line.size());
                location = strstr(ptr, "source");
                sscanf(location, "source %i", &source);
                delete[] ptr;
            }
            if (string::npos != line.find("target")) {
                ptr = new char[line.size() + 1];
                ptr[line.size()] = '\0';
                memcpy(ptr, line.c_str(), line.size());
                location = strstr(ptr, "target");
                sscanf(location, "target %i", &target);
                delete[] ptr;
            }
            if (string::npos != line.find("]"))
                break;
        } while (!fin.eof());

        if (source >= 0 && target >= 0) {
            if (isKarate) {
                vertex[--source].degree++;
                vertex[--target].degree++;
            } else {
                vertex[source].degree++;
                vertex[target].degree++;
            }
        }
    }

    //Calculate the number of edges to resize the map
    int tot_deg = 0;
    for (int i = 0; i < num_vertices; i++) {
        tot_deg += vertex[i].degree;
    }
    num_edges = tot_deg / 2;

    //  cout << "Average degree = " << (double) tot_deg / num_vertices << "\n\n";

    edges.reserve(num_edges);

    //Read in the edges
    reset_pointer(fin);
    int * count;
    for (int i = 0; i < num_vertices; i++) {
        vertex[i].neighbors = new int[vertex[i].degree];
        //vertex[i].neighbors.reserve(vertex[i].degree);
    }

    count = new int[num_vertices](); //zero-initialize temporary edge counts
    while (!fin.eof()) {
        getline(fin, line);
        if (!(string::npos != line.find("edge"))) //loop till edge entry
            continue;

        source = target = -1;
        do {
            getline(fin, line);
            if (string::npos != line.find("source")) {
                //grab the source
                ptr = new char[line.size() + 1];
                ptr[line.size()] = '\0';
                memcpy(ptr, line.c_str(), line.size());
                location = strstr(ptr, "source");
                sscanf(location, "source %i", &source);
                delete[] ptr;
            }
            if (string::npos != line.find("target")) {
                //grab the target
                ptr = new char[line.size() + 1];
                ptr[line.size()] = '\0';
                memcpy(ptr, line.c_str(), line.size());
                location = strstr(ptr, "target");
                sscanf(location, "target %i", &target);
                delete[] ptr;
            }
            if (string::npos != line.find("]")) //break if "]" is found
                break;
        } while (!fin.eof());
        if (source >= 0 && target >= 0) {
            if (isKarate) {
                target--;
                source--;
            }
            vertex[source].neighbors[count[source]] = target;
            vertex[target].neighbors[count[target]] = source;

            //update counts
            count[source]++;
            count[target]++;

            if (source < target) {
                pair<int, int> key;
                key = make_pair(source, target);
                Edge edge;
                edge.v1 = source;
                edge.v2 = target;
                edges.insert(make_pair(key, edge));
                //edges[key] = edge;
            } else {
                pair<int, int> key;
                key = make_pair(target, source);
                Edge edge;
                edge.v1 = target;
                edge.v2 = source;
                edges.insert(make_pair(key, edge));
                //edges[key] = edge;
            }
        }
    }
    delete[] count;
    fin.close();
}

Graph::Graph(const std::string& file_name,
             GraphFormat _format = GraphFormat::GRAPH_FORMAT_PAJEK,
             GraphIndex _idx_type = GraphIndex::GRAPH_INDEX_ONE_BASED) :

        idx_type(_idx_type),
        format(_format)
{
    std::ifstream fin(file_name);
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

    }
}

void Graph::build_neighb_edges()
{
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
}

Vertex::~Vertex()
{
//	cout << "In vertex destructor\n\n";
    if (neighbors != nullptr) {
        delete[] neighbors;
        neighbors = nullptr;
    }

    if (common != nullptr) {
        delete[] common;
        common = nullptr;
    }
}

void Graph::displayCount()
{
    int total_degree = 0;
    cout << "The number of vertices in this graph is = " << num_vertices
            << "\n";
    for (int i = 0; i < num_vertices; i++) {
        total_degree += vertex[i].degree;
    }
    cout << "The number of edges in this graph is = " << total_degree / 2
            << "\n";
}

Edge::Edge()
{
    initPhm = 1.0;
    phm = 1.0;
    nVisited = 0;
}
