#include <vector>
#include <string>

#include <villas/utils.hpp>

namespace villas {
namespace utils {

std::vector<std::string>
tokenize(std::string s, std::string delimiter)
{
	std::vector<std::string> tokens;

	size_t lastPos = 0;
	size_t curentPos;

	while((curentPos = s.find(delimiter, lastPos)) != std::string::npos) {
		const size_t tokenLength = curentPos - lastPos;
		tokens.push_back(s.substr(lastPos, tokenLength));

		// advance in string
		lastPos = curentPos + delimiter.length();
	}

	// check if there's a last token behind the last delimiter
	if(lastPos != s.length()) {
		const size_t lastTokenLength = s.length() - lastPos;
		tokens.push_back(s.substr(lastPos, lastTokenLength));
	}

	return tokens;
}

std::string
join(std::vector<std::string> strings, std::string delimiter)
{
	std::string out;

	for(size_t i = 0; i < strings.size(); i++) {
		const auto& s = strings[i];

		if(s.length() > 0) {
			out += strings[i];

			if(i < (strings.size() - 1) and strings[i+1].length() > 0)
				out += delimiter;
		}
	}

	return out;
}

} // namespace utils
} // namespace villas
