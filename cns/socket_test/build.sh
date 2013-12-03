set -x
rm -rf ./*.o
g++ ./test_sock_main_serv.cpp -lpthread -O2 -o test_s
