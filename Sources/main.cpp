#include "Renderer.h"

using namespace std;

int main(int argc, char** argv)
{
	auto renderer = make_unique<Renderer>();

	if (!renderer->init())
	{
		cout << "Failed to init" << endl;
		return 0;
	}

	renderer->run();

	renderer->end();

	return 0;
}