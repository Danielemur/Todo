CFLAGS = -g3 -std=c11

all : todo

todo : todo.c database.o common.o csv.o event.o date.o stredit.o termanip.o
	$(CC) $(CFLAGS) $^ -o $@

common.o : common.c common.h
	$(CC) $(CFLAGS) -c $<

csv.o : csv.c csv.h common.h
	$(CC) $(CFLAGS) -c $<

database.o : database.c database.h common.h csv.h event.h
	$(CC) $(CFLAGS) -c $<

date.o : date.c date.h common.h
	$(CC) $(CFLAGS) -c $<

event.o : event.c event.h common.h date.h
	$(CC) $(CFLAGS) -c $<

stredit.o : stredit.c stredit.h termanip.h
	$(CC) $(CFLAGS) -c $<

termanip.o : termanip.c termanip.h
	$(CC) $(CFLAGS) -c $<

clean :
	rm -f test *.o
