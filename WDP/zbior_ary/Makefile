CFLAGS = -std=c17 -pedantic -Wall -Wextra -Wformat-security -Wduplicated-cond\
         -Wfloat-equal -Wshadow -Wconversion -Wjump-misses-init\
         -Wlogical-not-parentheses -Wnull-dereference -Wvla -Werror\
         -fstack-protector-strong -fsanitize=undefined -fno-sanitize-recover\
         -g -fno-omit-frame-pointer -O1

CPPFLAGS = -std=c++20 -pedantic -Wall -Wextra -Wformat-security -Wduplicated-cond\
         -Wfloat-equal -Wshadow -Wconversion\
         -Wlogical-not-parentheses -Wnull-dereference -Wvla -Werror\
         -fstack-protector-strong -fsanitize=undefined -fno-sanitize-recover\
         -g -fno-omit-frame-pointer -O3

tester.e: tester.cpp zbior_ary.o
	g++ $(CPPFLAGS) tester.cpp zbior_ary.o -o tester.e

zbior_ary.o: zbior_ary.c
	gcc $(CFLAGS) -c zbior_ary.c

clean:
	rm -f *.e *.o