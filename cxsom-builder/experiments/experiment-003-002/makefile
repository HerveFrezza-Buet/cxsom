help:
	@echo
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations."
	@echo
	@echo "make input-setup                                   <-- sets up the input generator."
	@echo "make feed WALLTIME=500                             <-- feed inputs."
	@echo

# Adapt this path if needed.
include /usr/share/cxsom/cxsom-makefile

.PHONY: input-setup
input-setup:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- input
	@python3 build-eye-input.py `cat .cxsom-rootdir-config`
	@cxsom-ping `cat .cxsom-hostname-config` `cat .cxsom-port-config`

.PHONY: feed
feed:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- walltime ${WALLTIME}


