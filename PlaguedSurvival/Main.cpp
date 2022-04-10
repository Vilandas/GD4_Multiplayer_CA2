#include <iostream>
#include "Application.hpp"

/**
 * Vilandas Morrissey - D00218436
 */

int main()
{
	try
	{
		Application app;
		app.Run();
	}
	catch (std::exception& e)
	{
		std::cout << "\nEXCEPTION: " << e.what() << std::endl;
	}
}