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
    catch (const std::exception& exception)
    {
        std::cerr << "An exception was thrown: " << exception.what() << " Closing program." << std::endl;
    }
    catch(...)
    {
        std::cerr << "An unexpected error has occured. Closing program." << std::endl;
    }

    return 0;
}