#include "string.h"

#include <dnn/util/log/log.h>

#include <iostream>

namespace NDnn {
    namespace NStr {

        TString CamelCaseToOption(TString s) {
            std::regex e("([a-z])([A-Z])");
            std::string res = std::regex_replace(s, e, "$1-$2");
            std::transform(res.begin(), res.end(), res.begin(), ::tolower);
            return std::string("--") + res;
        }

        void Replace(TString &s, const TString &search, const TString &replace, size_t num) {
            size_t repl_num = 0;
            for( size_t pos = 0; ; pos += replace.length() ) {
                if(repl_num>=num) break;
                // Locate the subTString to replace
                pos = s.find( search, pos );
                if( pos == TString::npos ) break;
                // Replace by erasing and inserting
                s.erase( pos, search.length() );
                s.insert( pos, replace );
                repl_num+=1;
            }
        }


        TString Trim(const TString& strInp, TString symbols) {
            size_t startpos = 0;
            size_t endpos = strInp.size();
            bool meatFound = false;
            for (auto iter = strInp.begin(); iter != strInp.end(); ++iter) {
                if (symbols.find_first_of(*iter) != TString::npos) {
                    if (!meatFound) {
                        startpos++;
                    } else {
                        endpos--;
                    }
                } else {
                    meatFound = true;
                    endpos = strInp.size();
                }
            }
            return strInp.substr(startpos, endpos);
        }


        TString Strip(const TString& input) {
           size_t b = input.find_first_not_of(' ');
           if (b == TString::npos) b = 0;
           return input.substr(b, input.find_last_not_of(' ') + 1 - b);
        }

        TString LStrip(const TString& input, TString symbols) {
            size_t b = input.find_first_not_of(symbols);
            if (b != TString::npos) {
                return input.substr(b, TString::npos);
            } else {
                return input;
            }
        }

        TVector<TString> SplitInto(const TString &s, char delim, TVector<TString> &elems, ui32 numberOfSplits) {
            std::stringstream ss(s);
            TString item;

            ui32 iter = 0;
            while (std::getline(ss, item, delim)) {
                elems.push_back(item);
                iter++;
                if (iter >= numberOfSplits) {
                    TString rest;
                    while (std::getline(ss, item)) {
                        rest += item;
                    }
                    elems.push_back(rest);
                    break;
                }
            }
            return elems;
        }


        TVector<TString> Split(const TString &s, char delim, ui32 numberOfSplits) {
            TVector<TString> elems;
            SplitInto(s, delim, elems, numberOfSplits);
            return elems;
        }

        TVector<TString> Split(const TString &s_inp, const TString &delimiter, const TString &not_include) {
            TString s(s_inp);
            TVector<TString> out;
            if(s_inp.empty()) {
                return out;
            }

            size_t pos = 0;
            std::string acc_token;
            std::string token;
            while ((pos = s.find(delimiter)) != std::string::npos) {
                token = s.substr(0, pos);
                // cout << "token: " << token << "\n";
                if((!not_include.empty()) && (token.find_first_of(not_include) != std::string::npos)) {
                    if(!acc_token.empty()) {
                        acc_token += delimiter;
                    }
                    acc_token += token;
                    // cout << token << " " << acc_token << " 1\n";
                } else {
                    if(!acc_token.empty()) {
                        // cout << token << " " << acc_token << " 2\n";
                        token = acc_token + delimiter + token;
                        acc_token = std::string();
                    }
                    out.push_back(Trim(token));
                }
                s.erase(0, pos + delimiter.length());
            }
            if(!acc_token.empty()) {
                // cout << s << " " << acc_token << " 3\n";
                s = acc_token + delimiter + s;
            }
            out.push_back(Trim(s));
            return out;
        }

        bool EndsWith(const TString &str, const TString&& suffix) {
            if (str.length() >= suffix.length()) {
               return (0 == str.compare (str.length() - suffix.length(), suffix.length(), suffix));
            } else {
                return false;
            }
        }

        template <>
        ui32 As<ui32>(const TString& s) {
            return std::stoi(s);
        }

        template <>
        int As<int>(const TString& s) {
            return std::stoi(s);
        }

        template <>
        double As<double>(const TString& s) {
            return std::stof(s);
        }

        template <>
        TString As<TString>(const TString& s) {
            return s;
        }
    } // namespace NStr
} // namespace NDnn