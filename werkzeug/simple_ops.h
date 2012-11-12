#include <anabel/anabel.h>
#include <string>

int indent(char * db_to_indent);
int view(char * db_path, Anabel::Timestamp t_from, Anabel::Timestamp t_to, char * commontype);
int append(char * db_path, Anabel::Timestamp timestamp, std::string value, char * commontype);
int create(char * db_path, int record_size);
int truncate(char * db_path);