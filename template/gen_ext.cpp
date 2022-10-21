#include "gen_ext_common.h"

#include <m_pd.h>
#include "_ext.h"

namespace WRAPPER_NAMESPACE {

using namespace GEN_EXPORTED_NAME;

static t_class *WRAPPER_CLASS;

typedef struct WRAPPER_STRUCT {
  t_object  x_obj;
  
  float x_sr;
  int x_bs;
  
  t_float f;
  
  CommonState* m_genObject;
  
  int x_num_inputs;
  int x_num_outputs;
  
  t_symbol **x_param_symbols;
  int x_num_params;
  
} WRAPPER_TYPE;

static void *WRAPPER_NEW(void)
{
  WRAPPER_TYPE *x = (WRAPPER_TYPE *)pd_new(WRAPPER_CLASS);
  
  
  x->x_sr = sys_getsr();
  x->x_bs = sys_getblksize();
  
  x->m_genObject = (CommonState*)create(x->x_sr, x->x_bs);
  
  x->x_num_inputs = num_inputs();
  x->x_num_outputs = num_outputs();
  
  x->x_num_params = num_params();
  if (x->x_num_params > 0) {
	  x->x_param_symbols = ( t_symbol ** ) getbytes(sizeof(t_symbol *) * x->x_num_params);
	  int i = 0;
	  for(i = 0; i < x->x_num_params; i++)
	  {
		  x->x_param_symbols[i] = gensym(getparametername(x->m_genObject, i));
	  }
  }

  
  
  int ni = x->x_num_inputs - 1; // first input is created in _create
  if (ni > 0) {
	  while(ni--){
	  	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	  }
  }
  
  int no = x->x_num_outputs;
  while(no--) {
  	outlet_new(&x->x_obj, &s_signal);
  }

  
  return (void *)x;
}
/**
NOTE: we do not really need to free inlets and outlet. As Pd will automatically free them for us (unless we are doing higher-order magic, like displaying one object's iolet as another object's. but let's not get into that for now...)
**/

static void WRAPPER_FREE(WRAPPER_TYPE *x) {
	if (x->m_genObject) {destroy(x->m_genObject);}
	if (x->x_num_params > 0) {
		freebytes(x->x_param_symbols, sizeof(t_symbol *) * x->x_num_params);
	}
}

/**
Optimisation of the DSP-tree tries to avoid unnecessary copy-operations. 
Therefore it is possible, that in- and out-signal are located at the same
 address in the memory. In this case, the programmer has to be careful
 not to write into the out-signal before having read the in-signal to avoid 
overwriting data that is not yet saved.
**/
static t_int *WRAPPER_PERFORM(t_int *w)
{
	WRAPPER_TYPE *x = (WRAPPER_TYPE *)(w[1]);
	
	int inputIndex = 2;
	int outputIndex = inputIndex + x->x_num_inputs;
	int sampleCountIndex = outputIndex + x->x_num_outputs;
	
	
	int n= (int)(w[sampleCountIndex]);
	
	perform(x->m_genObject, (t_sample **)(&w[inputIndex]), x->x_num_inputs, (t_sample **)(&w[outputIndex]), x->x_num_outputs, n);
	
	return (w+sampleCountIndex+1);
	
}
static void WRAPPER_DSP(WRAPPER_TYPE *x, t_signal **sp)
  {
  	int i;
	int inOutCount = x->x_num_inputs + x->x_num_outputs;
  	int pointerCount = inOutCount + 2;
  	t_int **sigvec = (t_int **)getbytes(sizeof(t_int)*(pointerCount));
	
	sigvec[0] = (t_int*)x;
	for (i = 0; i < inOutCount; i++) {
		sigvec[1 + i] = (t_int*)sp[i]->s_vec;
	}
	sigvec[1 + inOutCount] = (t_int*)sp[0]->s_n;
	dsp_addv(WRAPPER_PERFORM, pointerCount, (t_int*)sigvec);
	
	freebytes(sigvec, sizeof(t_int)*(pointerCount));


}


static void WRAPPER_ANY_METHOD(WRAPPER_TYPE *x, t_symbol *s, int argc, t_atom *argv) {
	int i;
	// lookup method in symbol list and set parameter
	for (i = 0; i < x->x_num_params; i++) {
		if (s == x->x_param_symbols[i]) {
			if (argc > 0) {
				t_float f1 = atom_getfloatarg(0, argc, argv);
				setparameter(x->m_genObject, i, f1, 0);
				
				// post(getparametername(x->m_genObject, i));
			}
			break;
		}
	}
}

static void WRAPPER_BANG(WRAPPER_TYPE *x) {
	
    post("gen-ext wrapper v%s", STR(GEN_EXT_VERSION));
    post("%s~ samplerate: %g, blocksize: %d", STR(PD_EXT_NAME), x->x_sr, x->x_bs);
	post("num_audio_rate_inputs: %d", x->x_num_inputs);
	post("num_audio_rate_outputs: %d", x->x_num_outputs);
	post("num_params: %d", x->x_num_params);
	int i;
	for (i = 0; i < x->x_num_params; i++) {
		const char * name = getparametername(x->m_genObject, i);
		const char * units = getparameterunits(x->m_genObject, i);
		char hasMinMax = getparameterhasminmax(x->m_genObject, i);
		if (hasMinMax) {
			t_param minp = getparametermin(x->m_genObject, i);
			t_param maxp = getparametermax(x->m_genObject, i);
			if (units) {
				post("param: %s, min_float: %g, max_float %g, units: %s", name, minp, maxp, units);
			} else {
				post("param: %s, min_float: %g, max_float %g", name, minp, maxp);
			}
			
		} else {
			if (units) {
				post("param: %s, units: %s", name, units);
			} else {
				post("param: %s", name);
			}
			
		}
	}
	
}

extern "C" void WRAPPER_SETUP (void) {	

   WRAPPER_CLASS = class_new(gensym(STR(PD_EXT_NAME) "~"),
         (t_newmethod)WRAPPER_NEW,
         (t_method)WRAPPER_FREE, sizeof(WRAPPER_TYPE),
         CLASS_DEFAULT, A_NULL);
		 
  class_addbang(WRAPPER_CLASS, WRAPPER_BANG);
  
  class_addmethod(WRAPPER_CLASS,
            (t_method)WRAPPER_DSP, gensym("dsp"), A_CANT, 0);

			
	class_addanything(WRAPPER_CLASS, (t_method)WRAPPER_ANY_METHOD);
	
	if (num_inputs() > 0) {
		CLASS_MAINSIGNALIN(WRAPPER_CLASS, WRAPPER_TYPE, f);
	}
	
}
}