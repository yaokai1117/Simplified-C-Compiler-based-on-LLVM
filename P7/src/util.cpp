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
            case 'd':
                dumpfile_name = optarg;
                break;
            case '?':
                printf("Unknown option -%c\n", optopt);
                return false;
            case ':':
                if (optopt == 'o')
                    printf("Option -o requires an argument. Not support yet\n");
                else if (optopt == 'd')
                    printf("Option -d requires an argument. Not support yet\n");
                else
                    printf("Unknown option -%c\n", optopt);
                return false;
            default:
                printf("Error in handle_opt()\n");
        }
    }

    if (help_flag)
    {
        printf("usage: asgn2ast [options] [file]\n");
        printf("-h  --help     print this usage and exit\n");
        printf("-v  --version  print version and exit\n");
        printf("-d <file>      dump AST into <file>\n");
        return false;
    }
    if (version_flag)
    {
        printf("C1 compiler 1.2\n");
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
            printf("Can not open infile %s\n", infile_name);
            return false;
        }
    }
    if (dumpfile_name == NULL)
        dumpfp = NULL;
    else
    {
        dumpfp = fopen(dumpfile_name, "w");
        if (dumpfp == NULL)
        {
            printf("Can not open dumpfile %s\n", dumpfile_name);
            return false;
        }
    }
    return true;
}
