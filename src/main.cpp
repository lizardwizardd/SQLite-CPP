#include "../includes/database.h"


int main(int argc, char** argv)
{
    Database database(argc, argv);

    try
    {
        database.run();
    }
    catch (const std::runtime_error& error)
    {
        std::cerr << "Error: " << error.what() << " Closing program." << std::endl;
        system("pause");
    }

    return 0;
}