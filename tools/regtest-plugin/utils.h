#pragma once

// via https://stackoverflow.com/questions/6089231/
std::string read_line(std::istream &is) {
	std::string out;

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for(;;) {
		int c = sb->sbumpc();
		switch (c) {
			case '\n':
			return out;
		case '\r':
			if (sb->sgetc() == '\n') {
				sb->sbumpc();
			}
			return out;
		case EOF:
			// Also handle the case when the last line has no line ending
			if (out.empty()) {
				is.setstate(std::ios::eofbit);
			}
			return out;
		default:
			out += (char)c;
		}
	}
}

bool starts_with(const std::string& str, const std::string& start) {
	return str.length() >= start.length() &&
		   0 == str.compare(0, start.length(), start);
}

bool ends_with(const std::string& str, const std::string& end) {
	return str.length() >= end.length() &&
		0 == str.compare(str.length() - end.length(), end.length(), end);
}

std::vector<std::string> tokenize(const std::string &str_to_tokenize) {
	std::vector<std::string> tokens;
	std::string cur_token;

	for (char c : str_to_tokenize) {
		if (c == ',') {
			tokens.push_back(cur_token);
			cur_token.clear();
			continue;
		}

		cur_token.push_back(c);
	}

	tokens.push_back(cur_token);

	return tokens;
}

// converts to local codepage, not utf-8!
std::string wchar_to_char(const wchar_t* inwchar) {
	char outbuf[255];
	WideCharToMultiByte(CP_ACP, 0, inwchar, -1, outbuf, sizeof(outbuf), NULL, NULL);
	return std::string(outbuf);
}
