#include <cstdio>
#include <cstring>
#include <getopt.h>
#include "util.h"
#include "global.h"

// use getopt_long to handle arguments
// -h       show help
// -v       show version
// -o file  place results to file
// -d file  dump AST to file
bool handle_opt(int argc, char** argv)
{
    int c;
    int version_flag = 0;
    int help_flag = 0;
    struct option long_options[] =
    {
        {"version", no_argument, &version_flag, 'v'},
        {"help", no_argument, &help_flag, 'h'},
        {"dump", required_argument, NULL, 'd'},
        {0, 0, 0, 0}
    };
    int option_index = 0;
    opterr = 0;

    while ((c = getopt_long(argc, argv, ":hvo:d:", long_options, &option_index)) != -1) {
        switch (c)
        {
            case 0:
                break;
            case 'h':
                help_flag = 1;
                break;
            case 'v':
                version_flag = 1;
                break;
            case 'o':
                strcpy(outfile_name, optarg);
                break;
            case 'd':
				error("Not support yet\n");
                //dumpfile_name = optarg;
                break;
            case '?':
                error("Unknown option -%c\n", optopt);
                return false;
            case ':':
                if (optopt == 'o')
                    error("Option -o requires an argument. Not support yet\n");
                else if (optopt == 'd')
                    error("Option -d requires an argument. Not support yet\n");
                else
                    error("Unknown option -%c\n", optopt);
                return false;
            default:
                error("Error in handle_opt()\n");
        }
    }

    if (help_flag)
    {
        printf("usage: asgn2ast [options] [file]\n");
        printf("-h  --help     print this usage and exit\n");
        printf("-v  --version  print version and exit\n");
        printf("-o <file>      Not support yet. place the output into <file>\n");
        printf("-d <file>      Not support yet. dump AST into <file>\n");
        return false;
    }
    if (version_flag)
    {
        printf("C1 compiler 1.0\n");
        return false;
    }
    if (optind < argc)
        strcpy(infile_name, argv[optind]);
    if (infile_name[0] == '\0')
        infp = stdin;
    else
    {
        infp = fopen(infile_name, "r");
        if (infp == NULL)
        {
            error("Can not open infile %s\n", infile_name);
            return false;
        }
    }
    if (outfile_name[0] == '\0')
        outfp = stdout;
    else
    {
        outfp = fopen(outfile_name, "w");
        if (outfp == NULL)
        {
            error("Can not open outfile %s\n", outfile_name);
            return false;
        }
    }
    /*if (dumpfile_name == NULL)
        dumpfp = NULL;
    else
    {
        dumpfp = fopen(dumpfile_name, "w");
        if (dumpfp == NULL)
        {
            error("Can not open dumpfile %s\n", dumpfile_name);
            return false;
        }
    }*/
    return true;
}
