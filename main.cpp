#include <cassert>
#include <vector>
#include <string>
#include <string_view>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <map>
#include <variant>
#include <stdexcept>

using Int = std::ptrdiff_t;
using Uint = std::size_t;
using Float = double;
using String = std::string;
using StringView = std::string_view;
using InvalidArgument = std::invalid_argument;
template <typename T>
using DynArray = std::vector<T>;
using namespace std::string_literals;
using namespace std::string_view_literals;
using std::endl;
using std::flush;

namespace book_reading
{
	using User = Uint;
	using PageProgress = Uint;

	class Book
	{
	private:
		using UserToProgress = std::unordered_map<User, PageProgress>;

		UserToProgress user_to_progress_;
		DynArray<Uint> progress_to_users_count = {Uint()};

		void PrepareProgressManaging(Uint progress);
	public:

		void Read(User user, PageProgress progress);

		Float Cheer(const User& user) const;
	};
}

namespace comand_reading
{
	using IStream = std::istream;
	using OStream = std::ostream;
	using SStream = std::stringstream;
	using book_reading::User;
	using book_reading::PageProgress;

	struct CheeringComand
	{
		User user = User();
	};
	struct ReadingComand
	{
		User user = User();
		PageProgress progress = PageProgress();
	};

	const String read_header = "READ"s;
	const String cheer_header = "CHEER"s;

	using Comand = std::variant<std::monostate, CheeringComand, ReadingComand>;
	using Comands = std::vector<Comand>;

	class ComandReader
	{
	public:
		Comands ReadComands(IStream& input);
	};
}

void ReleaseFunction();

int main() 
{
	ReleaseFunction();
	return 0;
}

// vvv functions/methods/other implimentations vvv ======================================================================================

namespace book_reading
{
	void Book::PrepareProgressManaging(Uint progress)
	{
		assert(!progress_to_users_count.empty());
		Uint size = progress_to_users_count.size();
		if (++progress > size)
		{
			do
			{
				size *= 2;
			} while (progress > size);
			progress_to_users_count.resize(size);
		}
	}

	void Book::Read(User user, PageProgress progress)
	{
		PageProgress last_progress;
		{
			auto found_it = user_to_progress_.find(user);
			if (found_it == user_to_progress_.end())
			{
				last_progress = PageProgress(0);
				user_to_progress_[user] = progress;
			}
			else
			{
				last_progress = found_it->second;
				found_it->second = progress;
			}
		}

		PrepareProgressManaging(progress);
		for (; last_progress < progress; ++last_progress)
		{
			++progress_to_users_count[static_cast<Uint>(last_progress)];
		}
	}

	Float Book::Cheer(const User& user) const
	{
		auto found_it = user_to_progress_.find(user);
		if (found_it == user_to_progress_.end())
		{
			return Float(0);
		}
		
		PageProgress progress = found_it->second;

		assert(progress < progress_to_users_count.size());
		if (user_to_progress_.size() == 1)
		{
			return Float(1);
		}
		assert(user_to_progress_.size() > 1);

		Uint total_count = user_to_progress_.size();
		Uint others_count = total_count - Uint(1);
		Uint this_pg_count = progress_to_users_count[static_cast<Uint>(progress) - Uint(1)];
		return static_cast<Float>(total_count - this_pg_count) / (others_count);
	}
}

namespace comand_reading
{
	Comands ComandReader::ReadComands(IStream& input)
	{
		Comands result;

		Uint comands_count;
		input >> comands_count;

		result.reserve(comands_count);

		while (result.size() < comands_count)
		{
			String header;
			input >> header;

			if (header.front() == cheer_header.front())
			{
				assert(header == cheer_header);
				CheeringComand comand;
				input >> comand.user;
				result.push_back(std::move(comand));
			}
			else if (header.front() == read_header.front())
			{
				assert(header == read_header);
				ReadingComand comand;
				input >> comand.user >> comand.progress;
				result.push_back(std::move(comand));
			}
			else
			{
				assert(false);
			}
		}

		return result;
	}
}

void ReleaseFunction()
{
	using namespace ::book_reading;
	using namespace ::comand_reading;
	auto& input = std::cin;
	auto& output = std::cout;

	Book book_manager;

	auto comands = ComandReader().ReadComands(input);

	for(const Comand& comand : comands)
	{
		if (std::holds_alternative<CheeringComand>(comand))
		{
			auto& cheer_com = std::get<CheeringComand>(comand);
			auto result = book_manager.Cheer(cheer_com.user);
			output << result << '\n';
		}
		else if (std::holds_alternative<ReadingComand>(comand))
		{
			auto& read_com = std::get<ReadingComand>(comand);
			book_manager.Read(read_com.user, read_com.progress);
		}
		else
		{
			assert(false);
		}
	}	
	output << flush;
}
