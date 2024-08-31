// Project Identifier: 50EB44D3F029ED934858FFFCEAC3547C68768FC9
#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <limits>
#include <deque>
#include <optional>

struct cmd_line
{
	enum class method : bool
	{
		stack,
		queue,
	};

	enum class format : bool
	{
		word,
		mod,
	};

	cmd_line();

	void load(int argc, char **argv);

	std::string begin, end;
	method method;
	format format;
	bool can_do_change, can_do_length, can_do_swap;
};

std::vector<std::string> get_dictionary();

void do_queue_search(const cmd_line &cmd, const std::vector<std::string> &dictionary);
void do_stack_search(const cmd_line &cmd, const std::vector<std::string> &dictionary);

int main(int argc, char **argv)
{
	cmd_line cmd;

	try
	{
		cmd.load(argc, argv);
	}
	catch(const std::runtime_error& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	std::vector<std::string> dictionary;
	try
	{
		dictionary = get_dictionary();
	}
	catch(const std::exception& e)
	{
		std::cerr << e.what() << '\n';
		return 1;
	}

	if (std::find(dictionary.begin(), dictionary.end(), cmd.begin) == dictionary.end())
	{
		std::cerr << "Beginning word does not exist in the dictionary\n";
		return 1;
	}

	if (std::find(dictionary.begin(), dictionary.end(), cmd.end) == dictionary.end())
	{
		std::cerr << "Ending word does not exist in the dictionary\n";
		return 1;
	}

	if (cmd.method == cmd_line::method::queue)
		do_queue_search(cmd, dictionary);
	else
		do_stack_search(cmd, dictionary);

	return 0;
}

void output_help()
{
	std::cout << "This program calculates a path from a starting word to an ending word using specified modification abilities\n"
				 "Options:\n"
				 "  --help/-h          Display this menu\n"
				 "  --queue/-q         Use breadth first search\n"
				 "  --stack/-s         Use depth first search\n"
				 "  --begin/-b <word>  Specify begin word\n"
				 "  --end/-e <word>    Specify end word\n"
				 "  --output/-o (W|M)  Specify either word format (W), or mod format (M)\n"
				 "  --change/-c        Allow letterman to change one letter\n"
				 "  --length/-l        Allow letterman to insert or delete a letter\n"
				 "  --swap/-p          Allow letterman to swap any two adjacent letters\n";
	std::exit(0);
}

cmd_line::cmd_line() : format{format::word}, can_do_change{}, can_do_length{}, can_do_swap{}
{
}

void cmd_line::load(int argc, char **argv)
{
	bool found_method = false;
	for (int i = 1; i < argc; ++i)
	{
		std::string cur = argv[i];
		if (cur == "--queue")
		{
			if (found_method)
				throw std::runtime_error("Conflicting or duplicate stack and queue specified");
			
			method = method::queue;
			found_method = true;
		}
		else if (cur == "--stack")
		{
			if (found_method)
				throw std::runtime_error("Conflicting or duplicate stack and queue specified");
			
			method = method::stack;
			found_method = true;
		}
		else if (cur == "--begin")
		{
			if (i == argc - 1)
				throw std::runtime_error("Didn't specify begin word");
			begin = argv[++i];
		}
		else if (cur == "--end")
		{
			if (i == argc - 1)
				throw std::runtime_error("Didn't specify end word");
			end = argv[++i];
		}
		else if (cur.find("--output") != std::string::npos)
		{
			auto eq_loc = cur.find('=');
			char c;
			if (eq_loc == std::string::npos)
			{
				if (i == argc - 1)
					throw std::runtime_error("Invalid output mode specified");
				
				c = *argv[++i];
			}
			else
			{
				if (eq_loc + 1 >= cur.size())
					throw std::runtime_error("Invalid output mode specified");
				
				c = cur[eq_loc + 1];
			}

			if (c == 'W')
				format = format::word;
			else if (c == 'M')
				format = format::mod;
			else
				throw std::runtime_error("Invalid output mode specified");
		}
		else if (cur == "--change")
			can_do_change = true;
		else if (cur == "--length")
			can_do_length = true;
		else if (cur == "--swap")
			can_do_swap = true;
		else if (cur == "--help")
			output_help();
		else
		{
			if (cur.front() == '-')
			{
				if (cur.size() == 1)
					throw std::runtime_error("Unknown command line option");

				char extension = '\0';

				std::size_t cur_i = 1;
				for (; cur_i < cur.size(); ++cur_i)
				{
					switch (cur[cur_i])
					{
					case 'c':
						can_do_change = true;
						break;
					case 'l':
						can_do_length = true;
						break;
					case 'p':
						can_do_swap = true;
						break;
					case 'q':
						if (found_method)
							throw std::runtime_error("Conflicting or duplicate stack and queue specified");
						method = method::queue;
						found_method = true;
						break;
					case 's':
						if (found_method)
							throw std::runtime_error("Conflicting or duplicate stack and queue specified");

						method = method::stack;
						found_method = true;
						break;
					case 'b':
						if (extension)
							throw std::runtime_error("Unknown command line option");	
						extension = 'b';
						break;
					case 'e':
						if (extension)
							throw std::runtime_error("Unknown command line option");
						extension = 'e';
						break;
					case 'o':
						if (extension)
							throw std::runtime_error("Unknown command line option");
						extension = 'o';
						break;
					case 'h':
						output_help();
					case '=':
						goto after; // double break
					default:
						throw std::runtime_error("Unknown command line option");
					}
				}

				after:

				if (!extension)
					continue;

				std::string next;
				if (++cur_i < cur.size())
					next = cur.substr(cur_i);
				else
				{
					if (i == argc - 1)
						throw std::runtime_error("Unknown command line option");
					next = argv[++i];
				}

				switch (extension)
				{
				case 'b':
					begin = std::move(next);
					break;
				case 'e':
					end = std::move(next);
					break;
				case 'o':
					if (next == "W")
						format = format::word;
					else if (next == "M")
						format = format::mod;
					else
						throw std::runtime_error("Invalid output mode specified");
					break;
				}
			}
			else
				throw std::runtime_error("Unknown command line option");
		}
	}

	if (!found_method)
		throw std::runtime_error("Must specify one of stack or queue");

	if (begin.empty())
		throw std::runtime_error("Beginning word not specified");
	if (end.empty())
		throw std::runtime_error("Ending word not specified");
	if (begin.size() != end.size() && !can_do_length)
		throw std::runtime_error("The first and last words must have the same length when length mode is off");
	if (!can_do_change && !can_do_length && !can_do_swap)
		throw std::runtime_error("Must specify at least one modification mode (change length swap)");
}

std::vector<std::string> get_dictionary()
{
	std::string tmp;
	std::getline(std::cin, tmp);

	std::size_t N;
	std::cin >> N;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

	if (std::cin.fail())
		throw std::runtime_error("Invalid dictionary");

	std::vector<std::string> res;
	res.reserve(N);

	if (tmp == "C") // complex
	{
		std::vector<std::string> new_words;

		while (std::cin >> tmp)
		{
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			if (tmp[0] == '/' && tmp[1] == '/')
				continue;
			
			new_words.clear();
			
			for (std::size_t i = 0; i < tmp.size(); ++i)
			{
				switch (tmp[i])
				{
				case '&': // reversal
					tmp.erase(i);
					new_words.push_back(tmp);
					new_words.emplace_back(tmp.rbegin(), tmp.rend());
					break;
				case '[': // insert each
				{
					std::size_t first = i;
					std::string letters;
					for (++i; i < tmp.size() && tmp[i] != ']'; ++i)
						letters.push_back(tmp[i]);
					
					if (i == tmp.size()) // no closing bracket
						throw std::runtime_error("Invalid word in dictionary");
					
					std::string before = tmp.substr(0, first);
					std::string after = tmp.substr(i + 1);
					for (char c : letters)
						new_words.push_back(before + c + after);
					break;
				}
				case '!': // swap
					if (i < 2) // early swap
						throw std::runtime_error("Invalid word in dictionary");
					
					tmp.erase(i, 1); // erase !
					
					new_words.push_back(tmp); // push back original
					std::swap(tmp[i - 2], tmp[i - 1]);
					new_words.push_back(std::move(tmp)); // push back generated

					break;
				case '?': // double
					if (i == 0) // early double
						throw std::runtime_error("Invalid word in dictionary");
					
					tmp.erase(i, 1); // erase ?
					
					new_words.push_back(tmp); // push back original
					tmp.insert(tmp.begin() + i, tmp[i - 1]);
					new_words.push_back(std::move(tmp));
					break;
				default:
					continue;
				}

				break;
			}

			if (new_words.empty())
				res.push_back(std::move(tmp));
			else
				for (std::string &word : new_words)
					res.push_back(std::move(word));
		}
	}
	else if (tmp == "S") // simple
	{
		while (std::cin >> tmp)
		{
			std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

			if (tmp[0] == '/' && tmp[1] == '/')
				continue;
			
			res.push_back(std::move(tmp));
		}
	}
	else
		throw std::runtime_error("Invalid dictionary");

	return res;
}

struct change_data
{
	std::size_t loc;
	char which;
	char new_letter;
};

struct node
{
	std::optional<change_data> change;
	const std::string *word;
	node *backedge;
	bool visited;
};

void output(const cmd_line &cmd, const node *end)
{
	std::vector<const node *> words;
	while (end)
	{
		words.push_back(end);
		end = end->backedge;
	}

	std::cout << "Words in morph: " << words.size() << '\n';
	if (cmd.format == cmd_line::format::word)
	{
		for (auto it = words.rbegin(); it != words.rend(); ++it)
			std::cout << *(*it)->word << '\n';
	}
	else
	{
		std::cout << cmd.begin << '\n';
		for (auto it = words.rbegin() + 1; it != words.rend(); ++it)
		{
			auto &change = (*it)->change;
			std::cout << change->which << ',' << change->loc;
			if (change->which == 'c' || change->which == 'i')
				std::cout << ',' << change->new_letter;
			std::cout << '\n';
		}
	}
}

bool same_except_i(const std::string &a, const std::string &b, std::size_t loc)
{
	std::size_t i = 0;
	for (; i < loc; ++i)
		if (a[i] != b[i])
			return false;
	for (++i; i < a.size(); ++i)
		if (a[i] != b[i])
			return false;
	
	return true;
}

bool same_swapped_i(const std::string &a, const std::string &b, std::size_t loc)
{
	if (a[loc] == b[loc + 1] && a[loc + 1] == b[loc])
	{
		std::size_t i = 0;
		for (; i < loc; ++i)
			if (a[i] != b[i])
				return false;
		for (i += 2; i < a.size(); ++i)
			if (a[i] != b[i])
				return false;

		return true;
	}
	else
		return false;
}

std::optional<change_data> can_change(const cmd_line &cmd, const std::string &cur, const std::string &desired)
{
	if (cur.size() == desired.size())
	{
		for (std::size_t i = 0; i < cur.size(); ++i)
		{
			if (cmd.can_do_change && same_except_i(cur, desired, i))
				return change_data{i, 'c', desired[i]};
			if (cmd.can_do_swap && i != cur.size() - 1 && same_swapped_i(cur, desired, i))
				return change_data{i, 's', {}};
		}
	}

	if (cmd.can_do_length)
	{
		// do delete
		if (cur.size() > desired.size())
		{
			if (cur.size() - desired.size() > 1)
				return std::nullopt;

			auto cur_it = cur.begin();
			auto desired_it = desired.begin();
			std::size_t diff_loc = std::string::npos;

			while(desired_it < desired.end())
			{
				if (*cur_it == *desired_it)
				{
					++cur_it;
					++desired_it;
				}
				else
				{
					if (diff_loc != std::string::npos)
						return std::nullopt;
					
					diff_loc = cur_it - cur.begin();
					++cur_it;
				}
			}

			if (diff_loc == std::string::npos)
				return change_data{desired.size(), 'd', {}};
			return change_data{diff_loc, 'd', {}};
		}
		// do insert
		else if (cur.size() < desired.size())
		{
			if (desired.size() - cur.size() > 1)
				return std::nullopt;

			auto cur_it = cur.begin();
			auto desired_it = desired.begin();
			std::size_t diff_loc = std::string::npos;

			while(cur_it < cur.end())
			{
				if (*cur_it == *desired_it)
				{
					++cur_it;
					++desired_it;
				}
				else
				{
					if (diff_loc != std::string::npos)
						return std::nullopt;
					
					diff_loc = cur_it - cur.begin();
					++desired_it;
				}
			}

			if (diff_loc == std::string::npos)
				return change_data{cur.size(), 'i', desired[cur.size()]};
			return change_data{diff_loc, 'i', desired[diff_loc]};
		}
	}

	return std::nullopt;
}

void do_queue_search(const cmd_line &cmd, const std::vector<std::string> &dictionary)
{
	std::size_t X = 1;
	std::deque<node *> queue;
	std::vector<node> nodes;
	node *begin{};
	node *end{};
	nodes.reserve(dictionary.size());
	for (const std::string &word : dictionary)
	{
		nodes.push_back({std::nullopt, &word, nullptr, false});
		if (word == cmd.begin)
		{
			if (begin)
				throw std::runtime_error("Beginning word in dictionary twice");
			begin = &nodes.back();
			begin->visited = true;
		}
		else if (word == cmd.end)
		{
			if (end)
				throw std::runtime_error("End word in dictionary twice");
			end = &nodes.back();
		}
	}

	queue.push_front(begin);

	while (!queue.empty())
	{
		node *cur = queue.front();
		queue.pop_front();
		for (node &n : nodes)
		{
			if (n.visited || &n == cur)
				continue;
			
			auto change = can_change(cmd, *cur->word, *n.word);
			if (change)
			{
				++X;
				queue.push_back(&n);
				n.backedge = cur;
				n.visited = true;
				n.change = change;

				if (&n == end)
				{
					output(cmd, &n);
					return;
				}
			}
		}
	}

	std::cout << "No solution, " << X << " words discovered.\n";
}

void do_stack_search(const cmd_line &cmd, const std::vector<std::string> &dictionary)
{
	std::size_t X = 1;
	std::deque<node *> stack;
	std::vector<node> nodes;
	node *begin{};
	node *end{};
	nodes.reserve(dictionary.size());
	for (const std::string &word : dictionary)
	{
		nodes.push_back({std::nullopt, &word, nullptr, false});
		if (word == cmd.begin)
		{
			if (begin)
				throw std::runtime_error("Beginning word in dictionary twice");
			begin = &nodes.back();
			begin->visited = true;
		}
		else if (word == cmd.end)
		{
			if (end)
				throw std::runtime_error("End word in dictionary twice");
			end = &nodes.back();
		}
	}

	stack.push_back(begin);

	while (!stack.empty())
	{
		node *cur = stack.back();
		stack.pop_back();
		for (node &n : nodes)
		{
			if (n.visited || &n == cur)
				continue;
			
			auto change = can_change(cmd, *cur->word, *n.word);
			if (change)
			{
				++X;
				stack.push_back(&n);
				n.backedge = cur;
				n.visited = true;
				n.change = change;

				if (&n == end)
				{
					output(cmd, &n);
					return;
				}
			}
		}
	}

	std::cout << "No solution, " << X << " words discovered.\n";
}