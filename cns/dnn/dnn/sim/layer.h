
#include <dnn/base/base.h>

namespace NDnn {

	template <typename N, ui32 size>
	class TLayer {
	public:

		ui32 Size() const {
			return size;
		}
		
		N& operator[](ui32 id) {
			return Neurons[id];
		}

	private:
		std::array<N, size> Neurons;
	};



} // namespace NDnn