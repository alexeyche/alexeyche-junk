#pragma once


template <typename T>
T cast(string &s) {
	cerr << "Can't cast " << s << "\n";
	terminate();
}

template <>
double cast(string &s) {
	return std::stof(s);
}	

template <>
int cast(string &s) {
	return std::stoi(s);
}	

template <>
size_t cast(string &s) {
	return std::stoi(s);
}
template <>
string cast(string &s) {
	return string(s);
}

class OptionParser {
public:
	OptionParser(int argc, char **argv) {
		for(size_t i=1; i<argc; ++i) {
			opts.push_back(argv[i]);
		}
	}
	template <typename T>
	void option(string long_opt, string short_opt, T &src, bool required = true, bool as_flag=false) {
		bool found = false;
		auto it = opts.begin();
		while(it != opts.end()) {
			if((*it == long_opt)||(*it == short_opt)) {
				if(as_flag) { 
					src = true;
					it = opts.erase(it);
				} else {
					if( (it+1) == opts.end() ) {
						cerr << "Can't find value for option " << long_opt << "\n";
						terminate();
					}
					src = cast<T>(*(++it));
					it = opts.erase(it-1, it+1);
				}
				found = true;
			} else {
				++it;
			}
		}
		if((!found)&&(required)) {
			cerr << "Can't find value for option " << long_opt << "\n";
		}

	}
	vector<string>& getRawOptions() {
		return opts;
	}
private:

	vector<string> opts;
};