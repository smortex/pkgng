#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <libutil.h>
#include <sysexits.h>

#include <pkg.h>

#include "search.h"

void
usage_search(void)
{
	fprintf(stderr, "usage, pkg search [-gxXcd] pattern\n");
	fprintf(stderr, "For more information see 'pkg help search'.\n");
}

int
exec_search(int argc, char **argv)
{
	char *pattern;
	match_t match = MATCH_EXACT;
	int  retcode = 0;
	unsigned int field = REPO_SEARCH_NAME;
	char size[7];
	int ch;
	struct pkgdb *db = NULL;
	struct pkgdb_it *it = NULL;
	struct pkg *pkg = NULL;

	while ((ch = getopt(argc, argv, "gxXcd")) != -1) {
		switch (ch) {
			case 'g':
				match = MATCH_GLOB;
				break;
			case 'x':
				match = MATCH_REGEX;
				break;
			case 'X':
				match = MATCH_EREGEX;
				break;
			case 'c':
				field |= REPO_SEARCH_COMMENT;
				break;
			case 'd':
				field |= REPO_SEARCH_DESCRIPTION;
				break;
			default:
				usage_search();
				return (EX_USAGE);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc != 1) {
		usage_search();
		return (EX_USAGE);
	}

	pattern = argv[0];

	if (pkgdb_open(&db, PKGDB_REMOTE) != EPKG_OK) {
		pkg_error_warn("can not open database");
		retcode = EPKG_FATAL;
		goto cleanup;
	}

	if ((it = pkgdb_rquery(db, pattern, match, field)) == NULL) {
		pkg_error_warn("can not query database");
		retcode = EPKG_FATAL;
		goto cleanup;
	}

	while (( retcode = pkgdb_it_next(it, &pkg, PKG_LOAD_BASIC)) == EPKG_OK) {
		printf("Name: %s\n", pkg_get(pkg, PKG_NAME));
		printf("Version: %s\n", pkg_get(pkg, PKG_VERSION));
		printf("Origin: %s\n", pkg_get(pkg, PKG_ORIGIN));
		printf("Comment: %s\n", pkg_get(pkg, PKG_COMMENT));
		printf("\n");
	}

	cleanup:
	if (it != NULL)
		pkgdb_it_free(it);

	if (db != NULL)
		pkgdb_close(db);

	return (retcode);

}
