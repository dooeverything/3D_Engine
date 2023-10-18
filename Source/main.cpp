#include "Renderer.h"

using namespace std;

int main(int argc, char** argv)
{
	auto renderer = make_unique<Renderer>();
	//renderer->init();
	renderer->run();
	renderer->end();
	return 0;
}