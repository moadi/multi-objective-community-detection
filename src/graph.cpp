/*
 * graph.cpp
 *
 *  Created on: Jun 20, 2013
 *      Author: muddy
 */

#include "graph.h"

using namespace std;

int edge_counter;

/*
 * Function to reset the file pointer to the beginning of GML file
 * Takes as a parameter a reference to the file pointer, clears the
 * flags and sets the pointer to the beginning.
 */
void reset_pointer(ifstream &fin)
{
	fin.clear();
	fin.seekg(0, fin.beg);
}


/*
 * Function to count the number of vertices in GML file.
 * Scans the file for "node" entries and updates the counter.
 * Returns the number of vertices.
 */
int count_vertices(ifstream &fin) {
	string line;
	int result = 0;
	while (!fin.eof())
	{
		getline(fin, line);
		if (string::npos != line.find("node"))
			result++;
		if (string::npos != line.find("edge"))
			break;
	}
	reset_pointer(fin);
	return result;
}

/*
 * Count vertices in Pajek file
 */
int count_vertices_pajek(ifstream& fin)
{
	string line;
	char * ptr;
    char * location;
	int result;
	getline(fin, line);
	if(string::npos != line.find("Vertices"))
	{
		ptr = new char[line.size() + 1];
		ptr[line.size()] = '\0';
		memcpy(ptr, line.c_str(), line.size());
		location = strstr(ptr, "Vertices");
		sscanf(location, "Vertices %d", &result);
        delete[] ptr;
	}
	return result;
}

Graph::Graph(char * fileName)
{
	ifstream fin(fileName);

	if(!fin)
	{
		cerr << "Invalid file name! \n";
		exit(EXIT_FAILURE);
	}

	bool isKarate = false;

	string file(fileName);

	if(string::npos != file.find("karate.gml"))
		isKarate = true;

	if (string::npos != file.find(".net"))
	{
		string line;
		num_vertices = count_vertices_pajek(fin);

		vertex = new Vertex[num_vertices];
		//vertex.reserve(num_vertices);

		for(int i = 0; i < num_vertices; i++)
		{
			vertex[i].id = i;
			vertex[i].degree = 0;
		}
        int vertex_counter = 0;
		//Read vertex IDs
		do
		{
			getline(fin, line);
//			string id_text;
			if(string::npos != line.find("Edges") || string::npos != line.find("Arcs"))
				break;
//            std::string::size_type start_pos = line.find("\"");
//			int id;
//			if (string::npos != start_pos)
//			{
//				++start_pos;
//                std::string::size_type i;
//				for(i = start_pos; line[i] != '\"'; i++);
//				id_text = line.substr(start_pos, i - start_pos);
//				id = atoi(id_text.c_str());
//                vertex[vertex_counter++].node_id = id;
//				id--;
//				vertex[id].id = id;
//				vertex[id].degree = 0;
//			}
		}
		while(!fin.eof());



		int source, target;

		//Calculate the degree of each vertex
		while(fin >> source >> target)
		{
			vertex[--source].degree++;
			vertex[--target].degree++;
		}

		//Calculate the number of edges to resize the map
		int tot_deg = 0;
		for(int i = 0;i < num_vertices; i++)
		{
			tot_deg += vertex[i].degree;
		}
        
		num_edges = tot_deg/2;

//		cout << "Average degree = " << (double) tot_deg / num_vertices << "\n\n";
        
//      cout << "Total degree = " << tot_deg << "\n\n";

		edges.reserve(num_edges);

		reset_pointer(fin);

		//Loop till we reach the edges entry again
		while(!fin.eof())
		{
			getline(fin, line);
			if (string::npos != line.find("Edges"))
				break;
		}

		//Initialize the neighbors
		for(int i = 0; i < num_vertices; i++)
		{
			vertex[i].neighbors = new int[vertex[i].degree];
			//vertex[i].neighbors.reserve(vertex[i].degree);
		}
		int * count = new int[num_vertices]();
		while (fin >> source >> target)
		{
			--source;
			--target;
			pair<int, int> edge_key;
			Edge edge;
			if(source < target)
			{
				edge.v1 = source;
				edge.v2 = target;
				edge_key = make_pair(source, target);
			}
			else
			{
				edge.v1 = target;
				edge.v2 = source;
				edge_key = make_pair(target, source);
			}
			edges.insert(make_pair(edge_key, edge));
			vertex[source].neighbors[count[source]] = target;
			vertex[target].neighbors[count[target]] = source;
			count[source]++;
			count[target]++;
		}
		delete[] count;
		fin.close();
	}
	else
	{
		//cout<<"Calculating number of vertices... \n";
		num_vertices = count_vertices(fin); //count all vertices
		//cout<<"Done \n";

		vertex = new Vertex[num_vertices]; //create array of vertices
		//vertex.reserve(num_vertices);

		//Get all vertex IDs
		string line;
		char * ptr;
        char * location;
		int id;
		for(int i=0; i < num_vertices; i++)
		{
			vertex[i].degree = 0;
			do
			{
				getline(fin, line);
			}
			while(!(string::npos != line.find("node")));

			do
			{
				getline(fin, line);
				if (string::npos != line.find("id"))
                {
                    ptr = new char[line.size()+1];
                    ptr[line.size()]='\0';
                    memcpy(ptr, line.c_str(), line.size());
                    location = strstr(ptr, "id");
                    if(isKarate)
                    {
                        sscanf(location, "id %d", &id);
                        id--;
                        vertex[i].id = id;
                    }
                    else
                        sscanf(location, "id %d", &vertex[i].id);
                    
                    delete[] ptr;
                }
			if (string::npos != line.find("]"))
				break;
			}
			while(!fin.eof());
		}

		//Get the degree of each vertex
		int source, target;
		while (!fin.eof())
		{
			getline(fin, line);
			if (!(string::npos != line.find("edge")))
				continue;

			source = target = -1;
			do
			{
				getline(fin, line);
				if(string::npos != line.find("source"))
				{
					ptr = new char[line.size()+1];
					ptr[line.size()]='\0';
					memcpy(ptr, line.c_str(), line.size());
					location = strstr(ptr, "source");
					sscanf(location, "source %i", &source);
                    delete[] ptr;
				}
				if(string::npos != line.find("target"))
				{
					ptr = new char[line.size()+1];
					ptr[line.size()]='\0';
					memcpy(ptr, line.c_str(), line.size());
					location = strstr(ptr, "target");
					sscanf(location, "target %i", &target);
                    delete[] ptr;
				}
				if(string::npos != line.find("]"))
					break;
			}
			while(!fin.eof());

			if(source>=0 && target>=0)
			{
				if(isKarate)
				{
					vertex[--source].degree++;
					vertex[--target].degree++;
				}
				else
				{
					vertex[source].degree++;
					vertex[target].degree++;
				}
			}
		}

		//Calculate the number of edges to resize the map
		int tot_deg = 0;
		for(int i = 0;i < num_vertices; i++)
		{
			tot_deg += vertex[i].degree;
		}
		num_edges = tot_deg/2;

	//	cout << "Average degree = " << (double) tot_deg / num_vertices << "\n\n";

		edges.reserve(num_edges);

		//Read in the edges
		reset_pointer(fin);
		int * count;
		for(int i=0; i < num_vertices; i++)
		{
			vertex[i].neighbors = new int[vertex[i].degree];
			//vertex[i].neighbors.reserve(vertex[i].degree);
		}

		count = new int[num_vertices](); //zero-initialize temporary edge counts
		while(!fin.eof())
		{
			getline(fin, line);
			if (!(string::npos != line.find("edge"))) //loop till edge entry
					continue;

			source = target = -1;
			do
			{
				getline(fin, line);
				if(string::npos != line.find("source"))
				{
					//grab the source
					ptr = new char[line.size()+1];
					ptr[line.size()]='\0';
					memcpy(ptr, line.c_str(), line.size());
					location = strstr(ptr, "source");
					sscanf(location, "source %i", &source);
                    delete[] ptr;
				}
				if(string::npos != line.find("target"))
				{
					//grab the target
					ptr = new char[line.size()+1];
					ptr[line.size()]='\0';
					memcpy(ptr, line.c_str(), line.size());
					location = strstr(ptr, "target");
					sscanf(location, "target %i", &target);
                    delete[] ptr;
				}
				if(string::npos != line.find("]")) //break if "]" is found
					break;
			}
			while(!fin.eof());
			if (source>=0 && target>=0)
			{
				if(isKarate)
				{
					target--;
					source--;
				}
				vertex[source].neighbors[count[source]] = target;
				vertex[target].neighbors[count[target]] = source;

				//update counts
				count[source]++;
				count[target]++;

				if(source < target)
				{
					pair<int, int> key;
					key = make_pair(source, target);
					Edge edge;
					edge.v1 = source;
					edge.v2 = target;
					edges.insert(make_pair(key, edge));
					//edges[key] = edge;
				}
				else
				{
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
}

void Graph::build_neighb_edges()
{
    for (int i = 0; i < num_vertices; ++i)
    {
        vertex[i].neighb_edge.reserve(vertex[i].degree);
        for (int j = 0; j < vertex[i].degree; ++j)
        {
            std::pair<int, int> edge;
            if (i < vertex[i].neighbors[j])
            {
                edge = std::make_pair(i, vertex[i].neighbors[j]);
            }
            else
            {
                edge = std::make_pair(vertex[i].neighbors[j], i);
            }
            auto it = edges.find(edge);
            vertex[i].neighb_edge[j] = &(it->second);
        }
    }
}

Graph::~Graph()
{
	//cout << "In graph destructor\n\n";
	delete[] vertex;
}

Vertex::~Vertex()
{
//	cout << "In vertex destructor\n\n";
	if (neighbors != nullptr)
	{
		delete[] neighbors;
		neighbors = nullptr;
	}

	if(common != nullptr)
	{
		delete[] common;
		common = nullptr;
	}
}

void Graph::displayCount()
{
	int total_degree = 0;
	cout<<"The number of vertices in this graph is = "<<num_vertices<<"\n";
	for(int i=0; i < num_vertices; i++)
	{
		total_degree+=vertex[i].degree;
	}
	cout<<"The number of edges in this graph is = "<<total_degree/2<<"\n";
}

Edge::Edge()
{
	initPhm = 1.0;
	phm = 1.0;
	nVisited = 0;
}
