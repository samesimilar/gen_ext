// Basic support for referencing Pd Arrays from gen~.

#include "gen_ext_common.h"

#include "genlib.h"
#include "genlib_exportfunctions.h"
#include "genlib_ops.h"

#include <m_pd.h>

typedef struct PdBuffer : public DataInterface<t_sample> {
	
	PdBuffer() : DataInterface<t_sample>() {}

	int setarray(t_garray *a) {
		
		
		int size;
		
		
		if (!a) {
			channels = 0;
			dim = 0;
			mData = 0;
			return 1;
		}
		if (!garray_getfloatarray(a, &size, &mData)) {
			channels = 0;
			dim = 0;
			mData = 0;
			return 2;
		}
		channels = 1;
		dim = size;
		garray_usedindsp(a);
		return 0;
		
	}
	
} PdBuffer;