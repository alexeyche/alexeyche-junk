
#include <dnn/base/base.h>

namespace NDnn {

	template <typename N, ui32 size>
	class TLayer {
	public:
		TLayer()
			: Id(0)
		{}

		ui32 Size() const {
			return size;
		}
		
		N& operator[](ui32 id) {
			return Neurons[id];
		}

		void SetId(ui32 id) {
			Id = id;
		}

		const ui32& GetId() const {
			return Id;
		}
		
	private:
		ui32 Id;
		std::array<N, size> Neurons;
	};



} // namespace NDnn