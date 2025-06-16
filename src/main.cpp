#include <HelloTriangleApplication.h>
int main() {
	HelloTriangleApplication app;

	try {
		app.m_run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return EXIT_SUCCESS;
}