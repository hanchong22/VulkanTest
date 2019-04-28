#include "VulkanBase.h"
#include <memory>

int main() {
	std::unique_ptr<VulkanBase> app(new VulkanBase());
	//VulkanBase *app = new VulkanBase();

	try {
		app->Run();
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	//delete app;

	return EXIT_SUCCESS;
}
