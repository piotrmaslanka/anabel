%module Anabel
%{
#include "anabel.h"
#include "stdint.h"
#include "py-interface.h"
%}

%include "anabel.h"
%include "timeseries.h"
%include "exceptions.h"
%include "readquery.h"

/*%exception {
   try {
   $action
   } catch (Anabel::Exceptions::InvalidRootDirectory& e) {
        PyErr_SetString(PyExc_EnvironmentError, e.reason.c_str());
        return NULL;
   } catch (Anabel::Exceptions::InvalidInvocation& e) {
        PyErr_SetString(PyExc_AttributeError, e.reason.c_str());
        return NULL;
   }
}
*/
/* numpy part */
%{
#define SWIG_FILE_WITH_INIT
%}
%include "numpy.i"
%init %{
import_array();
%}

%apply (unsigned long long* ARGOUT_ARRAY1, int DIM1) {(unsigned long long * outvec, unsigned amount)}
%apply (int* ARGOUT_ARRAY1, int DIM1) {(int * outvec, unsigned amount)}
%apply (float* ARGOUT_ARRAY1, int DIM1) {(float * outvec, unsigned amount)}

%include "py-interface.h"

namespace Anabel {
        %template (extract_int64) extract_value<long long, long long>;
        %template (extract_int32) extract_value<int, int>;
        %template (extract_float) extract_value<float, float>;
        %template (extract_char) extract_value<char, char>;

        %template (compose_int64) compose<long long>;
        %template (compose_int32) compose<int>;
        %template (compose_float) compose<float>;
        %template (compose_char) compose<char>;
}
