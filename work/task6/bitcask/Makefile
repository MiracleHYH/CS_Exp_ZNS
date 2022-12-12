all:
	@ g++ -std=c++11 main.cpp bitcask.cpp -o main -lboost_serialization -lboost_date_time

run: all
	@ ./main
	@ rm -rf main 

run_once: run clean

clean:
	@ rm -rf ./db/*
	@ rm -rf main