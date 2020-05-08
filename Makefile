all: daemonize.c
	$(CC) -o firstdaemon $?

start:
	./firstdaemon

check:
	ps xj | head -1
	@ps xj | grep firstdaemon

log:
	grep firstdaemon /var/log/syslog | tail
