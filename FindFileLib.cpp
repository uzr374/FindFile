#include "FindFileLib.hpp"

namespace FinderLib
{
	FFinder::FFinder(int threads, bool skip_errs, stdfs::path rootp)
	{
		m_num_threads = threads;
		m_skip_errors = skip_errs;
		m_root = rootp;
		opts = stdfs::directory_options::follow_directory_symlink;
		if (skip_errs)
		{
			opts |= stdfs::directory_options::skip_permission_denied;
		}
	}

	FFinder::~FFinder()
	{
		if (m_num_threads > 1)
		{
			for (auto& thr : m_threads)
			{
				if (thr.joinable())
					thr.join();
			}
		}
	}

	void FFinder::findFile_TraverseDir(std::string filename, stdfs::path root)
	{
		stdfs::recursive_directory_iterator dir_iter(root, opts);
		stdfs::path candidate;

		for (const auto& entry : dir_iter)
		{
			if (m_num_threads > 1 //If we are doing MT search, check if this thread should proceed
				|| !candidate.empty())
			{
				std::scoped_lock lck(m_result_guard);
				if (m_search_complete)//Other thread have found element, so this thread should quit
				{
					break;
				}
				else if (!candidate.empty())
				{
					m_found = candidate;
					m_search_complete = true;//Notify other threads to quit
					break;
				}
			}

			if (entry.exists() && entry.is_regular_file() && entry.path().filename() == filename)
			{
				std::cout << entry.path() << '\n';
				candidate = entry.path();
			}
		}
	}

	void FFinder::findFile_MultiThreaded(std::string filename, stdfs::path root, int max_threads)
	{
		stdfs::directory_iterator dir_iter(root, opts);

		stdfs::path candidate;
		int threads_avaliable = max_threads;

		for (const auto& entry : dir_iter)
		{
			const bool exists = entry.exists();
			const bool is_file = entry.is_regular_file();
			const bool is_dir = entry.is_directory();

			if (exists)
			{
				if (is_file && entry.path().filename() == filename)
				{
					std::cout << entry.path() << '\n';
					candidate = entry.path();
				}
				else if (is_dir)
				{
					if (threads_avaliable > 0)
					{
						m_threads.push_back(std::thread(&FFinder::findFile_TraverseDir, this, filename, entry.path()));
						--threads_avaliable;
					}
					else
					{
						findFile_TraverseDir(filename, entry.path());
					}
				}
			}

			{
				std::scoped_lock lck(m_result_guard);
				if (m_search_complete)//Other thread have found element, so this thread should quit
				{
					break;
				}
				else if (!candidate.empty())
				{
					m_found = candidate;
					m_search_complete = true;//Notify other threads to quit
					break;
				}
			}
		}

		for (auto& thr : m_threads)
		{
			if (thr.joinable())
				thr.join();
		}

		m_threads.clear();
	}

	stdfs::path FFinder::findFile(std::string filename, stdfs::path root, int max_threads)
	{
		stdfs::path found;
		max_threads = std::clamp(max_threads, 1, 64);//Make sure that thread count is in meaningful range

		try
		{
			if (max_threads == 1)
			{
				findFile_TraverseDir(filename, root);
			}
			else
			{
				findFile_MultiThreaded(filename, root, max_threads);
			}

			found = m_found;
		}
		catch (stdfs::filesystem_error err)
		{
			m_last_error = err.what();
		}
		catch (std::system_error err)
		{
			m_last_error = err.what();
		}
		catch (...)
		{
			m_last_error = "Unknown error occured";
		}

		return found;
	}

	stdfs::path FFinder::doSearch(std::string filename)
	{
		const auto res = findFile(filename, m_root);
		m_search_complete = false;//Reset if we want to perform another search

		return res;
	}

	std::string FFinder::getLastErrorStr() const
	{
		return m_last_error;
	}
}