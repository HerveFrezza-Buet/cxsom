help:
	@echo
	@echo "Read the README.txt file for instructions."
	@echo
	@echo "make cxsom-help                                    <-- help for common cxsom manipulations."
	@echo
	@echo "# Inputs mode"
	@echo 
	@echo "make inputs-setup IMAGE_SIDE=100                   <-- sets up inputs."
	@echo "make show-samples                                  <-- shows the image check/test samples"
	@echo
	@echo

# Adapt this path if needed.
include /usr/share/cxsom/cxsom-makefile


.PHONY: inputs-setup
inputs-setup:
	@cxsom-builder-example-003-002-experiment send `cat .cxsom-hostname-config` `cat .cxsom-port-config` -- input ${IMAGE_SIDE}
	@python3 build-eye-input.py `cat .cxsom-rootdir-config`
	@python3 build-img-coords.py `cat .cxsom-rootdir-config` img ${IMAGE_SIDE}
	@make --quiet cxsom-ping-processor

.PHONY: show-samples
show-samples:
	@python3 show-samples.py `cat .cxsom-rootdir-config`

