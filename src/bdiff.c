/*
 * bdiff.c
 *
 * Author       : Finn Rayment <finn@rayment.fr>
 * Date created : 23/11/2021
 */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LIMIT 4
#define BLOCK_WIDTH 16

int
usage(char *argv)
{
	fprintf(stderr, "Usage: %s [options] <file1> <file2>\n", argv);
	fprintf(stderr, "Compare two binary files byte by byte.\n");
	fprintf(stderr, "\nOptions:\n");
	fprintf(stderr, "    -c      show coloured output\n");
	fprintf(stderr, "    -g n    num. bytes per output group (default 2)\n");
	fprintf(stderr, "    -h      print this help message\n");
	fprintf(stderr,
	        "    -l n    set number of lines to output, or < 1 for infinite\n");
	fprintf(stderr, "    -v      print the program version\n");
	return EXIT_FAILURE;
}

int
min(int a,
    int b)
{
	if (a < b)
		return a;
	return b;
}

void
print_buffer(unsigned char buf[BLOCK_WIDTH],
             int flags[BLOCK_WIDTH],
             int color,
             int group,
             int dir)
{
	int i, j;
	char c;

	if (dir > 0)
		fputs("> ", stdout);
	else
		fputs("< ", stdout);
	for (i = j = 0; i < BLOCK_WIDTH; ++i)
	{
		if (color && *(flags+i))
			fputs("\x1b[1;31m", stdout);
		else
			fputs("\x1b[0m", stdout);
		fprintf(stdout, "%02x", *(buf+i));
		if (i < BLOCK_WIDTH - 1 && j == group - 1)
		{
			fputc(' ', stdout);
			j = 0;
		}
		else
		{
			++j;
		}
	}
	fputs("  ", stdout);
	for (i = 0; i < BLOCK_WIDTH; ++i)
	{
		if (color && *(flags+i))
			fputs("\x1b[1;31m", stdout);
		else
			fputs("\x1b[0m", stdout);
		c = *(buf+i);
		if (c < 32 || c == 127)
			fputc('.', stdout);
		else
			fputc(c, stdout);
	}
	fputc('\n', stdout);
	fputs("\x1b[0m", stdout);
}

void
print_block(unsigned char bufa[BLOCK_WIDTH],
            unsigned char bufb[BLOCK_WIDTH],
            int flags[BLOCK_WIDTH],
            size_t block,
            int color,
            int group)
{
	fprintf(stdout, "%ld-%ld:\n", block*BLOCK_WIDTH+1, (block+1)*BLOCK_WIDTH);
	print_buffer(bufa, flags, color, group, -1);
	print_buffer(bufb, flags, color, group, 1);
}

int
run(char *file_a,
    char *file_b,
    ssize_t limit,
    int color,
    int group)
{
	FILE *fa, *fb;
	unsigned char bufa[BLOCK_WIDTH], bufb[BLOCK_WIDTH];
	int flags[BLOCK_WIDTH];
	int rda, rdb, i, dirty;
	size_t block;
	ssize_t hits;

	if (!(fa = fopen(file_a, "r")) || !(fb = fopen(file_b, "r")))
	{
		perror("fopen");
		return EXIT_FAILURE;
	}

	block = 0;
	hits = 0;
	while (1)
	{
		dirty = 0;
		memset(bufa,  0, sizeof(unsigned char) * BLOCK_WIDTH);
		memset(bufb,  0, sizeof(unsigned char) * BLOCK_WIDTH);
		memset(flags, 0, sizeof(int) * BLOCK_WIDTH);
		rda = (int) fread(bufa, sizeof(unsigned char), BLOCK_WIDTH, fa);
		rdb = (int) fread(bufb, sizeof(unsigned char), BLOCK_WIDTH, fb);
		if (rda != rdb)
		{
			/* differing read sizes, diff */
			for (i = min(rda, rdb); i < BLOCK_WIDTH; ++i)
				*(flags+i) = 1;
			dirty = 1;
		}
		for (i = 0; i < min(rda, rdb); ++i)
		{
			if (*(bufa+i) != *(bufb+i))
			{
				*(flags+i) = 1;
				dirty = 1;
			}
		}
		if (dirty)
		{
			++hits;
			if (hits < limit || limit <= 0)
				print_block(bufa, bufb, flags, block, color, group);
		}
		++block;
		if (feof(fa) || feof(fb))
			break;
	}
	if (hits >= limit && limit > 0)
		fprintf(stdout, "... and %ld more.\n", hits - limit);

	if (fclose(fa) != 0 || fclose(fb) != 0)
	{
		perror("fclose");
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

int
main(int argc,
     char **argv)
{
	int c, color, group;
	ssize_t limit;
	char *endptr;

	color = 0;
	group = 2;
	opterr = 0;
	limit = LIMIT;

	while ((c = getopt(argc, argv, "cg:hl:v")) != -1)
	{
		switch (c)
		{
		case 'c':
			color = 1;
			break;
		case 'g':
			group = (int) strtol(optarg, &endptr, 10);
			if (endptr != optarg+strlen(optarg) || group < 0)
				return usage(*argv);
			break;
		case 'h':
			usage(*argv);
			return EXIT_SUCCESS;
		case 'l':
			limit = (ssize_t) strtol(optarg, &endptr, 10);
			if (endptr != optarg+strlen(optarg))
				return usage(*argv);
			break;
		case 'v':
			fprintf(stderr, "%s - version %s\n", *argv, VERSION);
			return EXIT_SUCCESS;
		case '?':
			if (optopt == 'l')
				fprintf(stderr, "Option -l requires an argument.\n");
			else if (isprint(optopt))
				fprintf(stderr, "Unknown option -%c.\n", optopt);
			else
				fprintf(stderr, "Unknown option character \\x%x.\n", optopt);
			return usage(*argv);
		}
	}

	if (argc - optind != 2)
		return usage(*argv);

	return run(*(argv+optind), *(argv+optind+1), limit, color, group);
}

