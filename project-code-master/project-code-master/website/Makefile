# the port on which to run the server
PORT=21068
# the command that copies the config file into the correct directory
UPDATE_CONF=cp JH-project.conf /host/${USER}/nginx.d/default/
# the command that logs into SSH
SSH_LOGIN=ssh ${USER}@${USER}.host.cs.st-andrews.ac.uk
# the commnad that runs the server as a background proccess
RUN=nohup node index.js ${PORT} &
# the command that reloads nginx in order to update the config
RELOAD_NGINX=/usr/sbin/nginx -c /host/${USER}/nginx.conf -s reload
# the pid of the process listening on our port
RUNNING_SERVER_PID=/usr/bin/lsof -t -i:${PORT} -c ^nginx

.PHONY: run
.PHONY: run-in-ssh
.PHONY: ssh
.PHONY: update-conf-in-ssh
.PHONY: update-conf
.PHONY: open

run-in-ssh:
	$(RUN)

run:
	$(SSH_LOGIN) "cd $(CURDIR);${RUN}"

ssh:
	ssh -t ${USER}@${USER}.host.cs.st-andrews.ac.uk "cd $(CURDIR); echo SSH started in $(CURDIR);bash"

kill:
	$(SSH_LOGIN) "cd $(CURDIR);make kill-in-ssh"

kill-in-ssh:
	kill "$$(/usr/bin/lsof -t -i:${PORT} -c ^nginx)"

update-conf-in-ssh: JH-project.conf
	$(UPDATE_CONF)
	$(RELOAD_NGINX)

update-conf: JH-project.conf
	$(SSH_LOGIN) "cd $(CURDIR);${UPDATE_CONF}"
	$(SSH_LOGIN) "${RELOAD_NGINX}"

open:
	firefox ${USER}.host.cs.st-andrews.ac.uk/JH-project
