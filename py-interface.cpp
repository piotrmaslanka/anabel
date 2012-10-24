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
#include <anabel/py-interface.h>

void * Anabel::allocate_buffer(unsigned record_size, unsigned amount) { return malloc((record_size+8)*amount); }
void Anabel::deallocate_buffer(void * buffer) { free(buffer); }

void Anabel::extract_timestamps(unsigned long long * outvec, unsigned amount, void * buffer, unsigned record_size) {
	Anabel::Timestamp * pval = (Anabel::Timestamp*)buffer;
		for (unsigned i=0; i<amount; i++) {
			outvec[i] = *pval;
			pval = (Anabel::Timestamp*)(((char*)pval) + record_size);
		}
}