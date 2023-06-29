#include "../includes/database.h"


int main(int argc, char** argv)
{
	Database database(argc, argv);
    database.run();

    return 0;
}