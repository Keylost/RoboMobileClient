#include "CLP.hpp"

void CLP::usage(const char *progname)
{
	printf(
		"Usage: %s [OPTIONS]\n"
		"Options:\n"
		"\t-s SERVER_IP - use SERVER_IP\n"
		"\t-p PORT      - use PORT\n"
		"\t-v - capture video in file. Example: %s -v myvideo1.avi \n"
		"\t-c - could be combined with -v. Use it if you want get clear video from camera without any signs and rectangles. Example: %s -c -v myvideo1.avi \n"
		"\t-h           - display this help\n",
		progname, progname, progname);
}

int32_t optind = 1;
char* optarg;
int32_t getopt(int argc, char **argv,const char *options)
{
	if (optind == argc) return -1;
	int32_t options_lenght = strlen(options);
	delete[] optarg;
	if (strlen(argv[optind]) == 2 && argv[optind][0] == '-')
	{		
		for (int i = 0; i < options_lenght; i++)
		{
			if (options[i] == ':') continue;
			if (argv[optind][1] == options[i])
			{
				optind++;
				if (i + 1 < options_lenght && options[i + 1] == ':')
				{					
						if (optind < argc && argv[optind][0] != '-')
						{
							optarg = new char[strlen(argv[optind])+1];
							memcpy(optarg, argv[optind], strlen(argv[optind])+1);
							optind++;
							return options[i];
						}
						else return -1;
				}
				else return options[i];
			}
		}
	}

	return -1;
}

void CLP::parse(int argc, char **argv,System &syst)
{
	if (argc == 1) return;
	
	int32_t opt;
	while ((opt = getopt(argc, argv, "hs:p:v:c")) != -1)
	{
		
		switch (opt)
		{
		case 'h':
			usage(argv[0]);
			exit(0);
		case 's':			
			delete[] syst.host;
			syst.host = new char[strlen(optarg)+1];			
			memcpy(syst.host, optarg, strlen(optarg)+1);
			break;
		case 'p':			
			syst.portno = atoi(optarg);			
			break;
		case 'c':
			syst.clear_video = true;
			break;
		case 'v':
			if (strlen(optarg) > 80)
			{
				printf("Too long file name\n");
				exit(1);
			}
			syst.videomaker = true;
			memcpy(syst.videoname, optarg, strlen(optarg)+1);
			break;
		default:
			usage(argv[0]);
			exit(1);
		}
	}
	if (optind < argc)
	{
		usage(argv[0]);
		exit(1);
	}
}
