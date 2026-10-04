#
# Regular cron jobs for the game-of-ur package.
#
0 4	* * *	root	[ -x /usr/bin/game-of-ur_maintenance ] && /usr/bin/game-of-ur_maintenance
