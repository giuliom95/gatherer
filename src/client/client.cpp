#include "application.hpp"

int main()
{
	Application app;

	while (app.loop()) {}
	
	return 0;	
}