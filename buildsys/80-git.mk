.PHONY: push-ritual
push-ritual:
	$(GIT) add .
	$(GIT) commit
	$(GIT) push