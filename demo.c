/*
 * main.c
 * Copyright (C) 2018 tox <tox@rootkit>
 *
 * Distributed under terms of the MIT license.
 */

#include "pb.h"
#include <unistd.h>
#include <pthread.h>

static void *
do_load(void *arg) {
	int id = 0, i;

	pb(&id, -1, "init loading");
	usleep(50000);
	for(i = 0; i < 100; i+=5) {
		pb(&id, i, "loading");
		usleep(500000);
	}
	pb(&id, -1, "done");
	return NULL;
}

static void *
do_calculate(void *arg) {
	int id = 0, i;

	pb(&id, -1, "init calculating");
	usleep(1000000);
	for(i = 0; i < 100; i+=5) {
		pb(&id, i, "loading");
		usleep(50000);
	}
	pb(&id, -1, "done");
	return NULL;
}

static void *
do_hack(void *arg) {
	int id = 0, i;

	pb(&id, -1, "init hacking");
	usleep(100000);
	for(i = 0; i < 100; i+=5) {
		pb(&id, i, "hacking");
		if(i && i % 30 == 0)
			pb(0, -1, "HACKING TOO MUCH RAM!");
		usleep(1000000);
	}
	pb(&id, -1, "done");
	return NULL;
}

static void *
do_mine(void *arg) {
	int id = 0, i;

	pb(&id, -1, "init mining");
	usleep(100000);
	for(i = 0; i < 100; i++) {
		pb(&id, i, "mining");
		usleep(100000);
	}
	for(i = 0; i < 6; i++) {
		pb(&id, -1, "You Are Rich now");
		usleep(500000);
		pb(&id, -1, "");
		usleep(500000);
	}
	pb(&id, -1, "done");
	return NULL;
}

static void *
do_encrypt(void *arg) {
	int id = 0, i;

	pb(&id, -1, "init encrypting");
	usleep(1000000);
	for(i = 0; i < 100; i+=2) {
		pb(&id, i, "encrypting");
		usleep(300000);
	}
	pb(&id, -1, "done");
	return NULL;
}

int
main(int argc, char *argv[]) {
	pthread_t load_thread;
	pthread_t calculate_thread;
	pthread_t hack_thread;
	pthread_t mine_thread;
	pthread_t encrypt_thread;

	pb_init();

	pthread_create(&load_thread, NULL, do_load, NULL);
	pthread_create(&calculate_thread, NULL, do_calculate, NULL);
	pthread_create(&hack_thread, NULL, do_hack, NULL);
	pthread_create(&mine_thread, NULL, do_mine, NULL);
	pthread_create(&encrypt_thread, NULL, do_encrypt, NULL);

	pthread_join(load_thread, NULL);
	pthread_join(calculate_thread, NULL);
	pthread_join(hack_thread, NULL);
	pthread_join(mine_thread, NULL);
	pthread_join(encrypt_thread, NULL);

	pb_clean();
	return 0;
}
