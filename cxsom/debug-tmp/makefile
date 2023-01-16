help:
	@echo
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations"
	@echo "make rule-argmax"

include /usr/share/cxsom/cxsom-makefile

rule-argmax: argmax.cpp
	g++ -o rules argmax.cpp `pkg-config --cflags --libs cxsom-rules`

