#pragma once

#include <dnn/base/base.h>

#include <regex>
#include <iterator>
#include <limits>

namespace NDnn {
	namespace NStr {

	    TString CamelCaseToOption(TString s);

	    void Replace(TString &s, const TString &search, const TString &replace, size_t num = std::numeric_limits<size_t>::max());

	    TString Trim(const TString &strInp, TString symbols = " \t\n\r");

	    TString LStrip(const TString& input, TString symbols = " \t\n\r");

	    TString Strip(const TString& input);

		TVector<TString> SplitInto(const TString &s, char delim, TVector<TString> &elems, ui32 numberOfSplits = std::numeric_limits<ui32>::max());

		TVector<TString> Split(const TString &s, char delim, ui32 numberOfSplits = std::numeric_limits<ui32>::max());

	    TVector<TString> Split(const TString &s_inp, const TString &delimiter, const TString &not_include = "");


	    class TStringBuilder {
	    public:
	        template <typename T>
	        TStringBuilder& operator << (const T& s) {
	            ss << s;
	            return *this;
	        }

	        operator TString () {
	        	return ss.str();
	        }

		private:
	        stringstream ss;
	    };

	    bool EndsWith(const TString &str, const TString&& suffix);

	    template <typename T>
	    T As(const TString& s);
	    
	    template <typename T>
	    TString ToString(T v) {
	    	return std::to_string(v);
	    }

	} // namespace NStr
} // namespace NDnn