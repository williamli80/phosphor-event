#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>
#include <assert.h>
#include <unistd.h>
#include "message.H"

using namespace std;

string eventspath = "./events";

void build_event_record(event_record_t *rec,
			const char *message,
			const char *severity,
			const char *association,
			const char *reportedby,
			const uint8_t *p,
			size_t n)
{
	rec->message     = (char*) message;
	rec->severity    = (char*) severity;
	rec->association = (char*) association;
	rec->reportedby  = (char*) reportedby;
	rec->p           = (uint8_t*) p;
	rec->n           = n;

	return;
}

void setup(void)
{
	char *cmd;

	asprintf(&cmd, "exec rm -r %s 2> /dev/null", eventspath.c_str());
	system(cmd);
	free(cmd);

	asprintf(&cmd, "exec mkdir  %s 2> /dev/null", eventspath.c_str());
	system(cmd);
	free(cmd);

	return;
}


int main(int argc, char *argv[])
{
	uint8_t p[] = {0x3, 0x32, 0x34, 0x36};
	event_record_t rec, *prec;
	string s;


	setup();

	event_manager m(eventspath, 0);

	assert(m.get_managed_size() == 0);

	assert(m.next_log() == 0);
	m.next_log_refresh();
	assert(m.next_log() == 0);
	assert(m.latest_log_id() == 0);
	assert(m.log_count() == 0);

	// Building 1 Event log
	build_event_record(&rec,"Testing Message1", "Info", "Association", "Test", p, 4);
	assert(m.create(&rec) == 1);
	assert(m.get_managed_size() == 75);
	assert(m.log_count() == 1);
	assert(m.latest_log_id() == 1);
	m.next_log_refresh();
	assert(m.next_log() == 1);
	m.next_log_refresh();

	// Building 2nd Event log
	build_event_record(&rec,"Testing Message2", "Info", "Association", "Test", p, 4);
	assert(m.create(&rec) == 2);
	assert(m.get_managed_size() == 150);
	assert(m.log_count() == 2);
	assert(m.latest_log_id() == 2);
	m.next_log_refresh();
	assert(m.next_log() == 1);
	assert(m.next_log() == 2);

	// Read Log 1
	assert(m.open(1, &prec) == 1);
	s = prec->message;
	assert(s.compare("Testing Message1") == 0);
	m.close(prec);

	// Read Log 2
	assert(m.open(2, &prec) == 2);
	s = prec->message;
	assert(s.compare("Testing Message2") == 0);
	m.close(prec);

	// Lets delete the earlier log, then create a new event manager
	// the latest_log_id should still be 2
	m.remove(1);
	assert(m.get_managed_size() == 75);

	event_manager q(eventspath, 0);
	assert(q.latest_log_id() == 2);
	assert(q.log_count() == 1);
	m.next_log_refresh();

	// Travese log list stuff
	system("exec rm -r ./events/* 2> /dev/null");
	event_manager a(eventspath, 0);
	assert(a.next_log() == 0);

	build_event_record(&rec,"Testing list", "Info", "Association", "Test", p, 4);
	a.create(&rec);
	a.create(&rec);

	event_manager b(eventspath, 0);
	assert(b.next_log() == 1);


	/* Testing the max limits for event logs */
	setup();
	event_manager d(eventspath, 75);
	build_event_record(&rec,"Testing Message1", "Info", "Association", "Test", p, 4);
	assert(d.create(&rec) == 0);

	event_manager e(eventspath, 76);
	build_event_record(&rec,"Testing Message1", "Info", "Association", "Test", p, 4);
	assert(e.create(&rec) == 1);

	setup();
	event_manager f(eventspath, 149);
	build_event_record(&rec,"Testing Message1", "Info", "Association", "Test", p, 4);
	assert(f.create(&rec) == 1);
	assert(f.create(&rec) == 0);


	return 0;
}