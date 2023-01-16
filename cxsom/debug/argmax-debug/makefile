help:
	@echo
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations"
	@echo
	@echo "make rule-argmax"
	@echo
	@echo "make send-rules"
	@echo "make analyze TIMELINE=<timeline>"
	@echo
	@echo

include /usr/share/cxsom/cxsom-makefile

rule-argmax: argmax.cpp
	g++ -o rules argmax.cpp `pkg-config --cflags --libs cxsom-rules`

send-rules:
	@./rules send `cat .cxsom-hostname-config` `cat .cxsom-port-config`

analyse: analyse.cpp
	g++ -o analyse analyse.cpp `pkg-config --cflags --libs cxsom-viewer`

analyze: analyse
	./analyse `cat .cxsom-rootdir-config` ${TIMELINE}

