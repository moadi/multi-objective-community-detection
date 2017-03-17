#include <iostream>
#include <unistd.h>
#include <string>

#include "graph.h"
#include "logging.h"

struct CmdLineArgs
{
    std::string input_file;
    std::string format_str;
    GraphIndex index_type = GraphIndex::GRAPH_INDEX_ONE_BASED;
    GraphFormat graph_format = GraphFormat::GRAPH_FORMAT_UNKNOWN;

    bool index_specified = false;
    bool format_specified = false;
    bool input_file_specified = false;
};

void parse_cmd_line_args(int argc, char* argv[], CmdLineArgs& args)
{
    int c = -1;
    while((c = getopt(argc, argv, "i:zf:")) != -1)
    {
        switch(c)
        {
            case 'i':
            {
                args.input_file = std::string(optarg);
                break;
            }
            case 'z':
            {
                args.index_specified = true;
                args.index_type = GraphIndex::GRAPH_INDEX_ZERO_BASED;
                break;
            }
            case 'f':
            {
                args.format_specified = true;
                args.format_str = std::string(optarg);
                break;
            }
            case '?':
            {
                if(optopt == 'i' || optopt == 'f')
                    std::cerr << "Option -%c requires an argument" << optopt;
                else
                    std::cerr << "Unknown option character" << optopt;

                exit(EXIT_FAILURE);
            }
        }
    }
}

inline GraphFormat get_format(const std::string& str)
{
    GraphFormat format = GraphFormat::GRAPH_FORMAT_UNKNOWN;
    if(str == "gml")
        format = GraphFormat::GRAPH_FORMAT_GML;
    else if(str == "edgelist")
        format = GraphFormat::GRAPH_FORMAT_EDGELIST;
    else if(str == "net")
        format = GraphFormat::GRAPH_FORMAT_PAJEK;
    return format;
}

static GraphFormat guess_format(const std::string& input_file)
{
    size_t index = input_file.find_last_of(".");
    GraphFormat format = GraphFormat::GRAPH_FORMAT_UNKNOWN;
    if(index != std::string::npos)
    {
        format = get_format(input_file.substr(index+1));
    }
    else
        log_msg("No file extension specified: %s", input_file.c_str());

    return format;
}

int main(int argc, char* argv[])
{
    CmdLineArgs args;
    parse_cmd_line_args(argc, argv, args);

    if(args.input_file.length() > 0)
        log_msg("Input file: %s", args.input_file.c_str());

    if(args.index_specified)
        log_msg("Setting index to 0-based");

    if(args.format_specified)
    {
        log_msg("Input file format is: %s", args.format_str.c_str());
        args.graph_format = get_format(args.format_str);
    }
    else
        args.graph_format = guess_format(args.input_file);

    Graph g(args.input_file, args.graph_format, args.index_type);
	return 0;
}
