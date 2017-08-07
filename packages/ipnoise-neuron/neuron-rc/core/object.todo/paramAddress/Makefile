URL=http://www.gnivc.ru/html/gnivcsoft/KLADR/BASE.7z
TMP_FILE=BASE.7z
TMP_DIR=tmp
DB_FILE=kladr.db

.PHONY: default download unpack convert_to_utf8 dbf2sql crete_sqllite_db tests clean

default: tests
#default: tests crete_sqllite_db

download:
	wget $(URL) -O$(TMP_FILE)

unpack: download
	rm -rf $(TMP_DIR)
	mkdir $(TMP_DIR)
	7za x -o$(TMP_DIR) $(TMP_FILE)

dbf2sql: unpack
	@for i in $(TMP_DIR)/*.DBF; do							\
		echo "convert $$i to $$i.sql";						\
		sqlite3-dbf $$i | iconv -f cp866 -t utf8 > $$i.sql;	\
	done

crete_sqllite_db:	dbf2sql
	@for i in $(TMP_DIR)/*.sql; do							\
		echo "import $$i" in $(DB_FILE);					\
		sqlite3 $(DB_FILE) < $$i;							\
	done

tests:
	$(MAKE) -C tests

clean:
	rm -rf $(TMP_DIR) $(TMP_FILE)
	$(MAKE) -C tests clean

