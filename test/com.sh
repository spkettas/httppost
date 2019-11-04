#!/bin/sh


g++ -g -o producer producer.cpp -std=c++11 -I/usr/local/kafka/include \
	-I/usr/local/kafka/include/librdkafka \
	-L/usr/local/kafka/lib -lrdkafka -lrdkafka++

g++ -g -o consumer consumer.cpp -std=c++11 -I/usr/local/kafka/include \
	-I/usr/local/kafka/include/librdkafka \
	-L/usr/local/kafka/lib -lrdkafka -lrdkafka++ \
	-lz -lpthread -lrt -lssl
