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

	template<class T>
	class DataWrapper {
		private:
			unsigned allocated_size;
		public:
			void * data_buffer;
			unsigned entries_readed;
			DataWrapper(unsigned buffer_size): entries_readed(0), allocated_size(buffer_size) {
				this->data_buffer = malloc(buffer_size*(8+sizeof(T)));
			}

			DataWrapper() : entries_readed(0), allocated_size(20000) {
				this->data_buffer = malloc(20000*(8+sizeof(T)));
			}
			~DataWrapper() { free(this->data_buffer); }
			

			T read_value(unsigned id) throw(Anabel::Exceptions::InvalidInvocation) {
				if (id >= this->entries_readed) throw Anabel::Exceptions::InvalidInvocation("No such entry");
				T * tt = (T*)(((char*)this->data_buffer) + id*(8+sizeof(T)) + 8);
				return *tt;
			}
			Anabel::Timestamp read_timestamp(unsigned id) throw(Anabel::Exceptions::InvalidInvocation) {
				if (id >= this->entries_readed) throw Anabel::Exceptions::InvalidInvocation("No such entry");
				Anabel::Timestamp * tt = (Anabel::Timestamp*)(((char*)this->data_buffer) + id*(8+sizeof(T)));
				return *tt;
			}
	};
};