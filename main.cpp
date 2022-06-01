#include "FindFileLib.hpp"

#include <iostream>
#include <filesystem>

int main(int argc, const char* argv[])
{
	if (argc < 2)
	{
		std::cerr << "Usage: filefinder name filename.ext\n";
		return -1;
	}

	const std::string filename = (std::string(argv[1]) == "name") ? argv[2] : argv[1];
	constexpr auto num_threads = 8 - 1;//take into account main thread
	const auto root = std::filesystem::current_path().root_path();
	FinderLib::FFinder finder(num_threads, true, root);
	const auto path = finder.doSearch(filename);
	if (path.empty())
	{
		std::cout << "Failed to find requested file in:" << root << '\n';
	}
	else
	{
		std::cout << "Successfully found file " << filename << " in directory: " << path << '\n';
	}

	std::cin.get();

	return 0;
}