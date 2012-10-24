/*
    This file is part of Anabel

    Anabel is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    Anabel is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Anabel; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/
#include <anabel/anabel.h>

namespace Anabel {
#pragma pack(push)
#pragma pack(1)
	template<class T>
	struct Record {
		Anabel::Timestamp timestamp;
		T value;
	};
#pragma pack(pop)

	void * allocate_buffer(unsigned record_size, unsigned amount);
	void deallocate_buffer(void * buffer);
	void extract_timestamps(unsigned long long * outvec, unsigned amount, void * buffer, unsigned record_size);

	template <class InternalType, class ExtractionType> void extract_value(ExtractionType * outvec, unsigned amount, void * buffer) {
		InternalType * pval = (InternalType*)(((char*)buffer) + 8);
		for (unsigned i=0; i<amount; i++) {
			outvec[i] = (ExtractionType)(*pval);
			pval = (InternalType*)(((char*)pval) + 8 + sizeof(InternalType));
		}
	}
};