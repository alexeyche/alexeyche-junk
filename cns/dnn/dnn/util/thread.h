#pragma once

#include <dnn/base/base.h>

namespace NDnn {

	struct TIndexSlice {
	    TIndexSlice(ui32 from, ui32 to) : From(from), To(to), Size(To-From) {}

	    ui32 From;
	    ui32 To;
	    ui32 Size;
	};


	TVector<TIndexSlice> DispatchOnThreads(ui32 elements_size, ui32 jobs);


} // namespace NDnn