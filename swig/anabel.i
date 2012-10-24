%module Anabel
%{
#include "anabel.h"
#include "stdint.h"
%}

%include "anabel.h"
%include "timeseries.h"
%include "exceptions.h"
%include "readquery.h"
%include "prettyifier.h"

%exception {
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


namespace Anabel {
    %template(RecordInt) Record<signed long int>;
    %template(RecordShort) Record<signed int>;
    %template(RecordChar) Record<signed char>;
    %template(RecordFloat) Record<float>;

	%template(WrapperInt) DataWrapper<int>;
	%template(WrapperShort) DataWrapper<short>;
	%template(WrapperChar) DataWrapper<char>;
	%template(WrapperFloat) DataWrapper<float>;
}
