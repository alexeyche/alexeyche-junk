#include "global_ctx.h"

namespace NDnn {

	
	TGlobalCtx& TGlobalCtx::Inst() {
		static TGlobalCtx inst;
		return inst;
	}
	


} // namespace NDnn