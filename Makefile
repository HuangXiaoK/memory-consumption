all:
	gcc -O3  -mcmodel=medium mem_cons_type_one.c -o mem_cons_type_one -lnuma -lpthread -lrt
	gcc -O3  -mcmodel=medium mem_cons_type_two.c -o mem_cons_type_two -lnuma -lpthread -lrt

clean:
	rm ./mem_cons_type_one
	rm ./mem_cons_type_two