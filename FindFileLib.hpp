#pragma once

#include <filesystem>
#include <thread>
#include <string>
#include <iostream>
#include <mutex>
#include <vector>

namespace FinderLib
{
	namespace stdfs = std::filesystem;

	class FFinder final
	{
	private:
		int m_num_threads = 0;
		bool m_skip_errors = false;
		stdfs::path m_root;
		stdfs::directory_options opts = stdfs::directory_options::none;
		std::mutex m_result_guard;
		bool m_search_complete = false;
		stdfs::path m_found;
		std::vector<std::thread> m_threads;
		std::string m_last_error;

	private:
		void findFile_TraverseDir(std::string filename, stdfs::path root);
		void findFile_MultiThreaded(std::string filename, stdfs::path root, int max_threads);
		stdfs::path findFile(std::string filename, stdfs::path root, int max_threads = 0);

	public:
		FFinder() = delete;
		FFinder(const FFinder&) = delete;
		FFinder& operator=(const FFinder&) = delete;

		FFinder(int threads = 1, bool skip_errs = true, stdfs::path rootp = stdfs::current_path().root_path());
		~FFinder();

		stdfs::path doSearch(std::string filename);
		std::string getLastErrorStr() const;
	};
}