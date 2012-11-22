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
#include <anabel/stdafx.h>
#include <anabel/anabel.h>

void Anabel::ReadQuery::set_desired_cache_size(size_t elements) { this->desired_cache_size = elements; }

size_t Anabel::ReadQuery::get_cache_size() { return this->desired_cache_size; }

Anabel::BigDataBlock::BigDataBlock() : buffer(NULL), buffer_length(0), entries_readed(0) {}
Anabel::BigDataBlock::~BigDataBlock() { if (this->buffer != NULL) free(this->buffer); }
void Anabel::BigDataBlock::invalidate() {
	if (this->buffer != NULL) {
		free(this->buffer);
		this->buffer = NULL;
		this->entries_readed = 0; 
		this->buffer_length = 0; 
	} 
}

Anabel::BigDataBlock * Anabel::ReadQuery::read_everything() {
	BigDataBlock * bb = new BigDataBlock();
	const unsigned cache_buf_size = (8+this->record_size)*this->desired_cache_size;

	void * chunk_buffer = malloc(cache_buf_size);

	while (true) {
		size_t records_readed = this->get_data(this->desired_cache_size, chunk_buffer);
		if (records_readed == 0) break;
		bb->entries_readed += records_readed;
		size_t mem_for_records = (8+this->record_size)*records_readed;
		bb->buffer_length += mem_for_records;
		bb->buffer = realloc(bb->buffer, bb->buffer_length);
		memcpy((void*)((char*)(bb->buffer) + bb->buffer_length - mem_for_records), chunk_buffer, mem_for_records);
	}
	free(chunk_buffer);
	return bb;	
}


void Anabel::ReadQuery::prime_cache(void) {
	if (this->data_cache != NULL) free(this->data_cache);
	this->data_cache = malloc((8 + this->record_size) * this->desired_cache_size);
	this->available_cache_entries = this->ll_get_data(this->desired_cache_size, this->data_cache);
	if (this->available_cache_entries == 0) {
		free(this->data_cache);
		this->data_cache = NULL;
		return;
	}
	this->data_cache = realloc(this->data_cache, (8 + this->record_size) * this->available_cache_entries);
	this->cache_ofs = this->data_cache;
}

size_t Anabel::ReadQuery::get_data(size_t count, void * buffer) {
	if (this->available_cache_entries == 0) {
		this->prime_cache();
		if (this->available_cache_entries == 0) return 0;
	}
	if (count > this->available_cache_entries) count = this->available_cache_entries;
	size_t bytes_to_copy = count * (8+this->record_size);
	memcpy(buffer, this->cache_ofs, bytes_to_copy);
	this->cache_ofs = ((char*)(this->cache_ofs)) + bytes_to_copy;
	this->available_cache_entries -= count;
	return count;
}

size_t Anabel::ReadQuery::ll_get_data(size_t count, void * buffer) {
	if (this->opened_file != NULL)
		if (this->opened_file->records_remaining == 0) {
			delete this->opened_file;
			this->opened_file = NULL;
		}

	if (this->opened_file == NULL) {
	grab_next_file:
		if (this->diter->empty) return 0;		// no more data can be output
		this->opened_file = new Anabel::Internal::IntelligentFileReader(this->diter->next(), this->record_size);
		if (this->opened_file->records_remaining == 0) {
				// whacked or empty file
			delete this->opened_file;
			this->opened_file = NULL;
			goto grab_next_file;
		}
		if (this->first_readed) {
			this->first_readed = false;
			this->opened_file->limit_start(this->from);
		}
		if (this->diter->empty) this->opened_file->limit_end(this->to);
		this->opened_file->prepare_read();
	}

	size_t records_readed = this->opened_file->get_data(count, buffer);

	return records_readed;
}


Anabel::ReadQuery::ReadQuery(Timestamp from, Timestamp to, std::vector<boost::filesystem::path> files, int record_size) : from(from), to(to), 
																														  record_size(record_size),
																														  first_readed(true),
																														  opened_file(NULL), data_cache(NULL),																																
																														  desired_cache_size(20000),
																														  available_cache_entries(0)

{
	this->diter = new Anabel::Internal::DirectoryIterator(files);
}
Anabel::ReadQuery::~ReadQuery() {
	if (this->data_cache != NULL) free(this->data_cache);
	if (this->opened_file != NULL) free(this->opened_file);
	delete this->diter;
}