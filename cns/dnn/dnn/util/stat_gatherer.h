#pragma once

#include <dnn/base/base.h>
#include <dnn/util/serial/proto_serial.h>
#include <dnn/util/ptr.h>
#include <dnn/protos/stat_gatherer.pb.h>

namespace NDnn {

	struct TStatistics: public IProtoSerial<NDnnProto::TStatistics> { 
		TStatistics() {}

		TStatistics(const TString& name, ui32 from, ui32 to)
			: Name(name)
			, From(from)
			, To(to)
		{}

		void SerialProcess(TProtoSerial& serial) override {
            serial(Values);
            serial(From);
            serial(To);
            serial(Name);
        }

        TVector<double> Values;
        ui32 From;
        ui32 To;
        TString Name;
	};

	class TStatCollector {
	public:
		TStatCollector(const TString& name, const double& v, ui32 from, ui32 to)
			: Src(&v)
			, Result(name, from, to) 
		{}
		
		TStatCollector(const TStatCollector& other) {
			(*this) = other;	
		}

		TStatCollector& operator=(const TStatCollector& other) {
			if (this != &other) {
				Result = other.Result;
				throw TDnnException() << "Can't gather statistics from destroyed objects. Please do not copy after you've pointed variables to listen to";
				Src = other.Src;
			}
			return *this;
		}

		void Collect() {
			Result.Values.push_back(Src.Ref());
		}
		
		const ui32& GetFrom() const {
			return Result.From;
		}

		const ui32& GetTo() const {
			return Result.To;
		}

		TStatistics& GetMutStatistics() {
			return Result;
		}

	private:
		TStatistics Result;
		TPtr<const double> Src;
	};

	class TStatGatherer {
	public:
		TStatGatherer();

		TStatGatherer(const TStatGatherer& other);

		TStatGatherer& operator=(const TStatGatherer& other);

		void ListenStat(const TString& name, const double& v, ui32 from, ui32 to);

		void Collect(const TTime& t);

		void SaveStat(const TString& fname);

	private:
		TVector<TPtr<TStatCollector>> ActiveStats;
		TVector<TStatCollector> Stats;
	};


} // namespace NDnn