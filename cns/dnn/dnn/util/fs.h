#pragma once

#include <dnn/base/base.h>

namespace NDnn {

	bool FileExists(const TString& name);

    TVector<TString> ListDir(TString path);

    class TFsPath {
    public:
        static const TString Sep;

        TFsPath(TString path);

        TFsPath operator/(const TFsPath p);

        operator TString() const;

        friend std::ostream& operator<<(std::ostream& stream, const TFsPath& path);

        bool IsFile() const;

        TString AsString() const;

        TFsPath& operator+(const char *suffix);

    private:
        TString Path;
    };

} // namespace NDnn